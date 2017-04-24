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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <util/atomic.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "timer.h"

static uint32_t cur_time = 0;
static struct timer *timer_head = NULL;

#define CNT_PER_MS (((F_CPU / 1024) / 1000) - 1)

void
timer_init(void)
{
    TCCR0A = _BV(WGM01);
    TCCR0B |= _BV(CS02) | _BV(CS00);
    TIMSK0 |= _BV(OCIE0A);
    OCR0A = CNT_PER_MS;
    TCNT0 = 0;
}

uint32_t
timer_get(void)
{
    uint32_t ret;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ret = cur_time;
    }

    return ret;
}

void
timer_create(struct timer *timer, uint32_t intvl, timer_clbk clbk, void *param)
{
    struct timer *cur;

    /* First, cancel the timer */
    timer_cncl(timer);

    timer->clbk = clbk;
    timer->param = param;
    timer->exp_time = timer_get() + intvl;
    timer->next = NULL;
    timer->prev = NULL;

    /* Check if the timer needs to be at the head of the list */
    if (timer_head == NULL) {
        timer_head = timer;
    } else if (timer_head->exp_time >= timer->exp_time) {
        /* Insert as the new head */
        timer->next = timer_head;
        timer_head->prev = timer;

        timer_head = timer;
    } else {
        /* Find the node to insert after */
        cur = timer_head;

        while (cur->next && cur->next->exp_time < timer->exp_time)
            cur = cur->next;

        timer->next = cur->next;
        timer->prev = cur;

        if (cur->next)
            cur->next->prev = timer;

        cur->next = timer;
    }
}

void
timer_cncl(struct timer *timer)
{
    if (timer_head == timer)
        timer_head = timer->next;

    if (timer->next)
        timer->next->prev = timer->prev;

    if (timer->prev)
        timer->prev->next = timer->next;

    timer->next = NULL;
    timer->prev = NULL;
}

void
timer_process(void)
{
    struct timer *temp;

    while (timer_head && timer_head->exp_time <= timer_get()) {
        temp = timer_head;
        timer_head = timer_head->next;

        if( timer_head != NULL )
            timer_head->prev = NULL;

        temp->next = NULL;
        temp->prev = NULL;

        temp->clbk(temp->param);
    }
}

static void
timer_sleep_clbk(void *param)
{
    *(bool*)param = true;
}

void
timer_sleep(uint16_t millisec)
{
    struct timer t;
    bool done = false;

    memset(&t, 0, sizeof(t));

    timer_create(&t, millisec, timer_sleep_clbk, &done);

    while (!done) {
        _delay_ms(1);
        timer_process();
    }
}


ISR(TIMER0_COMPA_vect)
{
    cur_time++;
}

void
sleep(uint16_t millisec)
{
    while (millisec) {
        _delay_ms(1);/* 1 ms delay */
        millisec--;
    }
}


