#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include "Arduino.h"

#define BUTTON_SELECT BTN0
#define BUTTON_NEXT BTN1

#define BUTTON_DEBOUNCE_MS 30U

void Button_Init(void);
void Button_Update(void);
uint8_t Button_SelectPressed(void);
uint8_t Button_NextPressed(void);

#endif
