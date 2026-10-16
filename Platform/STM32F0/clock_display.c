#include "clock_display.h"

#include "ssd1306.h"

void clock_display_init(void)
{
    LCD_Init();
}

void clock_display_render(const ClockView *view)
{
    static uint8_t applied_brightness = CLOCK_DISPLAY_BRIGHTNESS_DEFAULT_LEVEL;

    if ((view->brightness_level != applied_brightness) &&
        LCD_SetContrast((uint8_t)(view->brightness_level * 8U))) {
        applied_brightness = view->brightness_level;
    }

    /* Compose one complete frame in the SSD1306 page buffer before transmitting. */
    LCD_Clear();
    if (view->octahedron_mode) {
        LCD_DrawOctahedron(view->octahedron_angle,
                           view->octahedron_x,
                           view->octahedron_y);
        LCD_DrawCube(view->cube_angle, view->cube_x, view->cube_y);
        LCD_DrawCross(view->cross_angle, view->cross_x, view->cross_y);
        LCD_Update();
        return;
    }

    LCD_2xFStr(view->time, 2U, 2U);
    if (view->status[0] != '\0') {
        LCD_FStr(view->status, 0U, 0U);
    } else {
        LCD_FStr(view->weekday, 0U, 0U);
        LCD_FStr(view->date, 4U, 0U);
    }
    LCD_Batt(view->battery_millivolts, 120U, 0U);
    LCD_Thermometer(view->temperature_celsius,
                    view->temperature_valid,
                    120U,
                    0U);
    /* Shift the ticker one pixel per animation frame. */
    LCD_FScrollStr(view->ticker, view->ticker_pixel_offset, 3U, 120U);
    LCD_Update();
}
