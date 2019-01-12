//SSD1306 library 
#include "SSD1306.h"
#include "stm32f0xx_hal.h"

//----------------------------------------------------------------------------------

#define OWN_ADDRESS     0x70
#define LCD_ADDRESS     0x78

//----------------------------------------------------------------------------------
#define DATA_MODE       0x40
#define COMMAND_MODE    0x00   // bit Co = 0, D/C# = 0

#define BUFF_SIZE       513

#define column_start    0
#define column_end      127
#define page_start      0
#define page_end        3
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
extern I2C_HandleTypeDef hi2c1;

uint8_t  lcd_buff[BUFF_SIZE];
uint16_t lcd_buff_idx=0;
uint8_t  x_cur=0;
uint8_t  y_cur=0;

uint8_t comm[]={ 
   0x00, 	         //COMMAND_MODE command
   0xAE,             //Display Off
   0xD5,0x80,
   
   0xA8,0x1F,        //Set Multiplex Ratio     1F - 128x32, 3F - 128x64
   
   0xD3,0x00,        //Set Display Offset
   0x40,
   
   0x8D,0x14,        // Charge Pump set:   0x10 = VCC Supplied Externally  
                      // 0x14 = VCC Generated by Internal
   
   0x20,0x00,        //Set Memory Addressing Mode 
   
   0xA0,             //Set Segment Re-map  A0/A1
   0xC0,             //Vertical flip    C0/C8
   
   0xDA,0x02,        //Set COM Pins Hardware Configuration 
   
   0x81,0xA2,        //Set Contrast Control 
   
   0xD9,0xF1,        //Set Pre-Charge Period:  0x22 = VCC Supplied Externally  \
   //0xF1 = VCC Generated by Internal             
   
   0xDB,0x40,        //Set VCOMH Deselect Level
   
   0xA4,             //Set Entire Display On/Off
   0xA6,             //Set NORMAL/INVERT Display   A6/A7
   
   0xAF,             //Display On
   
   0xFF
};
//----------------------------------------------------------------------------------
void LCD_Init(void)
{
   HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDRESS, comm, sizeof(comm),10);
}
//----------------------------------------------------------------------------------
void  LCD_command(uint8_t cmd)
{   
   uint8_t cmd_arr[2] = {0,0};
   cmd_arr[0] = COMMAND_MODE;
   cmd_arr[1] = cmd;
   HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDRESS, cmd_arr, sizeof(cmd_arr),10);
}
//----------------------------------------------------------------------------------
void LCD_On(void)
{
   LCD_command(0xAF);
}
//----------------------------------------------------------------------------------
void LCD_Off(void)
{
   LCD_command(0xAE);
}
//----------------------------------------------------------------------------------
void LCD_Clear(void)
{
   for(uint16_t ix = 1; ix < BUFF_SIZE; ix++)
   {
      lcd_buff[ix] = 0;
   }
   x_cur = 0;
   y_cur = 0;
}
//----------------------------------------------------------------------------------
void LCD_Update(void)
{
   LCD_command(0x21);		      // SSD1306_COLUMNADDR
   LCD_command(column_start);     // column start
   LCD_command(column_end);       // column end
   LCD_command(0x22);             // SSD1306_PAGEADDR
   LCD_command(page_start);       // page start
   LCD_command(page_end);         // page end (4 pages for 32 rows OLED)
   
   lcd_buff[0]= DATA_MODE;              
   
   HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDRESS, lcd_buff, BUFF_SIZE,50);
}
//----------------------------------------------------------------------------------
void LCD_Chr(char ch)
{
   uint8_t i;   
   lcd_buff_idx=(y_cur*128 + x_cur*6) + 1;    
   if ((ch >= 0x20)&&(ch <= 0x7F)) ch -= 32;
   else ch = 95;   
   for (i = 0; i < 5; i++)
   {
        lcd_buff[lcd_buff_idx++] = font[ch][i];       
   }
   lcd_buff[lcd_buff_idx++] = 0x00;  
   x_cur++;
   if (x_cur > 20)
   {
      x_cur = 0;
      y_cur++;
      if (y_cur > 3)
      {
         y_cur = 0;
      }
   }
}
//----------------------------------------------------------------------------------
void LCD_2xChr(char ch)
{
   uint8_t	i, c;
   uint8_t	b1, b2;  
   uint16_t	tmpIdx;
   
   lcd_buff_idx = (y_cur*128+x_cur*6)+1;
   tmpIdx = lcd_buff_idx-128;
   if ((ch >= 0x20)&&(ch <= 0x7F)) ch -= 32;
   else ch = 95;
   
   for ( i = 0; i < 5; i++ ){
      c = font[ch][i]; 
      b1  = (c&0x01)*3;
      b1 |= (c&0x02)*6;
      b1 |= (c&0x04)*12;
      b1 |= (c&0x08)*24;
      
      c >>= 4;
      b2  = (c&0x01)*3;
      b2 |= (c&0x02)*6;
      b2 |= (c&0x04)*12;
      b2 |= (c&0x08)*24;
      
      lcd_buff[tmpIdx++] = b1;
      lcd_buff[tmpIdx++] = b1;
      lcd_buff[tmpIdx+126] = b2;
      lcd_buff[tmpIdx+127] = b2;
   }
   lcd_buff_idx = (lcd_buff_idx + 11) % BUFF_SIZE;
   lcd_buff[lcd_buff_idx] = 0x00;    
   if(lcd_buff_idx == (BUFF_SIZE-1)) lcd_buff_idx = 1;
   else lcd_buff_idx++;
   x_cur = x_cur + 2;
   if (x_cur > 19){
      x_cur = 0;
      y_cur = y_cur + 2;
      if (y_cur > 3){
         y_cur = 0;
      }
   }
}
//----------------------------------------------------------------------------------
void LCD_FStr(const char *dataPtr, uint8_t x, uint8_t y)
{
   LCD_GotoXY(x,y);
   while(*dataPtr != 0)
   {
      LCD_Chr(*dataPtr);
      dataPtr++;
   }
}
////----------------------------------------------------
void LCD_2xFStr(const char *dataPtr, uint8_t x, uint8_t y)
{
   LCD_GotoXY(x,y);
   while(*dataPtr!=0)
   {
      LCD_2xChr(*dataPtr);
      dataPtr++;
   }
}
////----------------------------------------------------
void LCD_GotoXY(uint8_t x,uint8_t y)
{
   if((x < 21)&&(y < (page_end+1)))
   {
      x_cur = x;
      y_cur = y;
   }
}
////----------------------------------------------------
////  This function adds a battery symbol to the lcd buffer
////----------------------------------------------------
void LCD_Batt(int val, uint8_t x, uint8_t y)
{   
   uint8_t bat_arr[] = { 0x1C, 0x1C, 0x7F, 0x41, 0x41, 0x41, 0x41, 
                    0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 
                    0x41, 0x41, 0x41, 0x41, 0x7F 
                  };
   if(x == 0) 
   {
       x = sizeof(bat_arr);
   }
   uint8_t cnt_val = sizeof(bat_arr);
   val -=3218;
   while(val > 0)
   {      
      bat_arr[cnt_val--]= 0x7F;
      val -=((4096-3218)/(sizeof(bat_arr)-3));  // array
   }
   lcd_buff_idx = (y*128 + x-18);
   for(uint8_t i = 0; i<sizeof(bat_arr); i++)
   {
      lcd_buff[lcd_buff_idx++]=bat_arr[i];
   }
}
////-----------------------------------------------------------------------
const uint8_t font [][5] = 
{
   { 0x00, 0x00, 0x00, 0x00, 0x00 },   //   0x20  32
   { 0x00, 0x00, 0x5F, 0x00, 0x00 },   // ! 0x21  33
   { 0x00, 0x07, 0x00, 0x07, 0x00 },   // " 0x22  34
   { 0x14, 0x7F, 0x14, 0x7F, 0x14 },   // # 0x23  35
   { 0x24, 0x2A, 0x7F, 0x2A, 0x12 },   // $ 0x24  36
   { 0x4C, 0x2C, 0x10, 0x68, 0x64 },   // % 0x25  37
   { 0x36, 0x49, 0x55, 0x22, 0x50 },   // & 0x26  38
   { 0x00, 0x05, 0x03, 0x00, 0x00 },   // ' 0x27  39
   { 0x00, 0x1C, 0x22, 0x41, 0x00 },   // ( 0x28  40
   { 0x00, 0x41, 0x22, 0x1C, 0x00 },   // ) 0x29  41
   { 0x14, 0x08, 0x3E, 0x08, 0x14 },   // * 0x2A  42
   { 0x08, 0x08, 0x3E, 0x08, 0x08 },   // + 0x2B  43
   { 0x00, 0x00, 0x50, 0x30, 0x00 },   // , 0x2C  44
   { 0x00, 0x08, 0x08, 0x08, 0x00 },   // - 0x2D  45
   { 0x00, 0x60, 0x60, 0x00, 0x00 },   // . 0x2E  46
   { 0x20, 0x10, 0x08, 0x04, 0x02 },   // / 0x2F  47
   { 0x7F, 0x41, 0x41, 0x41, 0x7F },   // 0 0x30  48
   { 0x00, 0x02, 0x7F, 0x00, 0x00 },   // 1 0x31  49
   { 0x79, 0x49, 0x49, 0x49, 0x4F },   // 2 0x32  50
   { 0x41, 0x49, 0x49, 0x49, 0x7F },   // 3 0x33  51
   { 0x0F, 0x08, 0x08, 0x08, 0x7F },   // 4 0x34  52
   { 0x4F, 0x49, 0x49, 0x49, 0x79 },   // 5 0x35  53
   { 0x7F, 0x49, 0x49, 0x49, 0x79 },   // 6 0x36  54
   { 0x01, 0x01, 0x01, 0x01, 0x7F },   // 7 0x37  55
   { 0x7F, 0x49, 0x49, 0x49, 0x7F },   // 8 0x38  56
   { 0x4F, 0x49, 0x49, 0x49, 0x7F },   // 9 0x39  57
   { 0x00, 0x36, 0x36, 0x00, 0x00 },   // : 0x3A  58
   { 0x00, 0x56, 0x36, 0x00, 0x00 },   // ; 0x3B  59
   { 0x08, 0x14, 0x22, 0x41, 0x00 },   // < 0x3C  60
   { 0x14, 0x14, 0x14, 0x14, 0x14 },   // = 0x3D  61
   { 0x00, 0x41, 0x22, 0x14, 0x08 },   // > 0x3E  62
   { 0x02, 0x01, 0x51, 0x09, 0x06 },   // ? 0x3F  63
   { 0x32, 0x49, 0x79, 0x41, 0x3E },   // @ 0x40  64
   { 0x7E, 0x11, 0x11, 0x11, 0x7E },   // A 0x41  65
   { 0x7F, 0x49, 0x49, 0x49, 0x36 },   // B 0x42  66
   { 0x3E, 0x41, 0x41, 0x41, 0x22 },   // C 0x43  67
   { 0x7F, 0x41, 0x41, 0x22, 0x1C },   // D 0x44  68
   { 0x7F, 0x49, 0x49, 0x49, 0x41 },   // E 0x45  69
   { 0x7F, 0x09, 0x09, 0x09, 0x01 },   // F 0x46  70
   { 0x3E, 0x41, 0x49, 0x49, 0x7A },   // G 0x47  71
   { 0x7F, 0x08, 0x08, 0x08, 0x7F },   // H 0x48  72
   { 0x00, 0x41, 0x7F, 0x41, 0x00 },   // I 0x49  73
   { 0x20, 0x40, 0x41, 0x3F, 0x01 },   // J 0x4A  74
   { 0x7F, 0x08, 0x14, 0x22, 0x41 },   // K 0x4B  75
   { 0x7F, 0x40, 0x40, 0x40, 0x40 },   // L 0x4C  76
   { 0x7F, 0x02, 0x0C, 0x02, 0x7F },   // M 0x4D  77
   { 0x7F, 0x04, 0x08, 0x10, 0x7F },   // N 0x4E  78
   { 0x3E, 0x41, 0x41, 0x41, 0x3E },   // O 0x4F  79
   { 0x7F, 0x09, 0x09, 0x09, 0x06 },   // P 0x50  80
   { 0x3E, 0x41, 0x51, 0x21, 0x5E },   // Q 0x51  81
   { 0x7F, 0x09, 0x19, 0x29, 0x46 },   // R 0x52  82
   { 0x46, 0x49, 0x49, 0x49, 0x31 },   // S 0x53  83
   { 0x01, 0x01, 0x7F, 0x01, 0x01 },   // T 0x54  84
   { 0x3F, 0x40, 0x40, 0x40, 0x3F },   // U 0x55  85
   { 0x1F, 0x20, 0x40, 0x20, 0x1F },   // V 0x56  86
   { 0x3F, 0x40, 0x38, 0x40, 0x3F },   // W 0x57  87
   { 0x63, 0x14, 0x08, 0x14, 0x63 },   // X 0x58  88
   { 0x07, 0x08, 0x70, 0x08, 0x07 },   // Y 0x59  89
   { 0x61, 0x51, 0x49, 0x45, 0x43 },   // Z 0x5A  90
   { 0x00, 0x7F, 0x41, 0x41, 0x00 },   // [ 0x5B  91
   { 0x02, 0x04, 0x08, 0x10, 0x20 },   // \ 0x5C  92
   { 0x00, 0x41, 0x41, 0x7F, 0x00 },   // ] 0x5D  93
   { 0x04, 0x02, 0x01, 0x02, 0x04 },   // ^ 0x5E  94
   { 0x40, 0x40, 0x40, 0x40, 0x40 },   // _ 0x5F  95
   { 0x00, 0x01, 0x02, 0x04, 0x00 },   // ` 0x60  96
   { 0x20, 0x54, 0x54, 0x54, 0x78 },   // a 0x61  97
   { 0x7F, 0x48, 0x44, 0x44, 0x38 },   // b 0x62  98
   { 0x38, 0x44, 0x44, 0x44, 0x00 },   // c 0x63  99
   { 0x38, 0x44, 0x44, 0x48, 0x7F },   // d 0x64 100
   { 0x38, 0x54, 0x54, 0x54, 0x18 },   // e 0x65 101
   { 0x08, 0x7E, 0x09, 0x01, 0x02 },   // f 0x66 102
   { 0x0C, 0x52, 0x52, 0x52, 0x3E },   // g 0x67 103
   { 0x7F, 0x08, 0x04, 0x04, 0x78 },   // h 0x68 104
   { 0x00, 0x44, 0x7D, 0x40, 0x00 },   // i 0x69 105
   { 0x20, 0x40, 0x44, 0x3D, 0x00 },   // j 0x6A 106
   { 0x7F, 0x10, 0x28, 0x44, 0x00 },   // k 0x6B 107
   { 0x00, 0x41, 0x7F, 0x40, 0x00 },   // l 0x6C 108
   { 0x7C, 0x04, 0x18, 0x04, 0x78 },   // m 0x6D 109
   { 0x7C, 0x08, 0x04, 0x04, 0x78 },   // n 0x6E 110
   { 0x38, 0x44, 0x44, 0x44, 0x38 },   // o 0x6F 111
   { 0x7C, 0x14, 0x14, 0x14, 0x08 },   // p 0x70 112
   { 0x08, 0x14, 0x14, 0x18, 0x7C },   // q 0x71 113
   { 0x7C, 0x08, 0x04, 0x04, 0x08 },   // r 0x72 114
   { 0x48, 0x54, 0x54, 0x54, 0x20 },   // s 0x73 115
   { 0x04, 0x3F, 0x44, 0x40, 0x20 },   // t 0x74 116
   { 0x3C, 0x40, 0x40, 0x20, 0x7C },   // u 0x75 117
   { 0x1C, 0x20, 0x40, 0x20, 0x1C },   // v 0x76 118
   { 0x3C, 0x40, 0x30, 0x40, 0x3C },   // w 0x77 119
   { 0x44, 0x28, 0x10, 0x28, 0x44 },   // x 0x78 120
   { 0x0C, 0x50, 0x50, 0x50, 0x3C },   // y 0x79 121
   { 0x44, 0x64, 0x54, 0x4C, 0x44 },   // z 0x7A 122
   { 0x08, 0x1C, 0x36, 0x1C, 0x08 },   // { 0x7B 123
   { 0x00, 0x00, 0xFF, 0x00, 0x00 },   // | 0x7C 124
   { 0x0C, 0x3C, 0x36, 0x1E, 0x18 },   // } 0x7D 125
   { 0x18, 0x1E, 0x36, 0x3C, 0x0C },   // ~ 0x7E 126
   { 0x06, 0x09, 0x09, 0x06, 0x00 },   // heart 0x7F 127    
};





