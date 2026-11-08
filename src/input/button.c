#include "button.h"

typedef struct
{
    uint8_t pin;
    uint8_t stableState;
    uint8_t lastReading;
    uint8_t pressed;
    uint32_t lastChange;
} Button_t;

static Button_t selectButton;
static Button_t nextButton;

static void Button_UpdateOne(Button_t *button)
{
    uint8_t reading = digitalRead(button->pin);
    uint32_t now = millis();

    if (reading != button->lastReading)
    {
        button->lastChange = now;
        button->lastReading = reading;
    }

    if ((now - button->lastChange) >= BUTTON_DEBOUNCE_MS)
    {
        if (reading != button->stableState)
        {
            button->stableState = reading;

            if (LOW == reading)
            {
                button->pressed = 1U;
            }
        }
    }
}

void Button_Init(void)
{
    pinMode(BUTTON_SELECT, INPUT_PULLUP);
    pinMode(BUTTON_NEXT, INPUT_PULLUP);

    selectButton.pin = BUTTON_SELECT;
    selectButton.stableState = digitalRead(BUTTON_SELECT);
    selectButton.lastReading = selectButton.stableState;
    selectButton.pressed = 0U;
    selectButton.lastChange = millis();

    nextButton.pin = BUTTON_NEXT;
    nextButton.stableState = digitalRead(BUTTON_NEXT);
    nextButton.lastReading = nextButton.stableState;
    nextButton.pressed = 0U;
    nextButton.lastChange = millis();
}

void Button_Update(void)
{
    Button_UpdateOne(&selectButton);
    Button_UpdateOne(&nextButton);
}

uint8_t Button_SelectPressed(void)
{
    uint8_t result = selectButton.pressed;
    selectButton.pressed = 0U;
    return result;
}

uint8_t Button_NextPressed(void)
{
    uint8_t result = nextButton.pressed;
    nextButton.pressed = 0U;
    return result;
}
