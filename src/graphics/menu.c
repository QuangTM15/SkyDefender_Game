#include "menu.h"
#include "Arduino.h"
#include "lcd_tft.h"

#define MENU_BG TFT_BLACK
#define MENU_WHITE TFT_WHITE
#define MENU_CYAN 0x07FFU
#define MENU_YELLOW TFT_YELLOW
#define MENU_RED 0xF800U
#define MENU_GRAY 0x8410U
#define MENU_DARK 0x1082U
#define MENU_OPTION_X 36
#define MENU_OPTION_WIDTH 168U
#define MENU_OPTION_HEIGHT 32U
#define MENU_OPTION_Y1 164
#define MENU_OPTION_Y2 204
#define MENU_MARKER_X 39
#define MENU_MARKER_WIDTH 4U
#define MENU_MARKER_HEIGHT 24U

typedef enum
{
    MENU_SCREEN_START = 0,
    MENU_SCREEN_PAUSE,
    MENU_SCREEN_GAMEOVER,
    MENU_SCREEN_HELP
} MenuScreen_t;

static uint8_t selection = 0U;
static MenuScreen_t currentScreen = MENU_SCREEN_START;

static void Menu_PrintText(int16_t x, int16_t y, const char *text, uint16_t color, uint8_t size)
{
    TFT_SetTextColor(color);
    TFT_SetTextBackground(MENU_BG);
    TFT_SetTextSize(size);
    TFT_SetCursor(x, y);
    TFT_Print(text);
}

static void Menu_DrawFrame(uint16_t accent)
{
    TFT_FillScreen(MENU_BG);
    TFT_FillRect(10, 10, 220U, 2U, accent);
    TFT_FillRect(10, 268, 220U, 2U, accent);
    TFT_FillRect(10, 10, 2U, 260U, accent);
    TFT_FillRect(228, 10, 2U, 260U, accent);
}

static void Menu_DrawOption(uint8_t index, const char *text)
{
    int16_t y = (0U == index) ? MENU_OPTION_Y1 : MENU_OPTION_Y2;
    TFT_FillRect(MENU_OPTION_X, y, MENU_OPTION_WIDTH, MENU_OPTION_HEIGHT, MENU_BG);
    TFT_FillRect(MENU_OPTION_X + 12, y + 29, MENU_OPTION_WIDTH - 24U, 1U, MENU_DARK);
    Menu_PrintText(65, y + 8, text, MENU_WHITE, 2U);
}

static void Menu_DrawMarker(void)
{
    int16_t y = (0U == selection) ? MENU_OPTION_Y1 : MENU_OPTION_Y2;
    TFT_FillRect(MENU_MARKER_X, MENU_OPTION_Y1 + 4, MENU_MARKER_WIDTH, MENU_MARKER_HEIGHT, MENU_BG);
    TFT_FillRect(MENU_MARKER_X, MENU_OPTION_Y2 + 4, MENU_MARKER_WIDTH, MENU_MARKER_HEIGHT, MENU_BG);
    TFT_FillRect(MENU_MARKER_X, y + 4, MENU_MARKER_WIDTH, MENU_MARKER_HEIGHT, MENU_YELLOW);
}

static void Menu_DrawOptions(void)
{
    if (MENU_SCREEN_START == currentScreen)
    {
        Menu_DrawOption(0U, "PLAY");
        Menu_DrawOption(1U, "CONTROLS");
    }
    else if (MENU_SCREEN_PAUSE == currentScreen)
    {
        Menu_DrawOption(0U, "RESUME");
        Menu_DrawOption(1U, "RESTART");
    }
    else
    {
        Menu_DrawOption(0U, "RETRY");
        Menu_DrawOption(1U, "MENU");
    }
    Menu_DrawMarker();
}

static void Menu_DrawFooter(void)
{
    Menu_PrintText(32, 249, "NEXT: BTN1  OK: BTN0", MENU_GRAY, 1U);
}

void Menu_Init(void)
{
    selection = 0U;
    currentScreen = MENU_SCREEN_START;
}

void Menu_ResetSelection(void)
{
    selection = 0U;
}

void Menu_NextOption(void)
{
    selection ^= 1U;
}

void Menu_UpdateSelection(void)
{
    if (MENU_SCREEN_HELP != currentScreen) Menu_DrawMarker();
}

uint8_t Menu_GetSelection(void)
{
    return selection;
}

void Menu_DrawStart(void)
{
    currentScreen = MENU_SCREEN_START;
    Menu_DrawFrame(MENU_CYAN);
    Menu_PrintText(48, 40, "SKY", MENU_CYAN, 3U);
    Menu_PrintText(29, 76, "DEFENDER", MENU_WHITE, 3U);
    TFT_FillRect(40, 118, 160U, 2U, MENU_CYAN);
    Menu_PrintText(63, 135, "MAIN MENU", MENU_GRAY, 2U);
    Menu_DrawOptions();
    Menu_DrawFooter();
}

void Menu_DrawPause(void)
{
    currentScreen = MENU_SCREEN_PAUSE;
    Menu_DrawFrame(MENU_YELLOW);
    Menu_PrintText(60, 53, "PAUSED", MENU_YELLOW, 3U);
    TFT_FillRect(40, 102, 160U, 2U, MENU_YELLOW);
    TFT_FillRect(102, 120, 12U, 30U, MENU_YELLOW);
    TFT_FillRect(126, 120, 12U, 30U, MENU_YELLOW);
    Menu_DrawOptions();
    Menu_DrawFooter();
}

void Menu_DrawGameOver(uint32_t score)
{
    currentScreen = MENU_SCREEN_GAMEOVER;
    Menu_DrawFrame(MENU_RED);
    Menu_PrintText(30, 40, "GAME OVER", MENU_RED, 3U);
    TFT_FillRect(40, 88, 160U, 2U, MENU_RED);
    Menu_PrintText(69, 105, "FINAL SCORE", MENU_GRAY, 1U);
    TFT_SetTextColor(MENU_YELLOW);
    TFT_SetTextBackground(MENU_BG);
    TFT_SetTextSize(3U);
    TFT_SetCursor(80, 125);
    TFT_PrintInt((int32_t)score);
    Menu_DrawOptions();
    Menu_DrawFooter();
}

void Menu_DrawHelp(void)
{
    currentScreen = MENU_SCREEN_HELP;
    Menu_DrawFrame(MENU_CYAN);
    Menu_PrintText(23, 38, "CONTROLS", MENU_CYAN, 3U);
    TFT_FillRect(28, 82, 184U, 2U, MENU_CYAN);
    Menu_PrintText(30, 104, "TILT BOARD TO MOVE", MENU_WHITE, 1U);
    Menu_PrintText(30, 132, "WEAPON: AUTO FIRE", MENU_WHITE, 1U);
    Menu_PrintText(30, 160, "BTN0: PAUSE / OK", MENU_WHITE, 1U);
    Menu_PrintText(30, 188, "BTN1: NEXT OPTION", MENU_WHITE, 1U);
    Menu_PrintText(30, 236, "BTN0 / BTN1: BACK", MENU_YELLOW, 1U);
}
