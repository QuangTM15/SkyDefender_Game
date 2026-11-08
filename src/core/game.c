#include "game.h"
#include "Arduino.h"
#include "lcd_tft.h"
#include "entities/player.h"
#include "entities/bullet.h"
#include "entities/enemy.h"
#include "graphics/renderer.h"
#include "graphics/scene.h"
#include "graphics/hud.h"
#include "graphics/menu.h"
#include "input/input.h"
#include "input/button.h"
#include "core/difficulty.h"
#include "core/collision.h"
#include "core/game_state.h"

#define GAME_FRAME_MS 30U
#define GAME_MOVE_SPEED 350.0f
#define GAME_MAX_DELTA_MS 100U
#define GAME_FIRE_INTERVAL_MS 200U
#define GAME_INVINCIBLE_MS 1000U
#define GAME_HIT_FLASH_MS 120U
#define GAME_INITIAL_HEALTH 3U
#define GAME_MAX_SCORE 999999UL

static Player_t player;
static float playerX = 90.0f;
static float playerY = 210.0f;
static uint32_t lastFrame = 0U;
static uint32_t lastFire = 0U;
static uint32_t lastSpawn = 0U;
static uint32_t gameStart = 0U;
static uint32_t lastHit = 0U;
static uint32_t pauseStart = 0U;
static uint32_t score = 0U;
static uint8_t health = GAME_INITIAL_HEALTH;
static uint8_t inputReady = 0U;
static uint8_t invincible = 0U;
static uint8_t healthDirty = 0U;
static uint8_t scoreDirty = 0U;

static void Game_DrawScoreLabel(void)
{
    TFT_SetTextColor(0x8410U);
    TFT_SetTextBackground(TFT_BLACK);
    TFT_SetTextSize(1U);
    TFT_SetCursor(202, 2);
    TFT_Print("SCORE");
    Renderer_SetHUDMode(1U);
    Renderer_FillRect(0, 27, 240U, 1U, 0x1082U);
    Renderer_SetHUDMode(0U);
}

static uint8_t Game_PlayerVisible(uint32_t now)
{
    uint8_t visible = 1U;
    if ((0U != invincible) && (0U != (((now - lastHit) / GAME_HIT_FLASH_MS) & 1U)))
    {
        visible = 0U;
    }
    return visible;
}

static void Game_Start(void)
{
    uint32_t now;
    Renderer_Clear();
    Scene_Reset();
    HUD_Reset();
    Player_Init(&player);
    Bullet_Init();
    Enemy_Init();
    playerX = (float)player.x;
    playerY = (float)player.y;
    health = GAME_INITIAL_HEALTH;
    score = 0U;
    invincible = 0U;
    healthDirty = 0U;
    scoreDirty = 0U;
    Scene_Render(&player, 1U);
    HUD_DrawHealth(health);
    Game_DrawScoreLabel();
    HUD_DrawScore(score);
    now = millis();
    gameStart = now;
    lastFrame = now;
    lastFire = now;
    lastSpawn = now;
    lastHit = now;
    GameState_Set(GAME_STATE_PLAYING);
}

static void Game_Resume(void)
{
    uint32_t now = millis();
    uint32_t pausedTime = now - pauseStart;
    gameStart += pausedTime;
    lastFire += pausedTime;
    lastSpawn += pausedTime;
    lastHit += pausedTime;
    Renderer_Clear();
    Scene_Reset();
    HUD_Reset();
    Scene_Render(&player, 1U);
    HUD_DrawHealth(health);
    Game_DrawScoreLabel();
    HUD_DrawScore(score);
    lastFrame = millis();
    GameState_Set(GAME_STATE_PLAYING);
}

static void Game_OpenMenu(void)
{
    Menu_ResetSelection();
    Menu_DrawStart();
    GameState_Set(GAME_STATE_MENU);
}

static void Game_OpenPause(void)
{
    pauseStart = millis();
    Menu_ResetSelection();
    Menu_DrawPause();
    GameState_Set(GAME_STATE_PAUSED);
}

static void Game_OpenGameOver(void)
{
    Menu_ResetSelection();
    Menu_DrawGameOver(score);
    GameState_Set(GAME_STATE_GAME_OVER);
}

static void Game_HandleMenu(uint8_t selectPressed, uint8_t nextPressed)
{
    GameState_t state = GameState_Get();
    if (GAME_STATE_HELP == state)
    {
        if ((0U != selectPressed) || (0U != nextPressed)) Game_OpenMenu();
        return;
    }
    if (0U != nextPressed)
    {
        Menu_NextOption();
        Menu_UpdateSelection();
    }
    if (0U == selectPressed) return;
    if (GAME_STATE_MENU == state)
    {
        if (MENU_OPTION_PLAY == Menu_GetSelection())
        {
            Game_Start();
        }
        else
        {
            Menu_DrawHelp();
            GameState_Set(GAME_STATE_HELP);
        }
    }
    else if (GAME_STATE_PAUSED == state)
    {
        if (PAUSE_OPTION_RESUME == Menu_GetSelection()) Game_Resume();
        else Game_Start();
    }
    else if (GAME_STATE_GAME_OVER == state)
    {
        if (GAMEOVER_OPTION_RETRY == Menu_GetSelection()) Game_Start();
        else Game_OpenMenu();
    }
}

static void Game_CheckCollisions(uint32_t now)
{
    int16_t bx;
    int16_t by;
    int16_t ex;
    int16_t ey;
    uint8_t b;
    uint8_t e;

    for (b = 0U; b < BULLET_MAX_COUNT; b++)
    {
        if (0U == Bullet_GetPosition(b, &bx, &by)) continue;
        for (e = 0U; e < ENEMY_MAX_COUNT; e++)
        {
            if (0U == Enemy_GetPosition(e, &ex, &ey)) continue;
            if (0U != Collision_Check(bx, by, BULLET_WIDTH, BULLET_HEIGHT,
                                     ex, ey, ENEMY_WIDTH, ENEMY_HEIGHT))
            {
                Bullet_Remove(b);
                Enemy_Remove(e);
                if (score <= GAME_MAX_SCORE - 10UL) score += 10U;
                scoreDirty = 1U;
                break;
            }
        }
    }
    if ((0U != invincible) && ((now - lastHit) >= GAME_INVINCIBLE_MS)) invincible = 0U;
    for (e = 0U; e < ENEMY_MAX_COUNT; e++)
    {
        if (0U == Enemy_GetPosition(e, &ex, &ey)) continue;
        if (0U != Collision_Check(player.x, player.y, PLAYER_WIDTH, PLAYER_HEIGHT,
                                 ex, ey, ENEMY_WIDTH, ENEMY_HEIGHT))
        {
            Enemy_Remove(e);
            if ((0U == invincible) && (health > 0U))
            {
                health--;
                lastHit = now;
                invincible = 1U;
                healthDirty = 1U;
                if (0U == health)
                {
                    Game_OpenGameOver();
                    return;
                }
            }
        }
    }
}

void Game_Init(void)
{
    Renderer_Init();
    Button_Init();
    GameState_Init();
    Menu_Init();
    TFT_SetTextColor(TFT_CYAN);
    TFT_SetTextBackground(TFT_BLACK);
    TFT_SetTextSize(2U);
    TFT_SetCursor(31, 99);
    TFT_Print("CALIBRATING");
    TFT_SetTextColor(TFT_WHITE);
    TFT_SetTextSize(1U);
    TFT_SetCursor(44, 145);
    TFT_Print("KEEP BOARD STILL");
    inputReady = Input_Init();
    if (0U != inputReady)
    {
        delay(2000U);
        inputReady = Input_Calibrate();
    }
    if (0U == inputReady)
    {
        TFT_FillScreen(TFT_BLACK);
        TFT_SetTextColor(TFT_RED);
        TFT_SetTextBackground(TFT_BLACK);
        TFT_SetTextSize(2U);
        TFT_SetCursor(39, 109);
        TFT_Print("MPU ERROR");
        delay(1500U);
    }
    Game_OpenMenu();
}

void Game_Update(void)
{
    uint32_t now;
    uint32_t delta;
    uint8_t selectPressed;
    uint8_t nextPressed;
    Input_Acceleration_t acceleration;
    Difficulty_Config_t difficulty;

    Button_Update();
    selectPressed = Button_SelectPressed();
    nextPressed = Button_NextPressed();
    if (GAME_STATE_PLAYING != GameState_Get())
    {
        Game_HandleMenu(selectPressed, nextPressed);
        return;
    }
    if (0U != selectPressed)
    {
        Game_OpenPause();
        return;
    }
    now = millis();
    delta = now - lastFrame;
    if (delta < GAME_FRAME_MS) return;
    lastFrame = now;
    if (delta > GAME_MAX_DELTA_MS) delta = GAME_MAX_DELTA_MS;
    Difficulty_GetConfig(now - gameStart, &difficulty);
    if ((0U != inputReady) && (0U != Input_ReadAcceleration(&acceleration)))
    {
        playerX -= acceleration.accelY * GAME_MOVE_SPEED * (float)delta / 1000.0f;
        playerY -= acceleration.accelX * GAME_MOVE_SPEED * (float)delta / 1000.0f;
        if (playerX < 0.0f) playerX = 0.0f;
        if (playerY < 0.0f) playerY = 0.0f;
        if (playerX > (float)(240 - PLAYER_WIDTH)) playerX = (float)(240 - PLAYER_WIDTH);
        if (playerY > (float)(280 - PLAYER_HEIGHT)) playerY = (float)(280 - PLAYER_HEIGHT);
        Player_SetPosition(&player, (int16_t)playerX, (int16_t)playerY);
    }
    if ((now - lastFire) >= GAME_FIRE_INTERVAL_MS)
    {
        Bullet_Spawn(player.x + (PLAYER_WIDTH - BULLET_WIDTH) / 2,
                     player.y - (int16_t)BULLET_HEIGHT);
        lastFire = now;
    }
    if ((now - lastSpawn) >= difficulty.spawnInterval)
    {
        Enemy_Spawn(difficulty.maxEnemies);
        lastSpawn = now;
    }
    Bullet_Update(delta);
    Enemy_Update(delta, difficulty.enemySpeed);
    Game_CheckCollisions(now);
    if (GAME_STATE_PLAYING != GameState_Get()) return;
    Scene_Render(&player, Game_PlayerVisible(now));
    if (0U != healthDirty)
    {
        HUD_DrawHealth(health);
        healthDirty = 0U;
    }
    if (0U != scoreDirty)
    {
        HUD_DrawScore(score);
        scoreDirty = 0U;
    }
    HUD_Update();
}
