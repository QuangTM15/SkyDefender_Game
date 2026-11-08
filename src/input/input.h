#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

typedef struct
{
    float accelX;
    float accelY;
    float accelZ;
} Input_Acceleration_t;

uint8_t Input_Init(void);
uint8_t Input_Calibrate(void);
uint8_t Input_ReadAcceleration(Input_Acceleration_t *acceleration);

#endif
