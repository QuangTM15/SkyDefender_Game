#include "game.h"
#include "Arduino.h"
#include "entities/player.h"
#include "graphics/renderer.h"
#include "input/input.h"

#define GAME_FRAME_MS 30U
#define GAME_MOVE_SPEED 350.0f
#define GAME_MAX_DELTA_MS 100U

static Player_t player;
static float playerX = 90.0f;
static float playerY = 210.0f;
static uint32_t lastFrame = 0U;
static uint8_t inputReady = 0U;

void Game_Init(void)
{
    Renderer_Init();
    Player_Init(&player);
    playerX = (float)player.x;
    playerY = (float)player.y;
    inputReady = Input_Init();

    if (0U != inputReady)
    {
        delay(2000U);
        inputReady = Input_Calibrate();
    }

    Player_Draw(&player);
    lastFrame = millis();
}

void Game_Update(void)
{
    uint32_t now = millis();
    uint32_t delta = now - lastFrame;
    Input_Acceleration_t acceleration;
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
}
