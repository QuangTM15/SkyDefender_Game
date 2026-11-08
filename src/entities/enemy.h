#ifndef ENEMY_H
#define ENEMY_H

#include <stdint.h>

#define ENEMY_MAX_COUNT 8U
#define ENEMY_WIDTH 24U
#define ENEMY_HEIGHT 24U

void Enemy_Init(void);
void Enemy_Spawn(uint8_t maxEnemies);
void Enemy_Update(uint32_t deltaMs, uint16_t speed);
void Enemy_Draw(void);

#endif