typedef int Color;
void Display_init(void);
void Display_clear(Color color);
void Display_update(void);
unsigned int Display_getWidth(void);
unsigned int Display_getHeight(void);
void Display_drawPixel(int x, int y, Color color);
void Display_changeResolution(unsigned int x_res, unsigned int y_res);
void Display_lockResolution(void);
void Display_unlockResolution(void);
void Display_drawText(const char *str, int x, int y, Color color);
void Display_drawImage(const unsigned char *image_bits,
                       int x_size, int y_size,
                       int x_off, int y_off, int mode);
