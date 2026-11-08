#ifndef BULLET_H
#define BULLET_H

#include <stdint.h>

#define BULLET_MAX_COUNT 10U
#define BULLET_WIDTH 4U
#define BULLET_HEIGHT 10U

typedef struct
{
    int16_t x;
    int16_t y;
    uint8_t active;
} Bullet_t;

void Bullet_Init(void);
void Bullet_Spawn(int16_t x, int16_t y);
void Bullet_Update(uint32_t deltaMs);
void Bullet_Draw(void);
uint8_t Bullet_GetPosition(uint8_t index, int16_t *x, int16_t *y);
void Bullet_Remove(uint8_t index);

#endif
