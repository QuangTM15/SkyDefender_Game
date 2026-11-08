
#include "renderer.h"
#include "Arduino.h"
#include "lcd_tft.h"

#define RENDERER_SCREEN_WIDTH 240
#define RENDERER_SCREEN_HEIGHT 280
#define RENDERER_HUD_HEIGHT 28

static uint8_t hudMode = 0U;

void Renderer_Init(void)
{
    SPI_begin(SPI_ROLE_MASTER);
    TFT_Begin();
    TFT_FillScreen(TFT_BLACK);
    hudMode = 0U;
}

void Renderer_Clear(void)
{
    TFT_FillScreen(TFT_BLACK);
}

void Renderer_SetHUDMode(uint8_t enabled)
{
    hudMode = enabled;
}

void Renderer_FillRect(int16_t x, int16_t y,
                       uint16_t width, uint16_t height,
                       uint16_t color)
{
    int16_t left = x;
    int16_t top = y;
    int16_t right = x + (int16_t)width;
    int16_t bottom = y + (int16_t)height;
    int16_t minY = 0;

    if (0U == hudMode)
    {
        minY = RENDERER_HUD_HEIGHT;
    }

    if (left < 0)
        left = 0;
    if (top < minY)
        top = minY;
    if (right > RENDERER_SCREEN_WIDTH)
        right = RENDERER_SCREEN_WIDTH;
    if (bottom > RENDERER_SCREEN_HEIGHT)
        bottom = RENDERER_SCREEN_HEIGHT;

    if ((right > left) && (bottom > top))
    {
        TFT_FillRect(left, top,
                     (uint16_t)(right - left),
                     (uint16_t)(bottom - top),
                     color);
    }
}
