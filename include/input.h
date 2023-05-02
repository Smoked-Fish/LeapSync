// LeapSync - Copyright (c) 2023 Jacob Espy. See LICENSE.txt for more details. 
#pragma once

/// Sends a key press or release event to the server.
/// @param sock socket descriptor used for sending data
/// @param key_hex hex value of the key event
/// @param state true for key press, false for key release
void send_button_state(int sock, uint8_t key_hex, bool state);

/// Sends the CirclePad or C-Stick position to the server.
/// @param sock socket descriptor used for sending data
/// @param dx the x-axis value of the position
/// @param dy the y-axis value of the position
/// @param cPad true for CirclePad, false for C-Stick
void send_circle_position(int sock, int dx, int dy, bool cPad);

/// Sends the touchscreen position to the server
/// @param sock socket descriptor used for sending data
/// @param px the x-axis value of the position
/// @param py the y-axis value of the position
void send_touch_position(int sock, int px, int py);

/// Sends motion data to the server.
/// @param sock socket descriptor used for sending data
/// @param x the x-axis value of the motion data
/// @param y the y-axis value of the motion data
/// @param z the z-axis value of the motion data
/// @param gyro true for Gyro, false for Accel
void send_motion_data(int sock, int x, int y, int z, bool gyro);

/// Processes user input and sends it to the server.
/// @param sock socket descriptor used for sending data
/// @param kDownOld pointer to the previous key down state
/// @param kHeldOld pointer to the previous key held state
/// @param kUpOld pointer to the previous key up state
/// @param prevCirclePos pointer to the previous CirclePad position
/// @param prevCStickPos pointer to the previous C-Stick position
/// @param touchPosition pointer to the previous Touch position
/// @param prevGyroPos pointer to the previous Gyro position
/// @param prevAccelPos pointer to the previous Accel position
void process_input(int sock, u32 *kDownOld, u32 *kHeldOld, u32 *kUpOld, circlePosition *prevCirclePos, circlePosition *prevCStickPos, touchPosition *prevTouchPos, angularRate *prevGyroPos, accelVector *prevAccelPos);