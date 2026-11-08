#include "hud.h"
#include "graphics/renderer.h"

#define HUD_HEART_COLOR 0xF800U
#define HUD_BG_COLOR 0x0000U
#define HUD_SCORE_COLOR 0xFFE0U
#define HUD_SCORE_X 174
#define HUD_SCORE_Y 14
#define HUD_SCORE_DIGITS 6U
#define HUD_SCORE_ROWS 5U
#define HUD_SCORE_MAX 999999UL
#define HUD_DIGIT_SCALE 2U
#define HUD_DIGIT_STEP 8U

static const uint8_t digitFont[10][5] = {
    {7U, 5U, 5U, 5U, 7U},
    {2U, 6U, 2U, 2U, 7U},
    {7U, 1U, 7U, 4U, 7U},
    {7U, 1U, 7U, 1U, 7U},
    {5U, 5U, 7U, 1U, 1U},
    {7U, 4U, 7U, 1U, 7U},
    {7U, 4U, 7U, 5U, 7U},
    {7U, 1U, 1U, 1U, 1U},
    {7U, 5U, 7U, 5U, 7U},
    {7U, 5U, 7U, 1U, 7U}
};

/* The displayed pixels, not a cached score value, are the source of truth. */
static uint8_t displayedRows[HUD_SCORE_DIGITS][HUD_SCORE_ROWS];
static uint8_t renderedHealth = 0xFFU;
static uint8_t scanIndex = 0U;
static uint32_t targetScore = 0U;

static void HUD_DrawHeart(int16_t x, int16_t y, uint16_t color)
{
    Renderer_FillRect(x + 2, y, 4U, 2U, color);
    Renderer_FillRect(x + 8, y, 4U, 2U, color);
    Renderer_FillRect(x, y + 2, 14U, 4U, color);
    Renderer_FillRect(x + 2, y + 6, 10U, 2U, color);
    Renderer_FillRect(x + 4, y + 8, 6U, 2U, color);
    Renderer_FillRect(x + 6, y + 10, 2U, 2U, color);
}

static uint8_t HUD_TargetRow(uint8_t position, uint8_t row)
{
    uint32_t divisor = 1UL;
    uint8_t i;
    for (i = 0U; i < position; i++) divisor *= 10UL;
    if ((position > 0U) && (targetScore < divisor)) return 0U;
    return digitFont[(targetScore / divisor) % 10UL][row];
}

void HUD_Reset(void)
{
    uint8_t position;
    uint8_t row;
    targetScore = 0U;
    scanIndex = 0U;
    renderedHealth = 0xFFU;
    for (position = 0U; position < HUD_SCORE_DIGITS; position++)
    {
        for (row = 0U; row < HUD_SCORE_ROWS; row++) displayedRows[position][row] = 0U;
    }
}

void HUD_DrawHealth(uint8_t health)
{
    uint8_t i;
    Renderer_SetHUDMode(1U);
    for (i = 0U; i < 3U; i++)
    {
        uint8_t previous = (uint8_t)(0xFFU != renderedHealth && i < renderedHealth);
        uint8_t current = (uint8_t)(i < health);
        if ((0xFFU == renderedHealth) || (previous != current))
        {
            HUD_DrawHeart(18 + (int16_t)(i * 20U), 4, (0U != current) ? HUD_HEART_COLOR : HUD_BG_COLOR);
        }
    }
    renderedHealth = health;
    Renderer_SetHUDMode(0U);
}

void HUD_DrawScore(uint32_t score)
{
    targetScore = (score > HUD_SCORE_MAX) ? HUD_SCORE_MAX : score;
}

void HUD_Update(void)
{
    uint8_t attempt;
    for (attempt = 0U; attempt < HUD_SCORE_DIGITS * HUD_SCORE_ROWS; attempt++)
    {
        uint8_t position = (uint8_t)(scanIndex / HUD_SCORE_ROWS);
        uint8_t row = (uint8_t)(scanIndex % HUD_SCORE_ROWS);
        uint8_t previous = displayedRows[position][row];
        uint8_t desired = HUD_TargetRow(position, row);
        scanIndex = (uint8_t)((scanIndex + 1U) % (HUD_SCORE_DIGITS * HUD_SCORE_ROWS));
        if (previous != desired)
        {
            int16_t x = HUD_SCORE_X + (int16_t)((HUD_SCORE_DIGITS - 1U - position) * HUD_DIGIT_STEP);
            int16_t y = HUD_SCORE_Y + (int16_t)(row * HUD_DIGIT_SCALE);
            uint8_t col;
            Renderer_SetHUDMode(1U);
            for (col = 0U; col < 3U; col++)
            {
                uint8_t mask = (uint8_t)(1U << (2U - col));
                if (0U != ((previous ^ desired) & mask))
                {
                    uint16_t color = (0U != (desired & mask)) ? HUD_SCORE_COLOR : HUD_BG_COLOR;
                    Renderer_FillRect(x + (int16_t)(col * HUD_DIGIT_SCALE), y,
                                      HUD_DIGIT_SCALE, HUD_DIGIT_SCALE, color);
                }
            }
            Renderer_SetHUDMode(0U);
            displayedRows[position][row] = desired;
            break;
        }
    }
}
