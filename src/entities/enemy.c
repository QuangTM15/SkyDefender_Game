
#include "enemy.h"
#include "graphics/renderer.h"

#define ENEMY_SCREEN_WIDTH 240
#define ENEMY_SCREEN_HEIGHT 280
#define ENEMY_COLOR_BODY 0xF800U
#define ENEMY_COLOR_CENTER 0xFD20U
#define ENEMY_COLOR_BG 0x0000U
#define ENEMY_LIMIT 3U

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t drawnY;
    float positionY;
    uint8_t active;
    uint8_t drawn;
} Enemy_t;

static Enemy_t enemies[ENEMY_MAX_COUNT];
static uint32_t randomState = 12345U;

static uint32_t Enemy_Random(void)
{
    randomState = randomState * 1664525U + 1013904223U;
    return randomState;
}

static void Enemy_Fill(int16_t x, int16_t y,
                       uint16_t width, uint16_t height,
                       uint16_t color)
{
    int16_t left = x;
    int16_t top = y;
    int16_t right = x + (int16_t)width;
    int16_t bottom = y + (int16_t)height;

    if (left < 0)
        left = 0;
    if (top < 0)
        top = 0;
    if (right > ENEMY_SCREEN_WIDTH)
        right = ENEMY_SCREEN_WIDTH;
    if (bottom > ENEMY_SCREEN_HEIGHT)
        bottom = ENEMY_SCREEN_HEIGHT;

    if ((right > left) && (bottom > top))
    {
        Renderer_FillRect(left, top,
                          (uint16_t)(right - left),
                          (uint16_t)(bottom - top),
                          color);
    }
}

static uint16_t Enemy_ColorAt(uint8_t band, int16_t y)
{
    if ((y < 0) || (y >= ENEMY_HEIGHT))
    {
        return ENEMY_COLOR_BG;
    }

    if ((y >= 8) && (y < 16))
    {
        if (band == 2U)
        {
            return ENEMY_COLOR_CENTER;
        }
        return ENEMY_COLOR_BODY;
    }

    if ((band >= 1U) && (band <= 3U))
    {
        return ENEMY_COLOR_BODY;
    }

    return ENEMY_COLOR_BG;
}

static void Enemy_DrawShape(int16_t x, int16_t y)
{
    Enemy_Fill(x + 4, y, 16U, 24U, ENEMY_COLOR_BODY);
    Enemy_Fill(x, y + 8, 24U, 8U, ENEMY_COLOR_BODY);
    Enemy_Fill(x + 8, y + 8, 8U, 8U, ENEMY_COLOR_CENTER);
}

static void Enemy_DrawDelta(int16_t x, int16_t oldY, int16_t newY)
{
    static const uint8_t offsets[5] = {0U, 4U, 8U, 16U, 20U};
    static const uint8_t widths[5] = {4U, 4U, 8U, 4U, 4U};

    int16_t top = oldY < newY ? oldY : newY;
    int16_t bottom = oldY > newY ? oldY : newY;
    bottom += ENEMY_HEIGHT;

    for (uint8_t band = 0U; band < 5U; band++)
    {
        int16_t start = top;
        uint16_t current = 0xFFFFU;

        for (int16_t y = top; y <= bottom; y++)
        {
            uint16_t color = 0xFFFFU;

            if (y < bottom)
            {
                uint16_t oldColor = Enemy_ColorAt(band, y - oldY);
                uint16_t newColor = Enemy_ColorAt(band, y - newY);

                if (oldColor != newColor)
                {
                    color = newColor;
                }
            }

            if (color != current)
            {
                if ((current != 0xFFFFU) && (y > start))
                {
                    Enemy_Fill(x + offsets[band], start,
                               widths[band],
                               (uint16_t)(y - start),
                               current);
                }

                current = color;
                start = y;
            }
        }
    }
}

static void Enemy_Erase(int16_t x, int16_t y)
{
    Enemy_Fill(x, y, ENEMY_WIDTH, ENEMY_HEIGHT, ENEMY_COLOR_BG);
}

void Enemy_Init(void)
{
    randomState = 12345U;

    for (uint8_t i = 0U; i < ENEMY_MAX_COUNT; i++)
    {
        enemies[i].x = 0;
        enemies[i].y = 0;
        enemies[i].drawnY = 0;
        enemies[i].positionY = 0.0f;
        enemies[i].active = 0U;
        enemies[i].drawn = 0U;
    }
}

void Enemy_Spawn(uint8_t maxEnemies)
{
    uint8_t count = 0U;
    uint8_t limit = maxEnemies;

    if (limit > ENEMY_LIMIT)
    {
        limit = ENEMY_LIMIT;
    }

    for (uint8_t i = 0U; i < ENEMY_MAX_COUNT; i++)
    {
        if (0U != enemies[i].active)
        {
            count++;
        }
    }

    if (count >= limit)
    {
        return;
    }

    for (uint8_t i = 0U; i < ENEMY_MAX_COUNT; i++)
    {
        if ((0U == enemies[i].active) &&
            (0U == enemies[i].drawn))
        {
            enemies[i].x = (int16_t)(Enemy_Random() %
                                     (ENEMY_SCREEN_WIDTH - ENEMY_WIDTH + 1U));
            enemies[i].y = -(int16_t)ENEMY_HEIGHT;
            enemies[i].positionY = (float)enemies[i].y;
            enemies[i].active = 1U;
            break;
        }
    }
}

void Enemy_Update(uint32_t deltaMs, uint16_t speed)
{
    for (uint8_t i = 0U; i < ENEMY_MAX_COUNT; i++)
    {
        if (0U != enemies[i].active)
        {
            enemies[i].positionY +=
                (float)speed * (float)deltaMs / 1000.0f;

            enemies[i].y = (int16_t)enemies[i].positionY;

            if (enemies[i].y >= ENEMY_SCREEN_HEIGHT)
            {
                enemies[i].active = 0U;
            }
        }
    }
}

void Enemy_Draw(void)
{
    for (uint8_t i = 0U; i < ENEMY_MAX_COUNT; i++)
    {
        if (0U != enemies[i].drawn)
        {
            if (0U == enemies[i].active)
            {
                Enemy_Erase(enemies[i].x, enemies[i].drawnY);
                enemies[i].drawn = 0U;
            }
            else if (enemies[i].y != enemies[i].drawnY)
            {
                Enemy_DrawDelta(enemies[i].x,
                                enemies[i].drawnY,
                                enemies[i].y);
                enemies[i].drawnY = enemies[i].y;
            }
        }
        else if ((0U != enemies[i].active) &&
                 (enemies[i].y + ENEMY_HEIGHT > 0) &&
                 (enemies[i].y < ENEMY_SCREEN_HEIGHT))
        {
            Enemy_DrawShape(enemies[i].x, enemies[i].y);
            enemies[i].drawnY = enemies[i].y;
            enemies[i].drawn = 1U;
        }
    }
}
