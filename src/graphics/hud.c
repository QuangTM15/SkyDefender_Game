
#include "hud.h"
#include "Arduino.h"
#include "graphics/renderer.h"
#include "lcd_tft.h"

#define HUD_HEART_COLOR 0xF800U
#define HUD_BG_COLOR 0x0000U
#define HUD_SCORE_COLOR 0xFFE0U
#define HUD_LABEL_COLOR 0x8410U

#define HUD_SCORE_X 174
#define HUD_SCORE_Y 14
#define HUD_DIGIT_SCALE 2U
#define HUD_DIGIT_WIDTH 6U
#define HUD_DIGIT_HEIGHT 10U
#define HUD_DIGIT_SPACING 2U
#define HUD_SCORE_DIGITS 6U
#define HUD_SCORE_MAX 999999UL

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
    {7U, 5U, 7U, 1U, 7U}};

static uint32_t displayedScore = UINT32_MAX;
static uint32_t targetScore = 0U;
static uint8_t scorePosition = 0U;
static uint8_t scoreRow = 0U;
static uint8_t scoreUpdating = 0U;

static void HUD_DrawHeart(int16_t x, int16_t y, uint16_t color)
{
    Renderer_FillRect(x + 2, y, 4U, 2U, color);
    Renderer_FillRect(x + 8, y, 4U, 2U, color);
    Renderer_FillRect(x, y + 2, 14U, 4U, color);
    Renderer_FillRect(x + 2, y + 6, 10U, 2U, color);
    Renderer_FillRect(x + 4, y + 8, 6U, 2U, color);
    Renderer_FillRect(x + 6, y + 10, 2U, 2U, color);
}

static uint8_t HUD_GetDigit(uint32_t value, uint8_t position)
{
    uint8_t digit = 0U;
    uint8_t i;
    for (i = 0U; i <= position; i++)
    {
        digit = (uint8_t)(value % 10UL);
        value /= 10UL;
    }
    return digit;
}

static uint8_t HUD_IsVisible(uint32_t value, uint8_t position)
{
    uint8_t visible = 0U;
    uint32_t divisor = 1UL;
    uint8_t i;
    for (i = 0U; i < position; i++)
    {
        divisor *= 10UL;
    }
    if ((0U == position) || (value >= divisor))
    {
        visible = 1U;
    }
    return visible;
}

static void HUD_DrawDigitRow(uint8_t digit, uint8_t position, uint8_t row, uint8_t visible)
{
    int16_t x = HUD_SCORE_X + (int16_t)((HUD_SCORE_DIGITS - 1U - position) * (HUD_DIGIT_WIDTH + HUD_DIGIT_SPACING));
    int16_t y = HUD_SCORE_Y + (int16_t)(row * HUD_DIGIT_SCALE);
    uint8_t col;
    uint8_t bits = 0U;
    if (0U != visible)
    {
        bits = digitFont[digit][row];
    }
    for (col = 0U; col < 3U; col++)
    {
        uint16_t color = HUD_BG_COLOR;
        if (0U != (bits & (1U << (2U - col))))
        {
            color = HUD_SCORE_COLOR;
        }
        Renderer_FillRect(x + (int16_t)(col * HUD_DIGIT_SCALE), y, HUD_DIGIT_SCALE, HUD_DIGIT_SCALE, color);
    }
}

void HUD_Reset(void)
{
    displayedScore = UINT32_MAX;
    targetScore = 0U;
    scorePosition = 0U;
    scoreRow = 0U;
    scoreUpdating = 0U;
}

void HUD_DrawHealth(uint8_t health)
{
    uint8_t i;
    Renderer_SetHUDMode(1U);
    for (i = 0U; i < 3U; i++)
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

void HUD_DrawScore(uint32_t score)
{
    if (score > HUD_SCORE_MAX)
    {
        score = HUD_SCORE_MAX;
    }
    targetScore = score;
    if (displayedScore != targetScore)
    {
        scoreUpdating = 1U;
    }
}

void HUD_Update(void)
{
    uint8_t oldDigit;
    uint8_t newDigit;
    uint8_t oldVisible;
    uint8_t newVisible;

    if (0U == scoreUpdating)
    {
        return;
    }

    if (scorePosition >= HUD_SCORE_DIGITS)
    {
        displayedScore = targetScore;
        scorePosition = 0U;
        scoreRow = 0U;
        scoreUpdating = 0U;
        return;
    }

    oldDigit = HUD_GetDigit(displayedScore, scorePosition);
    newDigit = HUD_GetDigit(targetScore, scorePosition);
    oldVisible = HUD_IsVisible(displayedScore, scorePosition);
    newVisible = HUD_IsVisible(targetScore, scorePosition);

    if ((oldDigit != newDigit) || (oldVisible != newVisible) || (UINT32_MAX == displayedScore))
    {
        Renderer_SetHUDMode(1U);
        HUD_DrawDigitRow(newDigit, scorePosition, scoreRow, newVisible);
        Renderer_SetHUDMode(0U);
        scoreRow++;
        if (scoreRow >= 5U)
        {
            scoreRow = 0U;
            scorePosition++;
        }
    }
    else
    {
        scorePosition++;
        scoreRow = 0U;
    }
}
