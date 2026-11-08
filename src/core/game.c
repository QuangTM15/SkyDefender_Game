#include "game.h"
#include "Arduino.h"
#include "lcd_tft.h"
#include "entities/player.h"
#include "entities/bullet.h"
#include "entities/enemy.h"
#include "graphics/renderer.h"
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
#define GAME_INITIAL_HEALTH 3U

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
}

static void Game_Start(void)
{
    uint32_t now;

    Renderer_Clear();
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

    Player_Draw(&player);
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
    HUD_Reset();
    Player_Draw(&player);
    HUD_DrawHealth(health);
    Game_DrawScoreLabel();
    HUD_DrawScore(score);
    Bullet_Draw();
    Enemy_Draw();

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
    if (0U != nextPressed)
    {
        Menu_NextOption();
        Menu_UpdateSelection();
    }

    if (0U != selectPressed)
    {
        uint8_t selection = Menu_GetSelection();

        if (GAME_STATE_MENU == GameState_Get())
        {
            if (MENU_OPTION_PLAY == selection)
            {
                Game_Start();
            }
            else
            {
                Game_OpenMenu();
            }
        }
        else if (GAME_STATE_PAUSED == GameState_Get())
        {
            if (PAUSE_OPTION_RESUME == selection)
            {
                Game_Resume();
            }
            else
            {
                Game_Start();
            }
        }
        else if (GAME_STATE_GAME_OVER == GameState_Get())
        {
            if (GAMEOVER_OPTION_RETRY == selection)
            {
                Game_Start();
            }
            else
            {
                Game_OpenMenu();
            }
        }
    }
}

static void Game_CheckCollisions(uint32_t now)
{
    int16_t bx;
    int16_t by;
    int16_t ex;
    int16_t ey;

    for (uint8_t b = 0U; b < BULLET_MAX_COUNT; b++)
    {
        if (0U == Bullet_GetPosition(b, &bx, &by))
        {
            continue;
        }

        for (uint8_t e = 0U; e < ENEMY_MAX_COUNT; e++)
        {
            if (0U == Enemy_GetPosition(e, &ex, &ey))
            {
                continue;
            }

            if (0U != Collision_Check(bx, by, BULLET_WIDTH, BULLET_HEIGHT,
                                      ex, ey, ENEMY_WIDTH, ENEMY_HEIGHT))
            {
                Bullet_Remove(b);
                Enemy_Remove(e);
                score += 10U;
                scoreDirty = 1U;
                break;
            }
        }
    }

    if ((0U != invincible) && ((now - lastHit) >= GAME_INVINCIBLE_MS))
    {
        invincible = 0U;
    }

    for (uint8_t e = 0U; e < ENEMY_MAX_COUNT; e++)
    {
        if (0U == Enemy_GetPosition(e, &ex, &ey))
        {
            continue;
        }

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

    inputReady = Input_Init();

    if (0U != inputReady)
    {
        delay(2000U);
        inputReady = Input_Calibrate();
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
    Player_t previous;

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

    if (delta < GAME_FRAME_MS)
    {
        return;
    }

    lastFrame = now;

    if (delta > GAME_MAX_DELTA_MS)
    {
        delta = GAME_MAX_DELTA_MS;
    }

    Difficulty_GetConfig(now - gameStart, &difficulty);

    if ((0U != inputReady) && (0U != Input_ReadAcceleration(&acceleration)))
    {
        playerX -= acceleration.accelY * GAME_MOVE_SPEED * (float)delta / 1000.0f;
        playerY -= acceleration.accelX * GAME_MOVE_SPEED * (float)delta / 1000.0f;

        if (playerX < -8.0f)
        {
            playerX = -8.0f;
        }
        if (playerY < 0.0f)
        {
            playerY = 0.0f;
        }
        if (playerX > 180.0f)
        {
            playerX = 180.0f;
        }
        if (playerY > 236.0f)
        {
            playerY = 236.0f;
        }

        previous = player;
        Player_SetPosition(&player, (int16_t)playerX, (int16_t)playerY);

        if ((previous.x != player.x) || (previous.y != player.y))
        {
            Player_DrawDelta(&previous, &player);
        }
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

    if (GAME_STATE_PLAYING != GameState_Get())
    {
        return;
    }

    Bullet_Draw();
    Enemy_Draw();

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
