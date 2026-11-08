#ifndef HUD_H
#define HUD_H

#include <stdint.h>

void HUD_Reset(void);
void HUD_DrawHealth(uint8_t health);
void HUD_DrawScore(uint32_t score);
void HUD_Update(void);

#endif
