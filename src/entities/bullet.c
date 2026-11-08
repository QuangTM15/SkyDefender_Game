#include "bullet.h"
#include "graphics/renderer.h"

#define BULLET_SPEED 180.0f
#define BULLET_COLOR 0xFFE0U
#define BULLET_BG_COLOR 0x0000U

static Bullet_t bullets[BULLET_MAX_COUNT];
static float bulletY[BULLET_MAX_COUNT];
static int16_t drawnX[BULLET_MAX_COUNT];
static int16_t drawnY[BULLET_MAX_COUNT];
static uint8_t drawn[BULLET_MAX_COUNT];

void Bullet_Init(void)
{
    for (uint8_t i = 0U; i < BULLET_MAX_COUNT; i++)
    {
        bullets[i].active = 0U;
        bullets[i].x = 0;
        bullets[i].y = 0;
        bulletY[i] = 0.0f;
        drawn[i] = 0U;
    }
}

void Bullet_Spawn(int16_t x, int16_t y)
{
    for (uint8_t i = 0U; i < BULLET_MAX_COUNT; i++)
    {
        if (0U == bullets[i].active)
        {
            bullets[i].x = x;
            bullets[i].y = y;
            bullets[i].active = 1U;
            bulletY[i] = (float)y;
            break;
        }
    }
}

void Bullet_Update(uint32_t deltaMs)
{
    for (uint8_t i = 0U; i < BULLET_MAX_COUNT; i++)
    {
        if (0U != bullets[i].active)
        {
            bulletY[i] -= BULLET_SPEED * (float)deltaMs / 1000.0f;
            bullets[i].y = (int16_t)bulletY[i];

            if ((bullets[i].y + (int16_t)BULLET_HEIGHT) <= 0)
            {
                bullets[i].active = 0U;
            }
        }
    }
}

void Bullet_Draw(void)
{
    for (uint8_t i = 0U; i < BULLET_MAX_COUNT; i++)
    {
        if (0U != drawn[i])
        {
            Renderer_FillRect(drawnX[i], drawnY[i], BULLET_WIDTH, BULLET_HEIGHT, BULLET_BG_COLOR);
            drawn[i] = 0U;
        }
    }

    for (uint8_t i = 0U; i < BULLET_MAX_COUNT; i++)
    {
        if (0U != bullets[i].active)
        {
            Renderer_FillRect(bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, BULLET_COLOR);
            drawnX[i] = bullets[i].x;
            drawnY[i] = bullets[i].y;
            drawn[i] = 1U;
        }
    }
}
