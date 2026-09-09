#include "CRT_UARTDebug.h"

#ifdef CRT_DEBUG_UART

#include <cstdarg>

void crt_debug_uart_begin() {
  Serial1.setPins(D0, D1);
  Serial1.begin(115200);
  crt_debug_mirror("[CRT] UART debug mirror on D0 @115200\n");
}

void crt_debug_mirror(const char* fmt, ...) {
  char buf[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
 ; Serial.print(buf);
  Serial1.print(buf);
}


#endif