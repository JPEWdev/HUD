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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "led.h"
#include "pin.h"
#include "timer.h"
#include "uart.h"
#include "utl.h"
#include "vfd.h"

#define US      0x1F

#define BUSY_PIN PIN_A4
#define POWER_CTRL_PIN PIN_A7
#define RESET_PIN PIN_A3

#define UART    (UART1)
#define BAUD    (500000)

#define write_data(_data, _sz) UART_tx(UART, _data, _sz)

static VFD_win_t8 cur_win;

static bool win_enabled[VFD_WIN_CNT];

static VFD_bright_lvl_t8 cur_brightness = VFD_BRIGHT_DEFAULT;

#define write_buf(_buf) write_data(_buf, cnt_of_array(_buf))

#define DISP_FUNC(_f) US, '(', 'a', (_f)
#define FONT_FUNC(_f) US, '(', 'g', (_f)
#define WIN_FUNC(_f)  US, '(', 'w', (_f)

#define PACK_16(_p) (uint8_t)((_p) & 0x00FF), (uint8_t)(((_p) & 0xFF00) >> 8)


static const uint8_t win_sel_tbl[/* VFD_WIN_CNT */] = {
    /* VFD_WIN_BASE */  0x10,
    /* VFD_WIN_1    */  0x11,
    /* VFD_WIN_2    */  0x12,
    /* VFD_WIN_3    */  0x13, /* VFD documentation incorrectly lists this as 0x03 */
    /* VFD_WIN_4    */  0x14, /* VFD documentation incorrectly lists this as 0x04 */
};

STATIC_ASSERT(cnt_of_array(win_sel_tbl) == VFD_WIN_CNT);

bool
VFD_ready(void)
{
    #ifndef __AVR
    return true;
    #else
    return UART_tx_empty(UART);
    #endif
}

void
VFD_connect(void)
{
    #ifndef __AVR
    {
        struct termios options;

        fd = open("/dev/ttyS1", O_WRONLY | O_NOCTTY | O_NDELAY );
        if (fd < 0) {
            perror("open_port: Unable to open /dev/ttyS1 - ");
            return;
        }
        tcgetattr(fd, &options);

        cfmakeraw(&options);
        cfsetspeed(&options, B38400);
        options.c_cflag |= CLOCAL | CREAD;

        tcsetattr(fd, TCSANOW, &options);
    }
    #else
    {
        UART_init(UART, UART_TX | UART_RX | UART_SPI_MODE_3 | UART_SPI_LSB, BAUD);
        pin_set_direction(BUSY_PIN, PIN_INPUT);
        UART_set_cts_in(UART, BUSY_PIN, true);

        pin_set_direction(POWER_CTRL_PIN, PIN_OUTPUT);
        pin_set_direction(RESET_PIN, PIN_OUTPUT);

        pin_set_output(POWER_CTRL_PIN, PIN_LOW);
        pin_set_output(RESET_PIN, PIN_HIGH);
        timer_sleep(1000);
    }
    #endif
}

void
VFD_power_down(void)
{
#ifdef __AVR
    UART_tx_wait(UART);
    pin_set_output(POWER_CTRL_PIN, PIN_LOW);
#endif
}

void
VFD_printf(char const * fmt, ...)
{
    char buffer[100];
    va_list args;
    uint8_t len;

    va_start(args, fmt);

    len = vsnprintf(buffer, sizeof(buffer), fmt, args);

    va_end(args);

    /*
     * snprintf returns the number of characters that would have been written
     * if the buffer had been large enough (minus the NULL terminator). Since
     * we only want to end up writing data actually put in the buffer, we'll
     * check if the return value is larger than the buffer size and adjust
     * accordingly
     */
    if (len > sizeof(buffer) - 1)
        len = sizeof(buffer) - 1;

    write_data((uint8_t*)buffer, len);
}

void
VFD_write_string(char const *str)
{
    write_data((uint8_t const *)str, strlen(str));
}

/**
 * Sets the cursor position (relative to the current window)
 */
void
VFD_set_cursor(uint16_t x, uint16_t y)
{
    uint8_t buffer[] = {
        US,
        '$',
        PACK_16( x ),
        PACK_16( y )
    };

    write_buf(buffer);
}

/**
 * Clears the current window
 */
void
VFD_clear(void)
{
    uint8_t data = 0x0C;
    write_data(&data, 1);
}

/**
 * Enables or disables showing the cursor
 */
void
VFD_cursor_enable(bool on)
{
    uint8_t buffer[] = {
        US,
        'C',
        on
    };
    write_buf(buffer);
}

void
VFD_init(void)
{
    /*
     * Turn on the power and wait for the VFD to stabilize
     */
    pin_set_output(RESET_PIN, PIN_LOW);
    pin_set_output(POWER_CTRL_PIN, PIN_HIGH);
    timer_sleep(1000);
    pin_set_output(RESET_PIN, PIN_HIGH);

    /*
     * Wait for the busy pin to go low
     */
    while (pin_read(BUSY_PIN))
        ;
    timer_sleep(1000);

    VFD_soft_reset();
}

void
VFD_soft_reset(void)
{
    uint8_t i;
    uint8_t buffer[] = {
        0x1B,
        '@'
    };

    write_buf(buffer);

    cur_win = VFD_WIN_BASE;
    VFD_brightness_set(cur_brightness);

    win_enabled[VFD_WIN_BASE] = true;

    for (i = VFD_WIN_1; i < VFD_WIN_CNT; i++)
        win_enabled[i] = false;

    timer_sleep(10);
}

void
VFD_sel_font(VFD_font_t8 font)
{
    uint8_t buffer[] = {
        0x1B,
        'R',
        font
    };

    write_buf(buffer);
}

void
VFD_sel_char_code(VFD_char_code_t8 code)
{
    uint8_t buffer[] = {
        0x1B,
        't',
        code
    };
    write_buf(buffer);
}

void
VFD_overwite(void)
{
    uint8_t buffer[] = {
        US, 0x01
    };
    write_buf(buffer);
}

void
VFD_vert_scroll(void)
{
    uint8_t buffer[] = {
        US, 0x02
    };
    write_buf(buffer);
}

void
VFD_hort_scroll(void)
{
    uint8_t buffer[] = {
        US, 0x03
    };
    write_buf(buffer);
}

void
VFD_hort_scroll_speed(uint8_t speed)
{
    uint8_t buffer[] = {
        US, 's', speed
    };
    write_buf(buffer);
}

void
VFD_reverse(uint8_t rev)
{
    uint8_t buffer[] = {
        US, 'r', rev
    };
    write_buf(buffer);
}

void
VFD_mix_mode(VFD_mix_mode_t8 mode)
{
    uint8_t buffer[] = {
        US, 'w', mode
    };
    write_buf(buffer);
}

void
VFD_brightness_set(VFD_bright_lvl_t8 lvl)
{
    uint8_t buffer[] = {
        US, 'X', lvl
    };
    write_buf(buffer);
    cur_brightness = lvl;
}

VFD_bright_lvl_t8
VFD_brightness_get(void)
{
    return cur_brightness;
}

void
VFD_wait(uint8_t time)
{
    uint8_t buffer[] = {
        DISP_FUNC( 0x01 ), time
    };
    write_buf(buffer);
}

void
VFD_scroll(uint16_t width, uint16_t rep, uint8_t speed)
{
    uint8_t buffer[] = {
        DISP_FUNC(0x10), PACK_16(width), PACK_16(rep), speed
    };
    write_buf(buffer);
}

void
VFD_blink(VFD_pattern_t8 pattern, uint8_t on_time, uint8_t off_time,
        uint8_t reps)
{
    uint8_t buffer[] = {
        DISP_FUNC(0x11), pattern, on_time, off_time, reps
    };
    write_buf(buffer);
}

void
VFD_screen_saver(VFD_screen_saver_t8 ss)
{
    uint8_t buffer[] = {
        DISP_FUNC(0x40), ss
    };
    write_buf(buffer);
}

void
VFD_char_width(VFD_char_width_t8 wdth)
{
    uint8_t buffer[] = {
        FONT_FUNC(0x03), wdth
    };
    write_buf(buffer);
}

void
VFD_font_size(uint8_t x, uint8_t y)
{
    uint8_t buffer[] = {
        FONT_FUNC(0x40), x, y
    };
    write_buf(buffer);
}

void
VFD_win_select(VFD_win_t8 win)
{
    if ((cur_win != win) && win_enabled[win]) {
        write_data(&win_sel_tbl[win], 1);
        cur_win = win;
    }
}

bool
VFD_win_enabled(VFD_win_t8 win)
{
    return win_enabled[win];
}

void
VFD_create_window(VFD_win_t8 win, uint16_t left, uint16_t top, uint16_t wdth,
        uint16_t hght)
{
    uint8_t buffer[] = {
        WIN_FUNC(0x02), win, 1, PACK_16(left), PACK_16(top),
        PACK_16(wdth), PACK_16(hght)
    };

    write_buf(buffer);

    win_enabled[win] = true;

    /*
     * It is unclear from the documentation whether creating a window selects
     * it, so we'll do so just to be sure
     */
    VFD_win_select(win);
}

void
VFD_delete_window(VFD_win_t8 win)
{
    if (win != VFD_WIN_BASE) {
        uint8_t buffer[] = {
            WIN_FUNC(0x02), win, 0
        };

        write_buf(buffer);

        if (win == cur_win)
            VFD_win_select( VFD_WIN_BASE );

        win_enabled[win] = false;
    }
}

void
VFD_screen_mode(VFD_screen_mode_t8 mode)
{
    uint8_t buffer[] = {
        WIN_FUNC(0x10), mode
    };
    write_buf(buffer);
}
