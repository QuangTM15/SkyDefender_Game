#ifndef DIFFICULTY_H
#define DIFFICULTY_H

#include <stdint.h>

typedef struct
{
    uint16_t spawnInterval;
    uint16_t enemySpeed;
    uint8_t maxEnemies;
} Difficulty_Config_t;

void Difficulty_GetConfig(uint32_t elapsedMs, Difficulty_Config_t *config);

#endif