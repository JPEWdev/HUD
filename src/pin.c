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
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <util/atomic.h>

#include "pin.h"

struct pin_ctrl {
    volatile uint8_t PIN;
    volatile uint8_t DDR;
    volatile uint8_t PORT;
};

#define PIN_CTRL ((volatile struct pin_ctrl*)&PINA)

static struct pin_change_handler *pcint_list[PIN_CNT / 8];

static volatile uint8_t* pin_mask_reg[] =
    {
    &PCMSK0,
    &PCMSK1,
    &PCMSK2,
    &PCMSK3,
    };

void
pin_set_direction(enum pin pin, enum pin_direction direction)
{
    if (pin < PIN_INVALID) {
        if (direction == PIN_OUTPUT)
            PIN_CTRL[pin / 8].DDR |= _BV(pin % 8);
        else
            PIN_CTRL[pin / 8].DDR &= ~_BV(pin % 8);
    }
}

void
pin_set_output(enum pin pin, enum pin_output output)
{
    pin_set_pullup(pin, (bool)output);
}

void
pin_toggle_output(enum pin pin)
{
    if (pin < PIN_INVALID)
        PIN_CTRL[pin / 8].PORT ^= _BV(pin % 8);
}

void
pin_set_pullup(enum pin pin, bool enabled)
{
    if (pin < PIN_INVALID) {
        if (enabled)
            PIN_CTRL[pin / 8].PORT |= _BV(pin % 8);
        else
            PIN_CTRL[pin / 8].PORT &= ~_BV(pin % 8);
    }
}

bool
pin_read(enum pin pin)
{
    if (pin < PIN_INVALID && (PIN_CTRL[pin / 8].PIN & _BV(pin %8)))
        return true;
    return false;
}

static void
set_pcint_mask(uint8_t pcint)
{
    uint8_t mask = 0;
    struct pin_change_handler *cur = pcint_list[pcint];

    while (cur != NULL) {
        mask |= _BV(cur->pin % 8);
        cur = cur->next;
    }

    *pin_mask_reg[pcint] = mask;

    if (mask) {
        PCICR |= _BV(pcint);
    } else {
        PCICR &= ~_BV(pcint);
        PCIFR &= ~_BV(pcint);
    }
}

void
pin_enable_interrupt(struct pin_change_handler *handler)
{
    uint8_t pcint = handler->pin / 8;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        handler->next = pcint_list[pcint];
        handler->prev = NULL;

        if (pcint_list[pcint])
            pcint_list[pcint]->prev = handler;

        pcint_list[pcint] = handler;

        set_pcint_mask(pcint);
    }
}

void
pin_disable_interrupt(struct pin_change_handler *handler)
{
    uint8_t pcint = handler->pin / 8;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if (handler->next)
            handler->next->prev = handler->prev;

        if (handler->prev)
            handler->prev->next = handler->next;

        if (pcint_list[pcint] == handler)
            pcint_list[pcint] = handler->next;

        handler->next = NULL;
        handler->prev = NULL;

        set_pcint_mask(pcint);
    }
}

static void
handle_pc_isr(uint8_t pcint)
{
    struct pin_change_handler *cur, *next;

    cur = pcint_list[pcint];

    while (cur) {
        next = cur->next;

        if (cur->callback)
            cur->callback(cur->param);

        cur = next;
    }
}

ISR(PCINT0_vect)
{
    handle_pc_isr(0);
}

ISR(PCINT1_vect)
{
    handle_pc_isr(1);
}

ISR(PCINT2_vect)
{
    handle_pc_isr(2);
}

ISR(PCINT3_vect)
{
    handle_pc_isr(3);
}

