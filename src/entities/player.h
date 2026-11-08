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
void Player_Draw(const Player_t *player);
void Player_DrawDelta(const Player_t *previous, const Player_t *current);

#endif
