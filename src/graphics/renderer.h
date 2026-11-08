#ifndef RENDERER_H
#define RENDERER_H
#include <stdint.h>

void Renderer_Init(void);
void Renderer_Clear(void);
void Renderer_FillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color);

#endif