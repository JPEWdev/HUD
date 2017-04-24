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
#ifndef _LAYOUT_H_
#define _LAYOUT_H_

#include <stdint.h>

#include "vfd.h"

#define DATA_CNT (4)

struct data_wndw_type {
    VFD_win_t8 window;
    uint16_t x;
    uint16_t y;
    uint16_t wdth;
    uint16_t hght;
    uint8_t font_sz;
};

struct screen_layout_type {
    char const *name;
    uint8_t wndw_cnt;
    struct data_wndw_type windows[DATA_CNT];
};

const struct screen_layout_type layout_2_LR;
const struct screen_layout_type layout_2_TB;
const struct screen_layout_type layout_3L;
const struct screen_layout_type layout_3R;
const struct screen_layout_type layout_4;

void
create_layout_windows(struct screen_layout_type const *layout);

#endif /* _LAYOUT_H_ */


