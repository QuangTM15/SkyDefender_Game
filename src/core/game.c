#include "game.h"
#include "entities/player.h"
#include "graphics/renderer.h"

static Player_t player;

void Game_Init(void)
{
    Renderer_Init();
    Player_Init(&player);
    Player_Draw(&player);
}

void Game_Update(void)
{
}