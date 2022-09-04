#include "hal.h"
#include "chprintf.h"
#include "display.h"
#include <stdarg.h>

static BaseSequentialStream *chp;

void Logger_init(void)
{
  /* Port Configuration: PA2 and PA3 connected to Debugger Emulated Serial Port .
   * Serial Driver 2 works on STM32 USART2 Peripheral!
   */
  palSetPadMode( GPIOA, 2, PAL_MODE_ALTERNATE(7) );
  palSetPadMode( GPIOA, 3, PAL_MODE_ALTERNATE(7) );

  /* Starting Serial Driver #2 */
  sdStart(&SD2, NULL);

  chp = (BaseSequentialStream *) &SD2;
}

void Logger_quit(void)
{

}

static void Logger_vprintf(const char *file, unsigned int line,
                           const char *fmt, va_list va)
{
  chprintf(chp, "%s:%d :: ", file, line);
  chvprintf(chp, fmt, va);
  chprintf(chp, "\r\n");
}

void Logger_printf_(const char *file,
                    unsigned int line,
                    const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  Logger_vprintf(file, line, fmt, args);
  va_end(args);
}
