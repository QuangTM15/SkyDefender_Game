#include "difficulty.h"

void Difficulty_GetConfig(uint32_t elapsedMs, Difficulty_Config_t *config)
{
    static const Difficulty_Config_t levels[5] = {
        {1500U, 35U, 3U},
        {1300U, 43U, 3U},
        {1050U, 51U, 4U},
        {850U, 60U, 4U},
        {700U, 70U, 5U}
    };
    uint32_t level = elapsedMs / 50000U;
    if (level > 4U) level = 4U;
    if (0 != config) *config = levels[level];
}
