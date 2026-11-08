#include "enemy.h"

#define ENEMY_SCREEN_WIDTH 240
#define ENEMY_SCREEN_HEIGHT 280

typedef struct
{
    int16_t x;
    int16_t y;
    float positionY;
    uint8_t active;
} Enemy_t;

static Enemy_t enemies[ENEMY_MAX_COUNT];
static uint32_t randomState = 12345U;

static uint32_t Enemy_Random(void)
{
    randomState = randomState * 1664525U + 1013904223U;
    return randomState;
}

void Enemy_Init(void)
{
    uint8_t i;
    randomState = 12345U;
    for (i = 0U; i < ENEMY_MAX_COUNT; i++)
    {
        enemies[i].x = 0;
        enemies[i].y = 0;
        enemies[i].positionY = 0.0f;
        enemies[i].active = 0U;
    }
}

void Enemy_Spawn(uint8_t maxEnemies)
{
    uint8_t i;
    uint8_t count = 0U;
    if (maxEnemies > ENEMY_MAX_COUNT) maxEnemies = ENEMY_MAX_COUNT;
    for (i = 0U; i < ENEMY_MAX_COUNT; i++)
    {
        if (0U != enemies[i].active) count++;
    }
    if (count >= maxEnemies) return;
    for (i = 0U; i < ENEMY_MAX_COUNT; i++)
    {
        if (0U == enemies[i].active)
        {
            enemies[i].x = (int16_t)(Enemy_Random() % (ENEMY_SCREEN_WIDTH - ENEMY_WIDTH + 1U));
            enemies[i].y = -(int16_t)ENEMY_HEIGHT;
            enemies[i].positionY = (float)enemies[i].y;
            enemies[i].active = 1U;
            break;
        }
    }
}

void Enemy_Update(uint32_t deltaMs, uint16_t speed)
{
    uint8_t i;
    for (i = 0U; i < ENEMY_MAX_COUNT; i++)
    {
        if (0U != enemies[i].active)
        {
            enemies[i].positionY += (float)speed * (float)deltaMs / 1000.0f;
            enemies[i].y = (int16_t)enemies[i].positionY;
            if (enemies[i].y >= ENEMY_SCREEN_HEIGHT) enemies[i].active = 0U;
        }
    }
}

uint8_t Enemy_GetPosition(uint8_t index, int16_t *x, int16_t *y)
{
    uint8_t result = 0U;
    if ((index < ENEMY_MAX_COUNT) && (0 != x) && (0 != y) && (0U != enemies[index].active))
    {
        *x = enemies[index].x;
        *y = enemies[index].y;
        result = 1U;
    }
    return result;
}

void Enemy_Remove(uint8_t index)
{
    if (index < ENEMY_MAX_COUNT) enemies[index].active = 0U;
}
