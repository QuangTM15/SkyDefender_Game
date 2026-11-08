
#include "player.h"
#include "graphics/renderer.h"

#define PLAYER_START_X 90
#define PLAYER_START_Y 210
#define PLAYER_MAX_X (240 - PLAYER_WIDTH)
#define PLAYER_MAX_Y (280 - PLAYER_HEIGHT)

#define PLAYER_COLOR_BODY 0x07FF
#define PLAYER_COLOR_WING 0x001F
#define PLAYER_COLOR_BG 0x0000

#define PLAYER_MAX_RUNS 16U

typedef struct
{
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t color;
    uint8_t matched;
} Player_Rect_t;

static uint16_t Player_GetColor(const Player_t *player, int16_t x, int16_t y)
{
    int16_t dx = x - player->x;
    int16_t dy = y - player->y;
    uint16_t color = PLAYER_COLOR_BG;

    if ((dx >= 22) && (dx < 38) && (dy >= 0) && (dy < 38))
    {
        color = PLAYER_COLOR_BODY;
    }

    if ((dx >= 0) && (dx < 60) && (dy >= 17) && (dy < 29))
    {
        color = PLAYER_COLOR_WING;
    }

    if ((dx >= 18) && (dx < 42) && (dy >= 35) && (dy < 44))
    {
        color = PLAYER_COLOR_BODY;
    }

    return color;
}

static void Player_FlushRect(const Player_Rect_t *rect)
{
    if ((rect != 0) && (rect->width > 0U) && (rect->height > 0U))
    {
        Renderer_FillRect(rect->x, rect->y, rect->width, rect->height, rect->color);
    }
}

void Player_Init(Player_t *player)
{
    if (player != 0)
    {
        player->x = PLAYER_START_X;
        player->y = PLAYER_START_Y;
    }
}

void Player_SetPosition(Player_t *player, int16_t x, int16_t y)
{
    if (player != 0)
    {
        if (x < 0)
            x = 0;
        if (y < 0)
            y = 0;
        if (x > PLAYER_MAX_X)
            x = PLAYER_MAX_X;
        if (y > PLAYER_MAX_Y)
            y = PLAYER_MAX_Y;

        player->x = x;
        player->y = y;
    }
}

void Player_Draw(const Player_t *player)
{
    if (player != 0)
    {
        Renderer_FillRect(player->x + 22, player->y, 16, 38, PLAYER_COLOR_BODY);
        Renderer_FillRect(player->x, player->y + 17, 60, 12, PLAYER_COLOR_WING);
        Renderer_FillRect(player->x + 18, player->y + 35, 24, 9, PLAYER_COLOR_BODY);
    }
}

void Player_DrawDelta(const Player_t *previous, const Player_t *current)
{
    Player_Rect_t active[PLAYER_MAX_RUNS];
    Player_Rect_t next[PLAYER_MAX_RUNS];
    uint8_t activeCount = 0U;
    uint8_t nextCount;
    int16_t left, top, right, bottom;
    int16_t x, y, start;
    uint16_t oldColor, newColor, runColor;

    if ((previous == 0) || (current == 0))
    {
        return;
    }

    left = previous->x < current->x ? previous->x : current->x;
    top = previous->y < current->y ? previous->y : current->y;
    right = previous->x > current->x ? previous->x + PLAYER_WIDTH : current->x + PLAYER_WIDTH;
    bottom = previous->y > current->y ? previous->y + PLAYER_HEIGHT : current->y + PLAYER_HEIGHT;

    for (y = top; y < bottom; y++)
    {
        nextCount = 0U;
        x = left;

        for (uint8_t i = 0U; i < activeCount; i++)
        {
            active[i].matched = 0U;
        }

        while (x < right)
        {
            oldColor = Player_GetColor(previous, x, y);
            newColor = Player_GetColor(current, x, y);

            if (oldColor == newColor)
            {
                x++;
                continue;
            }

            start = x;
            runColor = newColor;
            x++;

            while (x < right)
            {
                oldColor = Player_GetColor(previous, x, y);
                newColor = Player_GetColor(current, x, y);

                if ((oldColor == newColor) || (newColor != runColor))
                {
                    break;
                }

                x++;
            }

            uint8_t found = 0U;

            for (uint8_t i = 0U; i < activeCount; i++)
            {
                if ((active[i].x == start) &&
                    (active[i].width == (uint16_t)(x - start)) &&
                    (active[i].color == runColor))
                {
                    if (nextCount < PLAYER_MAX_RUNS)
                    {
                        next[nextCount] = active[i];
                        next[nextCount].height++;
                        nextCount++;
                        active[i].matched = 1U;
                        found = 1U;
                    }
                    break;
                }
            }

            if ((0U == found) && (nextCount < PLAYER_MAX_RUNS))
            {
                next[nextCount].x = start;
                next[nextCount].y = y;
                next[nextCount].width = (uint16_t)(x - start);
                next[nextCount].height = 1U;
                next[nextCount].color = runColor;
                next[nextCount].matched = 0U;
                nextCount++;
            }
        }

        for (uint8_t i = 0U; i < activeCount; i++)
        {
            if (0U == active[i].matched)
            {
                Player_FlushRect(&active[i]);
            }
        }

        activeCount = nextCount;

        for (uint8_t i = 0U; i < activeCount; i++)
        {
            active[i] = next[i];
        }
    }

    for (uint8_t i = 0U; i < activeCount; i++)
    {
        Player_FlushRect(&active[i]);
    }
}
