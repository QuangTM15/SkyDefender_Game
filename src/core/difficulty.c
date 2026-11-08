#include "difficulty.h"

void Difficulty_GetConfig(uint32_t elapsedMs, Difficulty_Config_t *config)
{
    static const Difficulty_Config_t levels[5] =
        {
            {1500U, 35U, 3U},
            {1200U, 45U, 4U},
            {900U, 55U, 5U},
            {700U, 65U, 6U},
            {500U, 80U, 8U}};
    uint32_t level = elapsedMs / 50000U;

    if (level > 4U)
    {
        level = 4U;
    }

    if (config != 0)
    {
        *config = levels[level];
    }
}
