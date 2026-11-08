#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdint.h>

typedef enum
{
    GAME_STATE_MENU = 0,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAME_OVER
} GameState_t;

void GameState_Init(void);
void GameState_Set(GameState_t state);
GameState_t GameState_Get(void);

#endif
