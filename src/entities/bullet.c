#include "bullet.h"

#define BULLET_SPEED 180.0f

static Bullet_t bullets[BULLET_MAX_COUNT];
static float bulletY[BULLET_MAX_COUNT];

void Bullet_Init(void)
{
    uint8_t i;
    for (i = 0U; i < BULLET_MAX_COUNT; i++)
    {
        bullets[i].active = 0U;
        bullets[i].x = 0;
        bullets[i].y = 0;
        bulletY[i] = 0.0f;
    }
}

void Bullet_Spawn(int16_t x, int16_t y)
{
    uint8_t i;
    for (i = 0U; i < BULLET_MAX_COUNT; i++)
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
    uint8_t i;
    for (i = 0U; i < BULLET_MAX_COUNT; i++)
    {
        if (0U != bullets[i].active)
        {
            bulletY[i] -= BULLET_SPEED * (float)deltaMs / 1000.0f;
            bullets[i].y = (int16_t)bulletY[i];
            if (bullets[i].y + (int16_t)BULLET_HEIGHT <= 0) bullets[i].active = 0U;
        }
    }
}

uint8_t Bullet_GetPosition(uint8_t index, int16_t *x, int16_t *y)
{
    uint8_t result = 0U;
    if ((index < BULLET_MAX_COUNT) && (0 != x) && (0 != y) && (0U != bullets[index].active))
    {
        *x = bullets[index].x;
        *y = bullets[index].y;
        result = 1U;
    }
    return result;
}

void Bullet_Remove(uint8_t index)
{
    if (index < BULLET_MAX_COUNT) bullets[index].active = 0U;
}
