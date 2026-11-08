#include "input.h"
#include "Arduino.h"
#include "mpu6050.h"

#define INPUT_CALIBRATION_SAMPLES 100U
#define INPUT_CALIBRATION_MAX_ATTEMPTS 120U
#define INPUT_DEAD_ZONE 0.03f
#define INPUT_FILTER_ALPHA 0.35f

static uint8_t inputReady = 0U;
static float offsetX = 0.0f;
static float offsetY = 0.0f;
static float offsetZ = 0.0f;
static Input_Acceleration_t filtered = {0};

static float Input_Filter(float value, float previous)
{
    float result = INPUT_FILTER_ALPHA * value + (1.0f - INPUT_FILTER_ALPHA) * previous;
    if ((result > -INPUT_DEAD_ZONE) && (result < INPUT_DEAD_ZONE)) result = 0.0f;
    return result;
}

uint8_t Input_Init(void)
{
    inputReady = (0U != MPU_Begin()) ? 1U : 0U;
    return inputReady;
}

uint8_t Input_Calibrate(void)
{
    uint8_t success = 0U;
    MPU_Data_t data;
    float sumX = 0.0f;
    float sumY = 0.0f;
    float sumZ = 0.0f;
    uint16_t valid = 0U;
    uint16_t attempt;

    if (0U != inputReady)
    {
        for (attempt = 0U; attempt < INPUT_CALIBRATION_MAX_ATTEMPTS; attempt++)
        {
            if (0U != MPU_ReadData(&data))
            {
                sumX += data.accelX;
                sumY += data.accelY;
                sumZ += data.accelZ;
                valid++;
                if (valid >= INPUT_CALIBRATION_SAMPLES) break;
            }
            delay(10U);
        }
        if (valid == INPUT_CALIBRATION_SAMPLES)
        {
            offsetX = sumX / (float)valid;
            offsetY = sumY / (float)valid;
            offsetZ = sumZ / (float)valid;
            filtered = (Input_Acceleration_t){0};
            success = 1U;
        }
    }
    return success;
}

uint8_t Input_ReadAcceleration(Input_Acceleration_t *acceleration)
{
    uint8_t success = 0U;
    MPU_Data_t data;
    if ((0U != inputReady) && (0 != acceleration) && (0U != MPU_ReadData(&data)))
    {
        filtered.accelX = Input_Filter(data.accelX - offsetX, filtered.accelX);
        filtered.accelY = Input_Filter(data.accelY - offsetY, filtered.accelY);
        filtered.accelZ = Input_Filter(data.accelZ - offsetZ, filtered.accelZ);
        *acceleration = filtered;
        success = 1U;
    }
    return success;
}
