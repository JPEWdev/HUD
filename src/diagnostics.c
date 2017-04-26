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
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#include "btn.h"
#include "elm327.h"
#include "layout.h"
#include "menu.h"
#include "obd_pid.h"
#include "timer.h"
#include "utl.h"
#include "vfd.h"

static bool
test_btn_process(uint8_t btn)
{
    if (btn & BTN_L_PRESS)
        return true;

    if (btn & BTN_R_PRESS)
        btn = BTN_wait(0);

    return false;
}

static enum menu_id
display_diagnostics_menu(enum menu_id id, void *param)
{
    int i;
    uint32_t start;

    VFD_soft_reset();
    VFD_char_width(VFD_CHAR_WDTH_FIXED_1);
    VFD_printf("Press L to cancel");
    VFD_set_cursor(0, 1);
    VFD_printf("Press R to pause");
    if (test_btn_process(BTN_wait(5000)))
        goto done;

    VFD_clear();

    VFD_printf("#1: Pixel Test");
    if (test_btn_process(BTN_wait(1000)))
        goto done;

    VFD_clear();
    VFD_screen_saver(VFD_SCREEN_SAVER_ALL_ON);
    if (test_btn_process(BTN_wait(4000)))
        goto done;

    VFD_clear();
    VFD_printf("#2: Data Rate Test");
    if (test_btn_process(BTN_wait(1000)))
        goto done;

    start = timer_get();
    for (i = 0; i < 2600; i++) {
        char buf[2] = "a";

        buf[0] = 'a' + i % 26;
        VFD_write_string(buf);
        if (test_btn_process(BTN_process()))
            goto done;
    }
    VFD_clear();
    VFD_printf("Rate: %lu bits/sec", CHAR_BIT * 2600ul * 1000ul / (timer_get() - start));
    if (test_btn_process(BTN_wait(5000)))
        goto done;

    VFD_clear();
    VFD_printf("#3: Window Test");
    if (test_btn_process(BTN_wait(1000)))
        goto done;

    create_layout_windows(&layout_4);
    start = timer_get();

    while (timer_get() - start < 10000) {
        for (i = 0; i < layout_4.wndw_cnt; i++) {
            VFD_win_select(layout_4.windows[i].window);

            VFD_clear();
            VFD_printf("%lu", timer_get());

            if (test_btn_process(BTN_wait(10)))
                goto done;
        }
    }

done:
    VFD_soft_reset();
    return MENU_NONE;
}

static enum menu_id
show_pid_menu(enum menu_id id, void *param)
{
    obd_pid_t8 pid = id;
    uint8_t buffer[OBD_PID_MAX_LEN];
    size_t len;
    size_t i;

    VFD_soft_reset();
    VFD_char_width(VFD_CHAR_WDTH_FIXED_1);
    VFD_printf("PID 0x%02x:", pid);
    VFD_set_cursor(0, 1);

    if (ELM327_get_crnt_pid(pid, buffer, &len)) {
        for(i = 0; i < len; i++ )
            VFD_printf("%02x ", buffer[i]);
    } else {
        VFD_printf("No Data");
    }

    BTN_wait(10000);

    VFD_soft_reset();
    return MENU_NONE;
}

static struct menu_type pid_menu_items[OBD_PID_CNT + 1];
static char pid_menu_text[OBD_PID_CNT][5];
static size_t pid_menu_count;

static void
add_pid_menu(obd_pid_t8 pid)
{
    snprintf(pid_menu_text[pid_menu_count],
            sizeof(pid_menu_text[pid_menu_count]), "0x%02x", pid);
    pid_menu_items[pid_menu_count].id = pid;
    pid_menu_items[pid_menu_count].string = pid_menu_text[pid_menu_count];
    pid_menu_items[pid_menu_count].proc = show_pid_menu;
    pid_menu_count++;
}


static enum menu_id
pid_menu(enum menu_id id, void *param)
{
    obd_pid_t8 pid;
    obd_pid_t8 i, j;
    uint8_t buffer[OBD_PID_MAX_LEN];
    size_t len;
    enum menu_id m;

    pid_menu_count = 0;

    for (i = 0; i < OBD_PID_CNT; i += 32) {
        add_pid_menu(i);
        if(ELM327_get_crnt_pid(i, buffer, &len)) {
            for (j = 0; j < 32 && i + j < OBD_PID_CNT; j++) {
                pid = i + j;
                if (buffer[j / 8] & _BV(j % 8))
                    add_pid_menu(pid);
            }
        }
    }
    pid_menu_items[pid_menu_count].id = MENU_BACK;
    pid_menu_items[pid_menu_count].string = "Back";
    pid_menu_items[pid_menu_count].proc = NULL;
    pid_menu_count++;

    m = menu_process(&layout_4, pid_menu_items, pid_menu_count, 0, NULL);

    return (m == MENU_TIMEOUT) ? m : MENU_NONE;
}

enum menu_id
diagnostics_menu(enum menu_id id, void *param)
{
    static const struct menu_type menu[] = {
        {   MENU_NONE,  "Display",  display_diagnostics_menu    },
        {   MENU_NONE,  "PID",      pid_menu                    },
        {   MENU_BACK,  "Back",     NULL                        },
    };

    enum menu_id m;

    m = menu_process(&layout_4, menu, cnt_of_array(menu), 0, NULL);

    return (m == MENU_TIMEOUT) ? m : MENU_NONE;
}


