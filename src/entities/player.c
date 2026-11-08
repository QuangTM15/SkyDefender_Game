#include "player.h"

#define PLAYER_START_X 90
#define PLAYER_START_Y 210
#define PLAYER_MAX_X (240 - PLAYER_WIDTH)
#define PLAYER_MAX_Y (280 - PLAYER_HEIGHT)

void Player_Init(Player_t *player)
{
    if (0 != player)
    {
        player->x = PLAYER_START_X;
        player->y = PLAYER_START_Y;
    }
}

void Player_SetPosition(Player_t *player, int16_t x, int16_t y)
{
    if (0 != player)
    {
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x > PLAYER_MAX_X) x = PLAYER_MAX_X;
        if (y > PLAYER_MAX_Y) y = PLAYER_MAX_Y;
        player->x = x;
        player->y = y;
    }
}
