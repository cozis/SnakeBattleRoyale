#include "logger.h"
#include "display.h"
#include "ssd1306.h"

typedef struct {
  unsigned int x_resolution; // virtual pixel width
  unsigned int y_resolution; // virtual pixel height
  unsigned int res_lock;
  SSD1306Driver SSD1306D1;
} Display;

static Display display;

static void sanitizeResolution(unsigned int *x_res, unsigned int *y_res)
{
  const unsigned int display_w = SSD1306_WIDTH;
  const unsigned int display_h = SSD1306_HEIGHT;
  if (*x_res == 0) *x_res = 1;
  if (*y_res == 0) *y_res= 1;
  if (*x_res > display_w) *x_res = display_w;
  if (*y_res > display_h) *y_res = display_h;
}

void Display_changeResolution(unsigned int x_res,
                              unsigned int y_res)
{
  if (display.res_lock == 0) {
    sanitizeResolution(&x_res, &y_res);
    display.x_resolution = x_res;
    display.y_resolution = y_res;
  } else {
    Logger_printf("Couldn't change resolution because it was locked");
  }
}

void Display_lockResolution(void)
{
  display.res_lock++;
}

void Display_unlockResolution(void)
{
  // TODO: Check that the lock counter isn't zero
  display.res_lock--;
}

void Display_init(void)
{
  /* Configuring I2C related PINs */
     palSetLineMode(LINE_ARD_D15, PAL_MODE_ALTERNATE(4)
                                | PAL_STM32_OTYPE_OPENDRAIN
                                | PAL_STM32_OSPEED_HIGHEST
                                | PAL_STM32_PUPDR_PULLUP);

     palSetLineMode(LINE_ARD_D14, PAL_MODE_ALTERNATE(4)
                                | PAL_STM32_OTYPE_OPENDRAIN
                                | PAL_STM32_OSPEED_HIGHEST
                                | PAL_STM32_PUPDR_PULLUP);

  /* Initialize, start and configure the SSD1306 driver */
  ssd1306ObjectInit(&display.SSD1306D1);
  static const I2CConfig i2ccfg = { OPMODE_I2C, 400000, FAST_DUTY_CYCLE_2 };
  static const SSD1306Config ssd1306cfg = { &I2CD1, &i2ccfg, SSD1306_SAD_0X78, };
  ssd1306Start(&display.SSD1306D1, &ssd1306cfg);
}

unsigned int Display_getWidth(void)
{
  return SSD1306_WIDTH / display.x_resolution;
}

unsigned int Display_getHeight(void)
{
  return SSD1306_HEIGHT / display.y_resolution;
}

static void Display_drawPhysicalPixel(int x, int y, Color color)
{
  ssd1306DrawPixel(&display.SSD1306D1, x, y, color);
}

void Display_drawPixel(int x, int y, Color color)
{
  for (int rel_x = 0; rel_x < (int) display.x_resolution; ++rel_x)
    for (int rel_y = 0; rel_y < (int) display.y_resolution; ++rel_y)
      ssd1306DrawPixel(&display.SSD1306D1,
                       rel_x + x * display.x_resolution,
                       rel_y + y * display.y_resolution,
                       color);
}

void Display_clear(Color color)
{
  ssd1306FillScreen(&display.SSD1306D1, color);
}

void Display_update(void)
{
  ssd1306UpdateScreen(&display.SSD1306D1);
}

void Display_drawImage(const unsigned char *image_bits,
                       int x_size, int y_size,
                       int x_off, int y_off, int mode)
{
  int x;
  int y;
  int i;

  i = 0;
  for (y = 0; y < y_size; ++y) {
    for (x = 0; x < x_size; ++x) {
      if (image_bits[i] & (1 << (7-(x%8)))) {
        Display_drawPhysicalPixel(x + x_off, y + y_off, mode);
      }
      else {
        Display_drawPhysicalPixel(x + x_off, y + y_off, !mode);
      }

      if (x+1 != x_size && ((x+1) % 8 == 0)) ++i;
    }
    ++i;
  }
}

void Display_drawText(const char *str, int x, int y, Color color)
{
  ssd1306GotoXy(&display.SSD1306D1, x, y);
  ssd1306Puts(&display.SSD1306D1, str, &ssd1306_font_7x10, color == 1 ? SSD1306_COLOR_BLACK : SSD1306_COLOR_WHITE);
}
