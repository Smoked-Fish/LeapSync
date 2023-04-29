/// Circle Pad example for libctru, modified by Jacob Espy

#include <3ds.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "slip.h"

#define SERVER_IP "192.168.137.1"
#define SERVER_PORT 9001

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

static u32 *SOC_buffer = NULL;
s32 sock = -1;

void socShutdown() {
	printf("waiting for socExit...\n");
	socExit();
}

void send_key(int sock, uint8_t key_hex, bool state) {
	// Allocate space for the extra character with + 1
    slip_encode_message_t* msg = slip_encode_message_create(2);  
    slip_encode_begin(msg);

	slip_encode_byte(msg, key_hex);

    if (state) {
        slip_encode_byte(msg, SLIP_TRUE);
    } else {
        slip_encode_byte(msg, SLIP_FALSE);
    }

    slip_encode_finish(msg);

    send(sock, msg->encoded, msg->index, 0);

    slip_encode_message_destroy(msg);
}

void send_circle_position(int sock, int dx, int dy, bool cPad) {
    slip_encode_message_t* msg = slip_encode_message_create(13);
    slip_encode_begin(msg);

    // Encode the position as a string
    char pos_str[12];
    snprintf(pos_str, 12, "(%04d,%04d)", dx, dy);

    // Encode the position string
    int len = strlen(pos_str);
    int i;
    for (i = 0; i < len; i++) {
        slip_encode_byte(msg, pos_str[i]);
    }

	if (cPad) {
        slip_encode_byte(msg, SLIP_CIRCLE);
    } else {
        slip_encode_byte(msg, SLIP_CSTICK);
    }


    slip_encode_finish(msg);

    // Send the message
    send(sock, msg->encoded, msg->index, 0);

    slip_encode_message_destroy(msg);
}

void send_gyro_position(int sock, int gx, int gy, int gz) {
    slip_encode_message_t* msg = slip_encode_message_create(18);
    slip_encode_begin(msg);

    // Encode the position as a string
    char pos_str[17];
    snprintf(pos_str, 17, "(%04d,%04d,%04d)", gx, gy, gz);

    // Encode the position string
    int len = strlen(pos_str);
    int i;
    for (i = 0; i < len; i++) {
        slip_encode_byte(msg, pos_str[i]);
    }

	slip_encode_byte(msg, SLIP_GYRO);


    slip_encode_finish(msg);

    // Send the message
    send(sock, msg->encoded, msg->index, 0);

    slip_encode_message_destroy(msg);
}

void failExit(const char *fmt, ...) {
	if(sock>0) close(sock);

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

int main(int argc, char **argv)
{
	char keysNames[32][32] = {
		"KEY_A", "KEY_B", "KEY_SELECT", "KEY_START",
		"KEY_DRIGHT", "KEY_DLEFT", "KEY_DUP", "KEY_DDOWN",
		"KEY_R", "KEY_L", "KEY_X", "KEY_Y",
		"", "", "KEY_ZL", "KEY_ZR",
		"", "", "", "",
		"KEY_TOUCH", "", "", "",
		"KEY_CSTICK_RIGHT", "KEY_CSTICK_LEFT", "KEY_CSTICK_UP", "KEY_CSTICK_DOWN",
		"KEY_CPAD_RIGHT", "KEY_CPAD_LEFT", "KEY_CPAD_UP", "KEY_CPAD_DOWN"};

	uint8_t keysHex[24] = {
		0x00, 0x01, 0x02, 0x03,
		0x04, 0x05, 0x06, 0x07,
		0x08, 0x0B, 0x0C, 0x0E,
		0x00, 0x00, 0x0F, 0x10,
		0x00, 0x00, 0x00, 0x00,
		0x11, 0x00, 0x00, 0x00
	};

	int ret;

	struct sockaddr_in server;
	// char temp[1026];

	gfxInitDefault();

	// register gfxExit to be run when app quits
	// this can help simplify error handling
	atexit(gfxExit);

	consoleInit(GFX_TOP, NULL);

	// allocate buffer for SOC service
	SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);

	if(SOC_buffer == NULL) {
		failExit("memalign: failed to allocate\n");
	}

	// Now intialise soc:u service
	if ((ret = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0) {
    	failExit("socInit: 0x%08X\n", (unsigned int)ret);
	}

	// register socShutdown to run at exit
	// atexit functions execute in reverse order so this runs before gfxExit
	atexit(socShutdown);

	u32 kDownOld = 0, kHeldOld = 0, kUpOld = 0;

	// Connect to the server
	// libctru provides BSD sockets so most code from here is standard
	sock = socket (AF_INET, SOCK_STREAM, IPPROTO_IP);

	if (sock < 0) {
		failExit("socket: %d %s\n", errno, strerror(errno));
	}

	memset (&server, 0, sizeof (server));

	server.sin_family = AF_INET;
	server.sin_port = htons (SERVER_PORT);
	inet_aton(SERVER_IP, &server.sin_addr);

	printf("Connecting to server at %s\n",inet_ntoa(server.sin_addr));

	if ( (ret = connect(sock, (struct sockaddr *)&server, sizeof(server))) ) {
		close(sock);
		failExit("connect: %d %s\n", errno, strerror(errno));
	}

	printf("\x1b[1;1HPress Start to exit.");
	printf("\x1b[2;1HCirclePad position:");

	circlePosition prevCirclePos = {0, 0};
	circlePosition prevCStickPos = {0, 0};
	angularRate	   prevGyroPos   = {0, 0, 0};

	while (aptMainLoop())
	{
		hidScanInput();

		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();
		u32 kUp = hidKeysUp();

		if ((kDown & KEY_START) && (kDown & KEY_DDOWN))
			break;

		if (kDown != kDownOld || kHeld != kHeldOld || kUp != kUpOld)
		{
			consoleClear();
			printf("\x1b[1;1HPress Start and Down to exit.");
			printf("\x1b[2;1HCirclePad position:");
			printf("\x1b[4;1HC-Stick position:");
			printf("\x1b[6;1HGyro data:");
			printf("\x1b[8;1H");

			// Check for pressed keys and send them to the server
			int i;
			for (i = 0; i < 24; i++)
			{
				if (kDown & BIT(i))
				{
					printf("%s down\n", keysNames[i]);
					send_key(sock, keysHex[i], true);
				}
				if (kHeld & BIT(i))
				{
					printf("%s held\n", keysNames[i]);
				}
				if (kUp & BIT(i))
				{
					printf("%s up\n", keysNames[i]);
					send_key(sock, keysHex[i], false);
				}
			}
		}

		kDownOld = kDown;
		kHeldOld = kHeld;
		kUpOld = kUp;

		circlePosition circlePos;
		hidCircleRead(&circlePos);
		printf("\x1b[3;1H%04d; %04d", circlePos.dx, circlePos.dy);

		if (circlePos.dx != prevCirclePos.dx || circlePos.dy != prevCirclePos.dy) {
			send_circle_position(sock, circlePos.dx, circlePos.dy, true);
		}

		prevCirclePos = circlePos;

		circlePosition cstickPos;
		hidCstickRead(&cstickPos);
		printf("\x1b[5;1H%04d; %04d", cstickPos.dx, cstickPos.dy);

		if (cstickPos.dx != prevCStickPos.dx || cstickPos.dy != prevCStickPos.dy) {
			send_circle_position(sock, cstickPos.dx, cstickPos.dy, false);
		}

		prevCStickPos = cstickPos;

		angularRate gyroPos;
		hidGyroRead(&gyroPos);
		printf("\x1b[7;1H%04d, %04d, %04d", gyroPos.x, gyroPos.y, gyroPos.z);
		if (gyroPos.x != prevGyroPos.x || gyroPos.y != prevGyroPos.y || gyroPos.z != prevGyroPos.z) {
			send_gyro_position(sock, gyroPos.x, gyroPos.y, gyroPos.z);
		}

		prevGyroPos = gyroPos;

		if (!aptMainLoop()) {
			break;
		}

		u32 kHeldBreak = hidKeysHeld();

		if ((kHeldBreak & KEY_START) && (kHeldBreak & KEY_DDOWN)) {
			break;
		}

        gfxFlushBuffers();
        gfxSwapBuffers();
		gspWaitForVBlank();
	}

	close(sock);
	gfxExit();
	return 0;
}
