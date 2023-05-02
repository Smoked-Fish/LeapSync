// Parts of this code is based on the "read-controls" example included with devkitPro, originally created by devkitPro contributors.
// LeapSync - Copyright (c) 2023 Jacob Espy. See LICENSE.txt for more details. 

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "input.h"
#include "slip.h"
#include "network.h"

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

void send_button_state(int sock, uint8_t key_hex, bool state) {
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

void send_touch_position(int sock, int px, int py) {
    slip_encode_message_t* msg = slip_encode_message_create(11);
    slip_encode_begin(msg);

    // Encode the position as a string
    char pos_str[10];
    snprintf(pos_str, 10, "(%03d,%03d)", px, py);

    // Encode the position string
    int len = strlen(pos_str);
    int i;
    for (i = 0; i < len; i++) {
        slip_encode_byte(msg, pos_str[i]);
    }

    slip_encode_byte(msg, SLIP_TOUCH);
    slip_encode_finish(msg);

    // Send the message
    send(sock, msg->encoded, msg->index, 0);
    slip_encode_message_destroy(msg);
}

void send_motion_data(int sock, int x, int y, int z, bool gyro) {
    int msg_size = gyro ? 21 : 18;
    slip_encode_message_t* msg = slip_encode_message_create(msg_size);
    slip_encode_begin(msg);

    // Encode the position as a string
    char pos_str[msg_size-1];
	if (gyro) {
        snprintf(pos_str, msg_size-1, "(%05d,%05d,%05d)", x, y, z);
    } else {
        snprintf(pos_str, msg_size-1, "(%04d,%04d,%04d)", x, y, z);
    }

    // Encode the position string
    int len = strlen(pos_str);
    int i;
    for (i = 0; i < len; i++) {
        slip_encode_byte(msg, pos_str[i]);
    }

	if (gyro) {
        slip_encode_byte(msg, SLIP_GYRO);
    } else {

        slip_encode_byte(msg, SLIP_ACCEL);
    }

    slip_encode_finish(msg);

    // Send the message
    send(sock, msg->encoded, msg->index, 0);
    slip_encode_message_destroy(msg);
}

void process_input(int sock, u32 *kDownOld, u32 *kHeldOld, u32 *kUpOld, circlePosition *prevCirclePos, circlePosition *prevCStickPos, touchPosition *prevTouchPos, angularRate *prevGyroPos, accelVector *prevAccelPos) {
    u32 kDown = hidKeysDown();
    u32 kHeld = hidKeysHeld();
    u32 kUp = hidKeysUp();

    if (kDown != *kDownOld || kHeld != *kHeldOld || kUp != *kUpOld)
    {
        consoleClear();
        printf("\x1b[1;1HHold Start and Down and press R to exit.");
        printf("\x1b[2;1HCirclePad position:");
        printf("\x1b[4;1HC-Stick position:");
        printf("\x1b[6;1HTouch data:");
        printf("\x1b[8;1HGyro data:");
        printf("\x1b[10;1HAccel data:");
        printf("\x1b[12;1H");

        // Check for pressed keys and send them to the server
        int i;
        for (i = 0; i < 24; i++)
        {
            if (kDown & BIT(i))
            {
                printf("%s down\n", keysNames[i]);
                send_button_state(sock, keysHex[i], true);
            }
            if (kHeld & BIT(i))
            {
                printf("%s held\n", keysNames[i]);
            }
            if (kUp & BIT(i))
            {
                printf("%s up\n", keysNames[i]);
                send_button_state(sock, keysHex[i], false);
            }
        }
    }


    *kDownOld = kDown;
    *kHeldOld = kHeld;
    *kUpOld = kUp;

    circlePosition circlePos;
    circlePosition cstickPos;
    touchPosition touchPos;
    angularRate gyroPos;
    accelVector accelPos;

    hidCircleRead(&circlePos);
    hidCstickRead(&cstickPos);
    hidTouchRead(&touchPos);
    hidGyroRead(&gyroPos);
    hidAccelRead(&accelPos);

    printf("\x1b[3;1H%04d; %04d", circlePos.dx, circlePos.dy);
    printf("\x1b[5;1H%04d; %04d", cstickPos.dx, cstickPos.dy);
    printf("\x1b[7;1H%03d; %03d", touchPos.px, touchPos.py);
    printf("\x1b[9;1H%05d, %05d, %05d", gyroPos.z,  gyroPos.y, gyroPos.z);
    printf("\x1b[11;1H%04d, %04d, %04d", accelPos.x, accelPos.y, accelPos.z);


    if (circlePos.dx != prevCirclePos->dx || circlePos.dy != prevCirclePos->dy) {
        send_circle_position(sock, circlePos.dx, circlePos.dy, true);
    }

    if (cstickPos.dx != prevCStickPos->dx || cstickPos.dy != prevCStickPos->dy) {
        send_circle_position(sock, cstickPos.dx, cstickPos.dy, false);
    }

    if (touchPos.px != prevTouchPos->px || touchPos.py != prevTouchPos->py) {
        send_touch_position(sock, touchPos.px, touchPos.py);
    }

    if (gyroPos.x != prevGyroPos->x || gyroPos.y != prevGyroPos->y || gyroPos.z != prevGyroPos->z) {
        send_motion_data(sock, gyroPos.x, gyroPos.y, gyroPos.z, true);
    }

    if (accelPos.x != prevAccelPos->x || accelPos.y != prevAccelPos->y || accelPos.z != prevAccelPos->z) {
        send_motion_data(sock, accelPos.x, accelPos.y, accelPos.z, false);
    }


    *prevCirclePos = circlePos;
    *prevCStickPos = cstickPos;
    *prevTouchPos = touchPos;
    *prevGyroPos = gyroPos;
    *prevAccelPos = accelPos;
}