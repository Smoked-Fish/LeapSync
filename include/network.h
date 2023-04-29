// LeapSync - Copyright (c) 2023 Jacob Espy. See LICENSE.txt for more details. 
#pragma once

#include <3ds.h>

/// Initialize network connection and returns the socket descriptor.
/// @return socket descriptor
s32 network_init();

/// Clean up network resources.
/// @param sock socket descriptor to be closed
void network_cleanup(s32 sock);

/// Print an error message and exit the program.
/// @param sock socket descriptor to be closed
/// @param fmt formatted error message string
/// @param ... additional arguments to be used in the formatted error string
void failExit(s32 sock, const char *fmt, ...);

/// Shutdown and cleanup the soc service.
void socShutdown();