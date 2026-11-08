#ifndef MENU_H
#define MENU_H

#include <stdint.h>

typedef enum
{
    MENU_OPTION_PLAY = 0,
    MENU_OPTION_HELP
} MenuOption_t;

typedef enum
{
    PAUSE_OPTION_RESUME = 0,
    PAUSE_OPTION_RESTART
} PauseOption_t;

typedef enum
{
    GAMEOVER_OPTION_RETRY = 0,
    GAMEOVER_OPTION_MENU
} GameOverOption_t;

void Menu_Init(void);
void Menu_DrawStart(void);
void Menu_DrawPause(void);
void Menu_DrawGameOver(uint32_t score);
void Menu_DrawHelp(void);
void Menu_NextOption(void);
void Menu_ResetSelection(void);
void Menu_UpdateSelection(void);
uint8_t Menu_GetSelection(void);

#endif
