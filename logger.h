#include "config.h"

#ifndef NOLOGGING

void Logger_init(void);
void Logger_quit(void);
void Logger_printf_(const char *file, unsigned int line, const char *fmt, ...);

#define Logger_printf(fmt, ...) \
  Logger_printf_(__FILE__, __LINE__, fmt, ## __VA_ARGS__)

#else

#define Logger_init() {}
#define Logger_quit() {}
#define Logger_printf(fmt, ...) {}

#endif
