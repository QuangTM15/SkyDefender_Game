#include "player.h"
#include "graphics/renderer.h"

#define PLAYER_START_X 90
#define PLAYER_START_Y 210

void Player_Init(Player_t *player)
{
    if (player != 0)
    {
        player->x = PLAYER_START_X;
        player->y = PLAYER_START_Y;
    }
}

void Player_SetPosition(Player_t *player, int16_t x, int16_t y)
{
    if (player != 0)
    {
        player->x = x;
        player->y = y;
    }
}

void Player_Draw(const Player_t *player)
{
    if (player != 0)
    {
        Renderer_FillRect(player->x + 22, player->y, 16, 38, 0x07FF);
        Renderer_FillRect(player->x, player->y + 17, 60, 12, 0x001F);
        Renderer_FillRect(player->x + 18, player->y + 35, 24, 9, 0x07FF);
    }
}
