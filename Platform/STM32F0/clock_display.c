#include "clock_display.h"

#include "ssd1306.h"

void clock_display_init(void)
{
    LCD_Init();
}

void clock_display_render(const ClockView *view)
{
    LCD_Clear();
    LCD_2xFStr(view->time, 2U, 2U);
    LCD_FStr(view->weekday, 0U, 0U);
    LCD_FStr(view->date, 4U, 0U);
    LCD_Batt(view->battery_millivolts, 120U, 0U);
    LCD_FStr(view->ticker, 0U, 3U);
    LCD_Update();
}
