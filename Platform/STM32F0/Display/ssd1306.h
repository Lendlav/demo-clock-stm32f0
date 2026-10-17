#ifndef SSD1306_H
#define SSD1306_H

#include <stdbool.h>
#include <stdint.h>

void LCD_Init(void);
void LCD_command(uint8_t command);
bool LCD_SetContrast(uint8_t contrast);
void LCD_On(void);
void LCD_Off(void);
void LCD_Clear(void);
void LCD_Update(void);
void LCD_Chr(char character);
void LCD_2xChr(char character);
void LCD_FStr(const char *text, uint8_t x, uint8_t y);
/* Draw a scrolling ticker within width pixels; text must provide 23 chars plus NUL. */
void LCD_FScrollStr(const char *text, uint8_t pixel_offset, uint8_t page, uint8_t width);
void LCD_2xFStr(const char *text, uint8_t x, uint8_t y);
void LCD_GotoXY(uint8_t x, uint8_t y);
/* Draw the battery at page y, ending immediately before column x (0..128). */
void LCD_Batt(int millivolts, uint8_t x, uint8_t y);
/* Draw an 8x32 thermometer at pixel position x,y. */
void LCD_Thermometer(int16_t celsius, bool valid, uint8_t x, uint8_t y);
/* Draw a wireframe octahedron at the given center and angle (0..63). */
void LCD_DrawOctahedron(uint8_t angle, uint8_t center_x, uint8_t center_y);
/* Draw a smaller wireframe cube at the given center and angle (0..63). */
void LCD_DrawCube(uint8_t angle, uint8_t center_x, uint8_t center_y);
/* Draw a small rotating cross at the given center and angle (0..63). */
void LCD_DrawCross(uint8_t angle, uint8_t center_x, uint8_t center_y);
/* Draw a row-major monochrome bitmap into the framebuffer; zero bits are transparent. */
void LCD_DrawBitmap(const uint8_t *bitmap,
                    uint8_t width,
                    uint8_t height,
                    uint8_t row_stride,
                    int16_t x,
                    int16_t y);

#endif

