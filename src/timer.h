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
#ifndef _TIMER_H_
#define _TIMER_H_

typedef void (*timer_clbk)(void *);

struct timer {
    uint32_t exp_time;
    timer_clbk clbk;
    void *param;
    struct timer *next;
    struct timer *prev;
};

void
timer_init(void);

uint32_t
timer_get(void);

void
timer_create(struct timer *timer, uint32_t intvl, timer_clbk clbk,
             void * param);

void
timer_cncl(struct timer *timer);

void
timer_process(void);

void
timer_sleep(uint16_t millisec);

void
sleep(uint16_t millisec);

#endif /* _TIMER_H_ */
