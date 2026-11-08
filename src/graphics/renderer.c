#include "renderer.h"
#include "Arduino.h"
#include "lcd_tft.h"

void Renderer_Init(void)
{
    SPI_begin(SPI_ROLE_MASTER);
    TFT_Begin();
    TFT_FillScreen(TFT_BLACK);
}

void Renderer_Clear(void)
{
    TFT_FillScreen(TFT_BLACK);
}

void Renderer_FillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    TFT_FillRect(x, y, width, height, color);
}
