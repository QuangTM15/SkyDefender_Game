#include "game_state.h"

static GameState_t currentState = GAME_STATE_MENU;

void GameState_Init(void)
{
    currentState = GAME_STATE_MENU;
}

void GameState_Set(GameState_t state)
{
    currentState = state;
}

GameState_t GameState_Get(void)
{
    return currentState;
}
