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
#include "layout.h"
#include "utl.h"
#include "vfd.h"

const struct screen_layout_type layout_2_LR = {
    "Layout 2",
    2,
    {
        { VFD_WIN_1,    0,              0,  VFD_WDTH / 2,   VFD_LINE_HGHT,  2   },
        { VFD_WIN_2,    VFD_WDTH / 2,   0,  VFD_WDTH / 2,   VFD_LINE_HGHT,  2   },
    }
};

const struct screen_layout_type layout_2_TB = {
    "Layout 2",
    2,
    {
        { VFD_WIN_1,    0,      0,                  VFD_WDTH,   VFD_LINE_HGHT / 2 },
        { VFD_WIN_2,    0,      VFD_LINE_HGHT / 2,  VFD_WDTH,   VFD_LINE_HGHT / 2 },
    }
};

const struct screen_layout_type layout_3L = {
    "Layout 3L",
    3,
    {
        {   VFD_WIN_1,  0,                      0,                  ( VFD_WDTH * 3 ) / 5,   VFD_LINE_HGHT,      2   },
        {   VFD_WIN_2,  ( VFD_WDTH * 3 ) / 5,   0,                  ( VFD_WDTH * 2 ) / 5,   VFD_LINE_HGHT / 2,  1   },
        {   VFD_WIN_3,  ( VFD_WDTH * 3 ) / 5,   VFD_LINE_HGHT / 2,  ( VFD_WDTH * 2 ) / 5,   VFD_LINE_HGHT / 2,  1   },
    }
};

const struct screen_layout_type layout_3R = {
    "Layout 3R",
    3,
    {
        {   VFD_WIN_1,  0,                      0,                  ( VFD_WDTH * 2 ) / 5,   VFD_LINE_HGHT / 2,  1   },
        {   VFD_WIN_2,  0,                      VFD_LINE_HGHT / 2,  ( VFD_WDTH * 2 ) / 5,   VFD_LINE_HGHT / 2,  1   },
        {   VFD_WIN_3,  ( VFD_WDTH * 2 ) / 5,   0,                  ( VFD_WDTH * 3 ) / 5,   VFD_LINE_HGHT,      2   },
    }
};

const struct screen_layout_type layout_4 = {
    "Layout 4",
    4,
    {
        {   VFD_WIN_1,  0,              0,                  VFD_WDTH / 2,   VFD_LINE_HGHT / 2,  1   },
        {   VFD_WIN_2,  VFD_WDTH / 2,   0,                  VFD_WDTH / 2,   VFD_LINE_HGHT / 2,  1   },
        {   VFD_WIN_3,  0,              VFD_LINE_HGHT / 2,  VFD_WDTH / 2,   VFD_LINE_HGHT / 2,  1   },
        {   VFD_WIN_4,  VFD_WDTH / 2,   VFD_LINE_HGHT / 2,  VFD_WDTH / 2,   VFD_LINE_HGHT / 2,  1   },
    }
};

void
create_layout_windows(struct screen_layout_type const *layout)
{
    uint8_t i;

    for (i = 0; i < layout->wndw_cnt; i++) {
        VFD_create_window(layout->windows[i].window,
                layout->windows[i].x, layout->windows[i].y,
                layout->windows[i].wdth, layout->windows[i].hght );
    }
}

