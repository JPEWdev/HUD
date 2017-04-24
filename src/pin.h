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
#ifndef PIN_H_
#define PIN_H_

#include <stdbool.h>

enum pin_direction {
    PIN_INPUT,
    PIN_OUTPUT
};

enum pin_output {
    PIN_LOW,
    PIN_HIGH
};

enum pin {
    PIN_A0,
    PIN_A1,
    PIN_A2,
    PIN_A3,
    PIN_A4,
    PIN_A5,
    PIN_A6,
    PIN_A7,

    PIN_B0,
    PIN_B1,
    PIN_B2,
    PIN_B3,
    PIN_B4,
    PIN_B5,
    PIN_B6,
    PIN_B7,

    PIN_C0,
    PIN_C1,
    PIN_C2,
    PIN_C3,
    PIN_C4,
    PIN_C5,
    PIN_C6,
    PIN_C7,

    PIN_D0,
    PIN_D1,
    PIN_D2,
    PIN_D3,
    PIN_D4,
    PIN_D5,
    PIN_D6,
    PIN_D7,

    PIN_CNT,
    PIN_INVALID = PIN_CNT
};

struct pin_change_handler {
    struct pin_change_handler *next;
    struct pin_change_handler *prev;

    enum pin pin;
    void (*callback)(void *param);
    void* param;
};

void
pin_set_direction(enum pin pin, enum pin_direction direction);

void
pin_set_output(enum pin pin, enum pin_output state);

void
pin_toggle_output(enum pin pin);

void
pin_set_pullup(enum pin pin, bool enabled);

bool
pin_read(enum pin pin);

void
pin_enable_interrupt(struct pin_change_handler* handler);

void
pin_disable_interrupt(struct pin_change_handler* handler);


#endif /* PIN_H_ */
