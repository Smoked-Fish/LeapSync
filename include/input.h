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

/// Sends the Gyro data to the server.
/// @param sock socket descriptor used for sending data
/// @param gx the x-axis value of the gyro data
/// @param gy the y-axis value of the gyro data
/// @param gz the z-axis value of the gyro data
void send_gyro_data(int sock, int gx, int gy, int gz);

/// Processes user input and sends it to the server.
/// @param sock socket descriptor used for sending data
/// @param kDownOld pointer to the previous key down state
/// @param kHeldOld pointer to the previous key held state
/// @param kUpOld pointer to the previous key up state
/// @param prevCirclePos pointer to the previous CirclePad position
/// @param prevCStickPos pointer to the previous C-Stick position
/// @param prevGyroPos pointer to the previous Gyro position
void process_input(int sock, u32 *kDownOld, u32 *kHeldOld, u32 *kUpOld, circlePosition *prevCirclePos, circlePosition *prevCStickPos, angularRate *prevGyroPos);