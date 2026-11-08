#ifndef SCENE_H
#define SCENE_H

#include <stdint.h>
#include "entities/player.h"

/* Composite the player, bullets and enemies without a full framebuffer. */
void Scene_Reset(void);
void Scene_Render(const Player_t *player, uint8_t playerVisible);

#endif
