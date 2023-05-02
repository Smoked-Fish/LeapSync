// Parts of this code is based on the "read-controls" example included with devkitPro, originally created by devkitPro contributors.
// LeapSync - Copyright (c) 2023 Jacob Espy. See LICENSE.txt for more details. 

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
#include <fcntl.h>
#include <sys/select.h>

#include "slip.h"
#include "network.h"
#include "input.h"

s32 sock = -1;

int main(int argc, char **argv)
{
	gfxInitDefault();
	atexit(gfxExit);

	consoleInit(GFX_TOP, NULL);
	u32 kDownOld = 0, kHeldOld = 0, kUpOld = 0;

	// Connect to the server
	sock = network_init();

	printf("\x1b[1;1HHold Start and Down and press R to exit.");
	printf("\x1b[2;1HCirclePad position:");
	printf("\x1b[4;1HC-Stick position:");
	printf("\x1b[6;1HTouch data:");
	printf("\x1b[8;1HGyro data:");
	printf("\x1b[10;1HAccel data:");
	printf("\x1b[12;1H");

	circlePosition prevCirclePos = {0, 0};
	circlePosition prevCStickPos = {0, 0};
	touchPosition  prevTouchPos  = {0, 0};
	angularRate	   prevGyroPos   = {0, 0, 0};
	accelVector    prevAccelPos  = {0, 0, 0};

	HIDUSER_EnableAccelerometer();

	while (aptMainLoop())
	{
		hidScanInput();

		process_input(sock, &kDownOld, &kHeldOld, &kUpOld, &prevCirclePos, &prevCStickPos, &prevTouchPos, &prevGyroPos, &prevAccelPos);

		if ((kHeldOld & KEY_START) && (kHeldOld & KEY_DDOWN) && (kDownOld & KEY_R)) {
			break;
		}

		if (!aptMainLoop()) {
			break;
		}

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}


	network_cleanup(sock);
	gfxExit();
	return 0;
}
