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
	struct sockaddr_in server;
    s32 sock = -1;

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

	// Connect to the server
	sock = socket (AF_INET, SOCK_STREAM, IPPROTO_IP);

	if (sock < 0) {
		failExit(sock, "socket: %d %s\n", errno, strerror(errno));
	}

	memset (&server, 0, sizeof (server));

    // Get the host ID
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

    int connected = 0;

    // Set the socket to non-blocking mode
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    while (connected == 0) {
        // Try to connect
        printf("Connecting to server at %s\n", inet_ntoa(server.sin_addr));
        ret = connect(sock, (struct sockaddr *)&server, sizeof(server));

        if (ret < 0 && errno == EINPROGRESS) {
            fd_set write_fds;
            struct timeval timeout;

            FD_ZERO(&write_fds);
            FD_SET(sock, &write_fds);

            // Set the timeout to 10 seconds
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;

            // Use select() to wait for the connection or timeout
            int select_ret = select(sock + 1, NULL, &write_fds, NULL, &timeout);

            if (select_ret > 0) {
                // Check if the socket is writable (connection successful)
                if (FD_ISSET(sock, &write_fds)) {
                    // Set the socket back to blocking mode
                    fcntl(sock, F_SETFL, flags);
                    connected = 1;
                    break;
                } else {
                    close(sock);
                    failExit(sock, "connect: timed out\n");
                }
            } else if (select_ret == 0) {
                close(sock);
                failExit(sock, "connect: timed out\n");
            } else {
                close(sock);
                failExit(sock, "connect: %d %s\n", errno, strerror(errno));
            }
        } else if (ret < 0) {
            close(sock);
            failExit(sock, "connect: %d %s\n", errno, strerror(errno));
        } else {
            // Connection is successful
            connected = 1;
            break;
        }
    }

	printf("Connected to server at %s\n", inet_ntoa(server.sin_addr));
    
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