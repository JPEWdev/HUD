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

#include "led.h"
#include "pin.h"
#include "timer.h"
#include "utl.h"

struct led {
    enum pin pin;
    struct timer timer;
    uint32_t strobe_rate;
};

static struct led leds[LED_COUNT];

static struct led*
get(enum LED l)
{
    return &leds[l];
}

static void
off(struct led const *led)
{
    pin_set_output(led->pin, PIN_HIGH);
}

static void
on(struct led const *led)
{
    pin_set_output(led->pin, PIN_LOW);
}

static void
toggle(struct led const *led)
{
    pin_toggle_output(led->pin);
}

static void
blink_led_timer(void *param)
{
    struct led *led = param;
    off(led);
}

static void
strobe_led_timer(void *param)
{
    struct led *led = param;
    toggle(led);
    timer_create(&led->timer, led->strobe_rate, strobe_led_timer, led);
}

void
LED_init(void)
{
    static const enum pin led_pins[] = {
        PIN_D6,
        PIN_D5,
    };
    STATIC_ASSERT(cnt_of_array(led_pins) == LED_COUNT);

    enum LED i;

    for (i = 0; i < LED_COUNT; i++) {
        leds[i].pin = led_pins[i];
        pin_set_direction(led_pins[i], PIN_OUTPUT);
        off(&leds[i]);
   }
}

void
LED_set(enum LED l, bool val)
{
    struct led *led = get(l);
    timer_cncl(&led->timer);
    if (val)
        on(led);
    else
        off(led);
}

void
LED_on(enum LED l)
{
    struct led *led = get(l);
    timer_cncl(&led->timer);
    on(led);
}

void
LED_off(enum LED l)
{
    struct led *led = get(l);
    timer_cncl(&led->timer);
    off(led);
}

void
LED_toggle(enum LED l)
{
    struct led *led = get(l);
    timer_cncl(&led->timer);
    toggle(led);
}

void
LED_blink(enum LED l, uint16_t duration)
{
    struct led *led = get(l);
    on(led);
    timer_create(&led->timer, duration, blink_led_timer, led);
}

void
LED_strobe(enum LED l, uint16_t rate)
{
    struct led *led = get(l);
    led->strobe_rate = rate;
    timer_create(&led->timer, rate, strobe_led_timer, led);
}

void
LED_blink_wait(enum LED l, uint8_t count)
{
    uint8_t i;
    struct led *led = get(l);

    timer_cncl(&led->timer);

    for (i = 0; i < count; i++) {
        on(led);
        sleep(1000);
        off(led);
        sleep(1000);
    }
    sleep(1000);
}

