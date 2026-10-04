#ifndef BUTTONS_H
#define BUTTONS_H
#include <stdint.h>
enum { BUTTON_NONE, BUTTON_PRESS };
typedef struct {
    uint8_t raw_pressed, stable_pressed;
    uint32_t last_change_time;
} Button;
static inline void ButtonInit(Button *button, uint8_t pressed, uint32_t now)
{
    button->raw_pressed = button->stable_pressed = pressed;
    button->last_change_time = now;
}
/* Provided: stable level after 20 ms, and one PRESS event for each new press. */
static inline uint8_t ButtonUpdate(Button *button, uint8_t pressed, uint32_t now)
{
    if (pressed != button->raw_pressed) {
        button->raw_pressed = pressed;
        button->last_change_time = now;
    }
    if (button->stable_pressed != pressed && now - button->last_change_time >= 20U) {
        button->stable_pressed = pressed;
        if (pressed) return BUTTON_PRESS;
    }
    return BUTTON_NONE;
}
#endif
