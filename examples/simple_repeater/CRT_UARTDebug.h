#pragma once

#ifdef CRT_DEBUG_UART

#include <Arduino.h>

void crt_debug_uart_begin();
void crt_debug_mirror(const char* fmt, ...);

#endif