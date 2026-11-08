#include "scene.h"
#include "entities/bullet.h"
#include "entities/enemy.h"
#include "graphics/renderer.h"

#define SCENE_WIDTH 240
#define SCENE_HEIGHT 280
#define SCENE_TOP 28
#define SCENE_MAX_RUNS 48U
#define SCENE_BLACK 0x0000U
#define SCENE_PLAYER_BODY 0x07FFU
#define SCENE_PLAYER_WING 0x001FU
#define SCENE_BULLET 0xFFE0U
#define SCENE_ENEMY_BODY 0xF800U
#define SCENE_ENEMY_CENTER 0xFD20U

typedef struct
{
    int16_t x;
    int16_t y;
    uint8_t active;
} SceneSprite_t;

typedef struct
{
    Player_t player;
    uint8_t playerVisible;
    SceneSprite_t bullets[BULLET_MAX_COUNT];
    SceneSprite_t enemies[ENEMY_MAX_COUNT];
} SceneFrame_t;

typedef struct
{
    int16_t left;
    int16_t top;
    int16_t right;
    int16_t bottom;
} SceneBox_t;

typedef struct
{
    const SceneFrame_t *frame;
    uint8_t bullets[BULLET_MAX_COUNT];
    uint8_t enemies[ENEMY_MAX_COUNT];
    uint8_t bulletCount;
    uint8_t enemyCount;
    uint8_t showPlayer;
} SceneView_t;

typedef struct
{
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t color;
    uint8_t matched;
} SceneRun_t;

static SceneFrame_t previousFrame;
static SceneFrame_t currentFrame;
static SceneRun_t activeRuns[SCENE_MAX_RUNS];
static SceneRun_t nextRuns[SCENE_MAX_RUNS];

static uint8_t Scene_Intersects(SceneBox_t a, SceneBox_t b)
{
    return (uint8_t)((a.left < b.right) && (a.right > b.left) &&
                     (a.top < b.bottom) && (a.bottom > b.top));
}

static SceneBox_t Scene_Box(int16_t x, int16_t y, int16_t width, int16_t height)
{
    SceneBox_t box = {x, y, (int16_t)(x + width), (int16_t)(y + height)};
    return box;
}

static void Scene_MakeView(SceneView_t *view, const SceneFrame_t *frame, SceneBox_t area)
{
    uint8_t i;
    view->frame = frame;
    view->bulletCount = 0U;
    view->enemyCount = 0U;
    view->showPlayer = 0U;
    for (i = 0U; i < BULLET_MAX_COUNT; i++)
    {
        const SceneSprite_t *s = &frame->bullets[i];
        if ((0U != s->active) && (0U != Scene_Intersects(area, Scene_Box(s->x, s->y, BULLET_WIDTH, BULLET_HEIGHT))))
        {
            view->bullets[view->bulletCount++] = i;
        }
    }
    for (i = 0U; i < ENEMY_MAX_COUNT; i++)
    {
        const SceneSprite_t *s = &frame->enemies[i];
        if ((0U != s->active) && (0U != Scene_Intersects(area, Scene_Box(s->x, s->y, ENEMY_WIDTH, ENEMY_HEIGHT))))
        {
            view->enemies[view->enemyCount++] = i;
        }
    }
    if ((0U != frame->playerVisible) &&
        (0U != Scene_Intersects(area, Scene_Box(frame->player.x, frame->player.y, PLAYER_WIDTH, PLAYER_HEIGHT))))
    {
        view->showPlayer = 1U;
    }
}

static uint16_t Scene_ColorAt(const SceneView_t *view, int16_t x, int16_t y)
{
    uint8_t i;
    for (i = 0U; i < view->bulletCount; i++)
    {
        const SceneSprite_t *s = &view->frame->bullets[view->bullets[i]];
        if ((x >= s->x) && (x < s->x + (int16_t)BULLET_WIDTH) &&
            (y >= s->y) && (y < s->y + (int16_t)BULLET_HEIGHT))
        {
            return SCENE_BULLET;
        }
    }
    for (i = 0U; i < view->enemyCount; i++)
    {
        const SceneSprite_t *s = &view->frame->enemies[view->enemies[i]];
        int16_t dx = x - s->x;
        int16_t dy = y - s->y;
        if ((dx >= 8) && (dx < 16) && (dy >= 8) && (dy < 16))
        {
            return SCENE_ENEMY_CENTER;
        }
        if (((dx >= 4) && (dx < 20) && (dy >= 0) && (dy < 24)) ||
            ((dx >= 0) && (dx < 24) && (dy >= 8) && (dy < 16)))
        {
            return SCENE_ENEMY_BODY;
        }
    }
    if (0U != view->showPlayer)
    {
        int16_t dx = x - view->frame->player.x;
        int16_t dy = y - view->frame->player.y;
        if ((dx >= 18) && (dx < 42) && (dy >= 35) && (dy < 44))
        {
            return SCENE_PLAYER_BODY;
        }
        if ((dx >= 0) && (dx < 60) && (dy >= 17) && (dy < 29))
        {
            return SCENE_PLAYER_WING;
        }
        if ((dx >= 22) && (dx < 38) && (dy >= 0) && (dy < 38))
        {
            return SCENE_PLAYER_BODY;
        }
    }
    return SCENE_BLACK;
}

static void Scene_FlushRun(const SceneRun_t *run)
{
    Renderer_FillRect(run->x, run->y, run->width, run->height, run->color);
}

static void Scene_PaintArea(SceneBox_t area)
{
    SceneView_t oldView;
    SceneView_t newView;
    uint8_t activeCount = 0U;
    int16_t x;
    int16_t y;
    uint8_t i;

    if (area.left < 0) area.left = 0;
    if (area.top < SCENE_TOP) area.top = SCENE_TOP;
    if (area.right > SCENE_WIDTH) area.right = SCENE_WIDTH;
    if (area.bottom > SCENE_HEIGHT) area.bottom = SCENE_HEIGHT;
    if ((area.left >= area.right) || (area.top >= area.bottom)) return;

    Scene_MakeView(&oldView, &previousFrame, area);
    Scene_MakeView(&newView, &currentFrame, area);

    for (y = area.top; y < area.bottom; y++)
    {
        uint8_t nextCount = 0U;
        for (i = 0U; i < activeCount; i++) activeRuns[i].matched = 0U;
        x = area.left;
        while (x < area.right)
        {
            int16_t start;
            uint16_t oldColor = Scene_ColorAt(&oldView, x, y);
            uint16_t newColor = Scene_ColorAt(&newView, x, y);
            uint16_t runColor;
            uint8_t found = 0U;
            if (oldColor == newColor)
            {
                x++;
                continue;
            }
            start = x;
            runColor = newColor;
            x++;
            while (x < area.right)
            {
                oldColor = Scene_ColorAt(&oldView, x, y);
                newColor = Scene_ColorAt(&newView, x, y);
                if ((oldColor == newColor) || (newColor != runColor)) break;
                x++;
            }
            for (i = 0U; i < activeCount; i++)
            {
                if ((0U == activeRuns[i].matched) && (activeRuns[i].x == start) &&
                    (activeRuns[i].width == (uint16_t)(x - start)) &&
                    (activeRuns[i].color == runColor))
                {
                    if (nextCount < SCENE_MAX_RUNS)
                    {
                        nextRuns[nextCount] = activeRuns[i];
                        nextRuns[nextCount].height++;
                        nextRuns[nextCount].matched = 0U;
                        nextCount++;
                        activeRuns[i].matched = 1U;
                        found = 1U;
                    }
                    break;
                }
            }
            if (0U == found)
            {
                if (nextCount < SCENE_MAX_RUNS)
                {
                    SceneRun_t *run = &nextRuns[nextCount++];
                    run->x = start;
                    run->y = y;
                    run->width = (uint16_t)(x - start);
                    run->height = 1U;
                    run->color = runColor;
                    run->matched = 0U;
                }
                else
                {
                    SceneRun_t run = {start, y, (uint16_t)(x - start), 1U, runColor, 0U};
                    Scene_FlushRun(&run);
                }
            }
        }
        for (i = 0U; i < activeCount; i++)
        {
            if (0U == activeRuns[i].matched) Scene_FlushRun(&activeRuns[i]);
        }
        activeCount = nextCount;
        for (i = 0U; i < activeCount; i++) activeRuns[i] = nextRuns[i];
    }
    for (i = 0U; i < activeCount; i++) Scene_FlushRun(&activeRuns[i]);
}

static void Scene_PaintChange(uint8_t oldActive, int16_t oldX, int16_t oldY,
                             uint8_t newActive, int16_t newX, int16_t newY,
                             int16_t width, int16_t height)
{
    SceneBox_t area;
    if ((0U == oldActive) && (0U == newActive)) return;
    if ((0U != oldActive) && (0U != newActive) && (oldX == newX) && (oldY == newY)) return;
    /* A slot can be reused at a distant position in the next frame. */
    if ((0U != oldActive) && (0U != newActive) &&
        (((newX >= oldX + width) || (oldX >= newX + width)) ||
         ((newY >= oldY + height) || (oldY >= newY + height))))
    {
        Scene_PaintArea(Scene_Box(oldX, oldY, width, height));
        Scene_PaintArea(Scene_Box(newX, newY, width, height));
        return;
    }
    if (0U != oldActive)
    {
        area = Scene_Box(oldX, oldY, width, height);
    }
    else
    {
        area = Scene_Box(newX, newY, width, height);
    }
    if (0U != newActive)
    {
        SceneBox_t added = Scene_Box(newX, newY, width, height);
        if (added.left < area.left) area.left = added.left;
        if (added.top < area.top) area.top = added.top;
        if (added.right > area.right) area.right = added.right;
        if (added.bottom > area.bottom) area.bottom = added.bottom;
    }
    Scene_PaintArea(area);
}

void Scene_Reset(void)
{
    uint8_t i;
    previousFrame.player.x = 0;
    previousFrame.player.y = 0;
    previousFrame.playerVisible = 0U;
    for (i = 0U; i < BULLET_MAX_COUNT; i++) previousFrame.bullets[i].active = 0U;
    for (i = 0U; i < ENEMY_MAX_COUNT; i++) previousFrame.enemies[i].active = 0U;
}

void Scene_Render(const Player_t *player, uint8_t playerVisible)
{
    uint8_t i;
    if (0 == player) return;
    currentFrame.player = *player;
    currentFrame.playerVisible = playerVisible;
    for (i = 0U; i < BULLET_MAX_COUNT; i++)
    {
        SceneSprite_t *s = &currentFrame.bullets[i];
        s->active = Bullet_GetPosition(i, &s->x, &s->y);
    }
    for (i = 0U; i < ENEMY_MAX_COUNT; i++)
    {
        SceneSprite_t *s = &currentFrame.enemies[i];
        s->active = Enemy_GetPosition(i, &s->x, &s->y);
    }
    Scene_PaintChange(previousFrame.playerVisible, previousFrame.player.x, previousFrame.player.y,
                      currentFrame.playerVisible, currentFrame.player.x, currentFrame.player.y,
                      PLAYER_WIDTH, PLAYER_HEIGHT);
    for (i = 0U; i < BULLET_MAX_COUNT; i++)
    {
        const SceneSprite_t *a = &previousFrame.bullets[i];
        const SceneSprite_t *b = &currentFrame.bullets[i];
        Scene_PaintChange(a->active, a->x, a->y, b->active, b->x, b->y, BULLET_WIDTH, BULLET_HEIGHT);
    }
    for (i = 0U; i < ENEMY_MAX_COUNT; i++)
    {
        const SceneSprite_t *a = &previousFrame.enemies[i];
        const SceneSprite_t *b = &currentFrame.enemies[i];
        Scene_PaintChange(a->active, a->x, a->y, b->active, b->x, b->y, ENEMY_WIDTH, ENEMY_HEIGHT);
    }
    previousFrame = currentFrame;
}
