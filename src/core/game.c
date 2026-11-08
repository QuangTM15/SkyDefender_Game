
#include "game.h"
#include "Arduino.h"
#include "entities/player.h"
#include "entities/bullet.h"
#include "entities/enemy.h"
#include "graphics/renderer.h"
#include "input/input.h"
#include "core/difficulty.h"

#define GAME_FRAME_MS 30U
#define GAME_MOVE_SPEED 350.0f
#define GAME_MAX_DELTA_MS 100U
#define GAME_FIRE_INTERVAL_MS 200U

static Player_t player;
static float playerX = 90.0f;
static float playerY = 210.0f;
static uint32_t lastFrame = 0U;
static uint32_t lastFire = 0U;
static uint32_t lastSpawn = 0U;
static uint32_t gameStart = 0U;
static uint8_t inputReady = 0U;

void Game_Init(void)
{
    Renderer_Init();
    Player_Init(&player);
    Bullet_Init();
    Enemy_Init();

    playerX = (float)player.x;
    playerY = (float)player.y;

    inputReady = Input_Init();

    if (0U != inputReady)
    {
        delay(2000U);
        inputReady = Input_Calibrate();
    }

    Player_Draw(&player);
    gameStart = millis();
    lastFrame = gameStart;
    lastFire = gameStart;
    lastSpawn = gameStart;
}

void Game_Update(void)
{
    uint32_t now = millis();
    uint32_t delta = now - lastFrame;
    Input_Acceleration_t acceleration;
    Difficulty_Config_t difficulty;
    Player_t previous;

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

        if (playerX < 0.0f)
            playerX = 0.0f;
        if (playerY < 0.0f)
            playerY = 0.0f;
        if (playerX > 180.0f)
            playerX = 180.0f;
        if (playerY > 236.0f)
            playerY = 236.0f;

        previous = player;
        Player_SetPosition(&player, (int16_t)playerX, (int16_t)playerY);

        if ((previous.x != player.x) || (previous.y != player.y))
        {
            Player_DrawDelta(&previous, &player);
        }
    }

    if ((now - lastFire) >= GAME_FIRE_INTERVAL_MS)
    {
        Bullet_Spawn(player.x + 8, player.y - (int16_t)BULLET_HEIGHT);
        Bullet_Spawn(player.x + PLAYER_WIDTH - 8 - BULLET_WIDTH, player.y - (int16_t)BULLET_HEIGHT);
        lastFire = now;
    }

    if ((now - lastSpawn) >= difficulty.spawnInterval)
    {
        Enemy_Spawn(difficulty.maxEnemies);
        lastSpawn = now;
    }

    Bullet_Update(delta);
    Enemy_Update(delta, difficulty.enemySpeed);
    Bullet_Draw();
    Enemy_Draw();
}
