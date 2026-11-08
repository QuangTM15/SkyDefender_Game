#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>

#define PLAYER_WIDTH 60
#define PLAYER_HEIGHT 44

typedef struct
{
    int16_t x;
    int16_t y;
} Player_t;

void Player_Init(Player_t *player);
void Player_SetPosition(Player_t *player, int16_t x, int16_t y);

#endif
