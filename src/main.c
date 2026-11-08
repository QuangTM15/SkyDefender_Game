#include "Arduino.h"
#include "core/game.h"

int main(void)
{
    setup();
    Game_Init();

    while (1)
    {
        Game_Update();
    }

    return 0;
}
