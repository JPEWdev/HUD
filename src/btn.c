/*
 * Copyright 2017 Joshua Watt
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include <avr/io.h>
#include <string.h>

#include "btn.h"
#include "pin.h"
#include "timer.h"

#define DEBOUNCE_TIME (100)
#define HOLD_TIME (3000)

#define LEFT_BTN PIN_C5
#define RIGHT_BTN PIN_C4

#define LEFT_EXT PIN_B4
#define RIGHT_EXT PIN_B3

static struct pin_change_handler pch[4];

void
BTN_init(void)
{
    pin_set_direction(LEFT_BTN, PIN_INPUT);
    pin_set_direction(RIGHT_BTN, PIN_INPUT);
    pin_set_direction(LEFT_EXT, PIN_INPUT);
    pin_set_direction(RIGHT_EXT, PIN_INPUT);

    pin_set_pullup(LEFT_BTN, true);
    pin_set_pullup(RIGHT_BTN, true);
    pin_set_pullup(LEFT_EXT, true);
    pin_set_pullup(RIGHT_EXT, true);

    /*
     * Enable interrupts so that buttons can wake the device from sleep mode
     */
    pch[0].pin = LEFT_BTN;
    pch[1].pin = RIGHT_BTN;
    pch[2].pin = LEFT_EXT;
    pch[3].pin = RIGHT_EXT;

    pin_enable_interrupt(&pch[0]);
    pin_enable_interrupt(&pch[1]);
    pin_enable_interrupt(&pch[2]);
    pin_enable_interrupt(&pch[3]);
}

static bool left_enabled = true;
static bool right_enabled = true;

static bool
check_left(void)
{
    return !pin_read(LEFT_BTN) || !pin_read(LEFT_EXT);
}

static bool
check_right(void)
{
    return !pin_read(RIGHT_BTN) || !pin_read(RIGHT_EXT);
}

uint8_t
BTN_check(void)
{
    uint8_t btns = 0;

    if (check_left())
        btns |= BTN_L_PRESS;

    if (check_right())
        btns |= BTN_R_PRESS;

    if (btns) {
        sleep(DEBOUNCE_TIME);
        btns = 0;

        if (check_left())
            btns |= BTN_L_PRESS;

        if (check_right())
            btns |= BTN_R_PRESS;
    }

    return btns;
}

uint8_t
BTN_process(void)
{
    uint8_t btns = 0;
    uint32_t time;

    /* Re-Enable buttons if they are not pressed */
    if (!check_left())
        left_enabled = true;

    if (!check_right())
        right_enabled = true;

    /* Debounce */
    if (check_left() || check_right()) {
        timer_sleep(DEBOUNCE_TIME);

        /* We have presses for sure */
        if (check_left() && left_enabled)
            btns |= BTN_L_PRESS;

        if (check_right() && right_enabled)
            btns |= BTN_R_PRESS;

        /* Check for holds */
        time = timer_get();
        while ( (check_left() && left_enabled) || (check_right() && right_enabled)) {
            if (timer_get() - time >= HOLD_TIME) {
                /* Upgrade presses to holds */
                if (check_left()) {
                    btns &= ~BTN_L_PRESS;
                    btns |= BTN_L_HOLD;

                    /* Mask off this button until it is released */
                    left_enabled = false;
                }

                if (check_right()) {
                    btns &= ~BTN_R_PRESS;
                    btns |= BTN_R_HOLD;

                    /* Mask off this button until it is released */
                    right_enabled = false;
                }
                break;
            }
            timer_process();
        }

    }

    return btns;
}

static void
btn_wait_clbk(void *param)
{
    *(bool*)param = true;
}

uint8_t
BTN_wait(uint32_t timeout)
{
    bool done = false;
    uint8_t btns = 0;
    struct timer timer;

    memset(&timer, 0, sizeof(timer));

    if (timeout)
        timer_create(&timer, timeout, btn_wait_clbk, &done);

    while (!btns && !done) {
        btns = BTN_process();
        timer_process();
    }

    timer_cncl(&timer);

    return btns;
}

