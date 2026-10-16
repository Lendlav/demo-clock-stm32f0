#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

void LCD_Init(void);
void LCD_command(uint8_t command);
void LCD_On(void);
void LCD_Off(void);
void LCD_Clear(void);
void LCD_Update(void);
void LCD_Chr(char character);
void LCD_2xChr(char character);
void LCD_FStr(const char *text, uint8_t x, uint8_t y);
void LCD_2xFStr(const char *text, uint8_t x, uint8_t y);
void LCD_GotoXY(uint8_t x, uint8_t y);
void LCD_Batt(int millivolts, uint8_t x, uint8_t y);

#endif

