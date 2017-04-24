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
#ifndef _VFD_H_
#define _VFD_H_

#include <stdint.h>
#include <stdbool.h>

#define VFD_WDTH        (140)
#define VFD_HGHT        (16)
#define VFD_LINE_HGHT   (2)
#define VFD_CHAR_WDTH   (7)

typedef uint8_t VFD_mix_mode_t8; enum {
    VFD_MIX_NORMAL = 0,
    VFD_MIX_OR = 1,
    VFD_MIX_AND = 2,
    VFD_MIX_XOR = 3,

    VFD_MIX_DEFAULT = VFD_MIX_NORMAL
};

typedef uint8_t VFD_font_t8;

typedef uint8_t VFD_char_code_t8;

typedef uint8_t VFD_bright_lvl_t8; enum {
    VFD_BRIGHT_12_5 = 1,
    VFD_BRIGHT_25   = 2,
    VFD_BRIGHT_37_5 = 3,
    VFD_BRIGHT_50   = 4,
    VFD_BRIGHT_62_5 = 5,
    VFD_BRIGHT_75   = 6,
    VFD_BRIGHT_87_5 = 7,
    VFD_BRIGHT_100  = 8,

    VFD_BRIGHT_DEFAULT = VFD_BRIGHT_100
};

typedef uint8_t VFD_pattern_t8; enum {
    VFD_PATTERN_NORMAL  = 0,
    VFD_PATTERN_BLANK   = 1,
    VFD_PATTERN_REVERSE = 2
};

typedef uint8_t VFD_screen_saver_t8; enum {
    VFD_SCREEN_SAVER_POWER_OFF = 0,
    VFD_SCREEN_SAVER_POWER_ON  = 1,
    VFD_SCREEN_SAVER_ALL_OFF   = 2,
    VFD_SCREEN_SAVER_ALL_ON    = 3,
    VFD_SCREEN_SAVER_BLINK     = 4
};

typedef uint8_t VFD_char_width_t8; enum {
    VFD_CHAR_WDTH_FIXED_1 = 0,
    VFD_CHAR_WDTH_FIXED_2 = 1,
    VFD_CHAR_WDTH_PROP_1  = 2,
    VFD_CHAR_WDTH_PROP_2  = 3,

    VFD_CHAR_WDTH_DEFAULT = VFD_CHAR_WDTH_FIXED_2
};

typedef uint8_t VFD_win_t8; enum {
    VFD_WIN_BASE = 0,
    VFD_WIN_1    = 1,
    VFD_WIN_2    = 2,
    VFD_WIN_3    = 3,
    VFD_WIN_4    = 4,

    VFD_WIN_CNT
};

typedef uint8_t VFD_screen_mode_t8; enum {
    VFD_MODE_VISIBLE_AREA = 0,
    VFD_MODE_FULL_AREA    = 1,

    VFD_MODE_DEFAULT      = VFD_MODE_VISIBLE_AREA
};

bool
VFD_ready(void);

void
VFD_connect(void);

void
VFD_power_down(void);

void
VFD_printf(char const * fmt, ...);

void
VFD_write_string(char const * str);

void
VFD_set_cursor(uint16_t x, uint16_t y);

void
VFD_clear(void);

void
VFD_cursor_enable(bool on);

void
VFD_init(void);

void
VFD_soft_reset(void);

void
VFD_sel_font(VFD_font_t8 font);

void
VFD_sel_char_code(VFD_char_code_t8 code);

void
VFD_overwite(void);

void
VFD_vert_scroll(void);

void
VFD_hort_scroll(void);

void
VFD_hort_scroll_speed(uint8_t speed);

void
VFD_reverse(uint8_t rev);

void
VFD_mix_mode(VFD_mix_mode_t8 mode);

void
VFD_brightness_set(VFD_bright_lvl_t8 lvl);

VFD_bright_lvl_t8
VFD_brightness_get(void);

void
VFD_wait(uint8_t time);

void
VFD_scroll(uint16_t width, uint16_t rep, uint8_t speed);

void
VFD_blink(VFD_pattern_t8 pattern, uint8_t on_time, uint8_t off_time,
        uint8_t reps);

void
VFD_screen_saver(VFD_screen_saver_t8 ss);

void
VFD_char_width(VFD_char_width_t8 wdth);

void
VFD_font_size(uint8_t x, uint8_t y);

void
VFD_win_select(VFD_win_t8 win);

bool
VFD_win_enabled(VFD_win_t8 win);

void
VFD_create_window(VFD_win_t8 win, uint16_t left, uint16_t top,
        uint16_t wdth, uint16_t hght);

void
VFD_delete_window(VFD_win_t8 win);

void
VFD_screen_mode(VFD_screen_mode_t8 mode);

#endif /* _VFD_H_ */
