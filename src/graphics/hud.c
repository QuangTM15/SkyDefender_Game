#include "hud.h"
#include "graphics/renderer.h"

#define HUD_HEART_COLOR 0xF800U
#define HUD_BG_COLOR 0x0000U

static void HUD_DrawHeart(int16_t x, int16_t y, uint16_t color)
{
    Renderer_FillRect(x + 2, y, 4U, 2U, color);
    Renderer_FillRect(x + 8, y, 4U, 2U, color);
    Renderer_FillRect(x, y + 2, 14U, 4U, color);
    Renderer_FillRect(x + 2, y + 6, 10U, 2U, color);
    Renderer_FillRect(x + 4, y + 8, 6U, 2U, color);
    Renderer_FillRect(x + 6, y + 10, 2U, 2U, color);
}

void HUD_DrawHealth(uint8_t health)
{
    Renderer_SetHUDMode(1U);
    for (uint8_t i = 0U; i < 3U; i++)
    {
        uint16_t color = HUD_BG_COLOR;
        if (i < health)
        {
            color = HUD_HEART_COLOR;
        }
        HUD_DrawHeart(18 + (int16_t)i * 20, 4, color);
    }
    Renderer_SetHUDMode(0U);
}