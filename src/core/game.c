#include "game.h"
#include "Arduino.h"
#include "entities/player.h"
#include "entities/bullet.h"
#include "entities/enemy.h"
#include "graphics/renderer.h"
#include "graphics/hud.h"
#include "input/input.h"
#include "core/difficulty.h"
#include "core/collision.h"

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
static uint32_t score = 0U;

static uint8_t health = GAME_INITIAL_HEALTH;
static uint8_t inputReady = 0U;
static uint8_t invincible = 0U;
static uint8_t gameOver = 0U;
static uint8_t healthDirty = 0U;

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

            if (0U != Collision_Check(
                          bx, by, BULLET_WIDTH, BULLET_HEIGHT,
                          ex, ey, ENEMY_WIDTH, ENEMY_HEIGHT))
            {
                Bullet_Remove(b);
                Enemy_Remove(e);
                score += 10U;
                break;
            }
        }
    }

    if ((0U != invincible) &&
        ((now - lastHit) >= GAME_INVINCIBLE_MS))
    {
        invincible = 0U;
    }

    for (uint8_t e = 0U; e < ENEMY_MAX_COUNT; e++)
    {
        if (0U == Enemy_GetPosition(e, &ex, &ey))
        {
            continue;
        }

        if (0U != Collision_Check(
                      player.x, player.y, PLAYER_WIDTH, PLAYER_HEIGHT,
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
                    gameOver = 1U;
                }
            }
        }
    }
}

void Game_Init(void)
{
    Renderer_Init();
    Player_Init(&player);
    Bullet_Init();
    Enemy_Init();

    playerX = (float)player.x;
    playerY = (float)player.y;

    health = GAME_INITIAL_HEALTH;
    score = 0U;
    invincible = 0U;
    gameOver = 0U;
    healthDirty = 0U;

    inputReady = Input_Init();

    if (0U != inputReady)
    {
        delay(2000U);
        inputReady = Input_Calibrate();
    }

    Player_Draw(&player);
    HUD_DrawHealth(health);

    gameStart = millis();
    lastFrame = gameStart;
    lastFire = gameStart;
    lastSpawn = gameStart;
    lastHit = gameStart;
}

void Game_Update(void)
{
    uint32_t now = millis();
    uint32_t delta = now - lastFrame;
    Input_Acceleration_t acceleration;
    Difficulty_Config_t difficulty;
    Player_t previous;

    if (0U != gameOver)
    {
        return;
    }

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

    if ((0U != inputReady) &&
        (0U != Input_ReadAcceleration(&acceleration)))
    {
        playerX -= acceleration.accelY *
                   GAME_MOVE_SPEED * (float)delta / 1000.0f;

        playerY -= acceleration.accelX *
                   GAME_MOVE_SPEED * (float)delta / 1000.0f;

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

        Player_SetPosition(&player,
                           (int16_t)playerX,
                           (int16_t)playerY);

        if ((previous.x != player.x) ||
            (previous.y != player.y))
        {
            Player_DrawDelta(&previous, &player);
        }
    }

    if ((now - lastFire) >= GAME_FIRE_INTERVAL_MS)
    {
        Bullet_Spawn(player.x + (PLAYER_WIDTH - BULLET_WIDTH) / 2, player.y - (int16_t)BULLET_HEIGHT);
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

    Bullet_Draw();
    Enemy_Draw();

    if (0U != healthDirty)
    {
        HUD_DrawHealth(health);
        healthDirty = 0U;
    }
}
