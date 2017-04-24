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
#ifndef _LED_H_
#define _LED_H_

#include <stdint.h>
#include <stdbool.h>

enum LED {
    LED_LEFT,
    LED_RIGHT,

    LED_COUNT,

};

#define LED_STATUS LED_RIGHT
#define LED_POWER LED_LEFT

void
LED_init(void);

void
LED_set(enum LED l, bool val);

void
LED_on(enum LED l);

void
LED_off(enum LED l);

void
LED_toggle(enum LED l);

void
LED_blink(enum LED l, uint16_t time);

void
LED_strobe(enum LED l, uint16_t rate);

void
LED_blink_wait(enum LED l, uint8_t count);

#endif /* _LED_H_ */
