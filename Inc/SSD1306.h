#include "stdint.h"
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------

void LCD_Init   (void);
void LCD_command(uint8_t cmd);
void LCD_On     (void);
void LCD_Off    (void);
void LCD_Clear  (void);
void LCD_Update (void);
void LCD_Chr    (char ch);
void LCD_2xChr  (char ch);
void LCD_FStr   (const char *dataPtr,uint8_t x,uint8_t y);
void LCD_2xFStr (const char *dataPtr,uint8_t x,uint8_t y);
void LCD_GotoXY (uint8_t x, uint8_t y);
void LCD_Batt   (int val, uint8_t x, uint8_t y);


//----------------------------------------------------------------------------------
extern const unsigned char font [][5];;

