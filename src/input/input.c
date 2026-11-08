#include "input.h"
#include "Arduino.h"
#include "mpu6050.h"

#define INPUT_CALIBRATION_SAMPLES 100U
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
    if ((result > -INPUT_DEAD_ZONE) && (result < INPUT_DEAD_ZONE))
    {
        result = 0.0f;
    }
    return result;
}

uint8_t Input_Init(void)
{
    MPU_Begin();
    inputReady = MPU_IsInitialized() ? 1U : 0U;
    return inputReady;
}

uint8_t Input_Calibrate(void)
{
    uint8_t success = 0U;
    MPU_Data_t data;
    float sumX = 0.0f;
    float sumY = 0.0f;
    float sumZ = 0.0f;

    if (0U != inputReady)
    {
        for (uint16_t i = 0U; i < INPUT_CALIBRATION_SAMPLES; i++)
        {
            MPU_ReadData(&data);
            sumX += data.accelX;
            sumY += data.accelY;
            sumZ += data.accelZ;
            delay(10U);
        }

        offsetX = sumX / (float)INPUT_CALIBRATION_SAMPLES;
        offsetY = sumY / (float)INPUT_CALIBRATION_SAMPLES;
        offsetZ = sumZ / (float)INPUT_CALIBRATION_SAMPLES;
        filtered = (Input_Acceleration_t){0};
        success = 1U;
    }

    return success;
}

uint8_t Input_ReadAcceleration(Input_Acceleration_t *acceleration)
{
    uint8_t success = 0U;
    MPU_Data_t data;

    if ((0U != inputReady) && (0 != acceleration))
    {
        MPU_ReadData(&data);
        filtered.accelX = Input_Filter(data.accelX - offsetX, filtered.accelX);
        filtered.accelY = Input_Filter(data.accelY - offsetY, filtered.accelY);
        filtered.accelZ = Input_Filter(data.accelZ - offsetZ, filtered.accelZ);
        *acceleration = filtered;
        success = 1U;
    }

    return success;
}
