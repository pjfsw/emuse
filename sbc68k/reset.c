// reset.c - ATtiny2313 reset controller for MC68000
// Button: PD6, /RESET: PD2, /HALT: PD0
//
// Lines are externally pulled up.
// AVR drives LOW for assert, switches to input/high-Z for release.

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>

#define RESET_BTN   PD6
#define RESET_OUT   PD2
#define HALT_OUT    PD0

#define RESET_TIME_MS   1000
#define DEBOUNCE_MS       25

static void assert_68k_reset(void)
{
    // Drive /RESET and /HALT low
    PORTD &= ~((1 << RESET_OUT) | (1 << HALT_OUT));
    DDRD  |=  ((1 << RESET_OUT) | (1 << HALT_OUT));
}

static void release_68k_reset(void)
{
    // High-Z release; external pullups raise the lines
    DDRD  &= ~((1 << RESET_OUT) | (1 << HALT_OUT));
    PORTD &= ~((1 << RESET_OUT) | (1 << HALT_OUT)); // no internal pullups
}

static bool button_pressed(void)
{
    // Assumes button shorts PD6 to GND when pressed.
    return !(PIND & (1 << RESET_BTN));
}

static void do_reset_pulse(void)
{
    assert_68k_reset();
    _delay_ms(RESET_TIME_MS);
    release_68k_reset();
}

int main(void)
{
    // Reset outputs start high-Z immediately
    release_68k_reset();

    // Button input with internal pullup enabled
    DDRD  &= ~(1 << RESET_BTN);
    PORTD |=  (1 << RESET_BTN);

    // Hold 68000 reset/halt during AVR startup
    do_reset_pulse();

    while (1) {
        if (button_pressed()) {
            _delay_ms(DEBOUNCE_MS);

            if (button_pressed()) {
                do_reset_pulse();

                // Wait until button is released so it doesn't retrigger
                while (button_pressed()) {
                    _delay_ms(10);
                }

                _delay_ms(DEBOUNCE_MS);
            }
        }
    }
}
