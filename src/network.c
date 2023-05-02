// LeapSync - Copyright (c) 2023 Jacob Espy. See LICENSE.txt for more details. 

#include <3ds.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>

#include "network.h"

#define SERVER_PORT 9001
#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

static u32 *SOC_buffer = NULL;

s32 network_init() {
	int ret;
    int connected = 0;
	struct sockaddr_in server;
    s32 sock = -1;
    int retry_count = 0;
    int max_retries = 5;

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

    SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    
    #pragma GCC diagnostic pop

    atexit(socShutdown);

	if(SOC_buffer == NULL) {
		failExit(sock, "memalign: failed to allocate\n");
	}

	if ((ret = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0) {
    	failExit(sock, "socInit: 0x%08X\n", (unsigned int)ret);
	}

    // Loop until the connection is successful
    while (!connected && retry_count < max_retries) {
        // Connect to the server
        sock = socket (AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) {
            failExit(sock, "socket: %d %s\n", errno, strerror(errno));
        }

	    memset (&server, 0, sizeof (server));

        long int host_id = gethostid();
        // Convert the host ID to IP address string
        struct in_addr inaddr;
        inaddr.s_addr = htonl(host_id);
        char* ip_address = inet_ntoa(inaddr);

        // Reverse the order of the octets
        char* octets[4];
        int i = 0;
        char* octet = strtok(ip_address, ".");
        while (octet != NULL && i < 4) {
            octets[i++] = octet;
            octet = strtok(NULL, ".");
        }
        // Replease the end octet with 1, for gateway IP
        octets[0] = "1";
        char reversed_ip[16];
        sprintf(reversed_ip, "%s.%s.%s.%s", octets[3], octets[2], octets[1], octets[0]);

        server.sin_family = AF_INET;
        server.sin_port = htons (SERVER_PORT);
        inet_aton(reversed_ip, &server.sin_addr);

        // Set the socket to non-blocking mode
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);

        // Attempt to connect
        printf("Connecting to server at %s\n", inet_ntoa(server.sin_addr));
        ret = connect(sock, (struct sockaddr *)&server, sizeof(server));

        // Check if connection was successful
        if (ret < 0 && errno == EINPROGRESS) {
            // Connection is in progress, use select() to wait for connection or timeout
            fd_set write_fds;
            struct timeval timeout;

            FD_ZERO(&write_fds);
            FD_SET(sock, &write_fds);

            // Set the timeout to 5 seconds
            timeout.tv_sec = 4;
            timeout.tv_usec = 0;

            // Wait for the socket to become writable
            int select_ret = select(sock + 1, NULL, &write_fds, NULL, &timeout);

            if (select_ret > 0 && FD_ISSET(sock, &write_fds)) {
            // Connection successful
            connected = 1;
            } else {
                // Connection failed, close socket and retry
                close(sock);
                retry_count++;
            }
        } else if (ret < 0) {
            // Connection failed, close socket and retry
            close(sock);
            retry_count++;
        } else {
            // Connection successful
            connected = 1;
            consoleClear();
        }

        // Set socket back to blocking mode
        fcntl(sock, F_SETFL, flags);

    } // End of connection loop

    if (!connected) {
        // Connection failed after maximum retries, exit the program
        failExit(sock, "Failed to connect after %d retries.\n", max_retries);
    }

    return sock;
}

void network_cleanup(s32 sock) {
    if (sock > 0) {
        close(sock);
    }
    if (SOC_buffer != NULL) {
        socExit();
        free(SOC_buffer);
        SOC_buffer = NULL;
    }
}

void failExit(s32 sock, const char *fmt, ...) {

    network_cleanup(sock);
	va_list ap;

	printf(CONSOLE_RED);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf(CONSOLE_RESET);
	printf("\nPress B to exit\n");
	while (aptMainLoop()) {
	gspWaitForVBlank();
	hidScanInput();

	u32 kDown = hidKeysDown();
	if (kDown & KEY_B) exit(0);
	}
}

void socShutdown() {
	printf("waiting for socExit...\n");
	socExit();
}