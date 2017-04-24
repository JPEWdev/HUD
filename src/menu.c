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
#include <stddef.h>

#include "btn.h"
#include "layout.h"
#include "menu.h"
#include "timer.h"
#include "utl.h"
#include "vfd.h"

#define TIMEOUT 60000

static void
draw_menu(struct screen_layout_type const *layout, struct menu_type const *menu,
        int menu_cnt, int cur_idx)
{
    int first_idx;
    int last_idx;
    int i;

    // Calculate first and last visible indexes
    first_idx = (cur_idx / layout->wndw_cnt) * layout->wndw_cnt;
    last_idx = first_idx + layout->wndw_cnt - 1;

    if (last_idx >= menu_cnt)
        last_idx = menu_cnt - 1;

    // Reset display
    VFD_soft_reset();

    for (i = 0; i < layout->wndw_cnt && (first_idx + i) <= last_idx; i++) {
        VFD_set_cursor(layout->windows[i].x, layout->windows[i].y);
        VFD_reverse(first_idx + i == cur_idx);

        VFD_char_width(VFD_CHAR_WDTH_PROP_2);
        VFD_printf("%c", menu[first_idx + i].string[0]);

        if (menu[first_idx + i].string[0]) {
            VFD_char_width(VFD_CHAR_WDTH_PROP_1);
            VFD_printf("%s", &menu[first_idx + i].string[1]);
        }
    }
    VFD_reverse(false);
    VFD_font_size(1, 1);
}

static const struct screen_layout_type menu_prompt_layout = {
    "Prompt",
    2,
    {
        {   VFD_WIN_1, 0,            VFD_LINE_HGHT / 2, VFD_WDTH / 2,   VFD_LINE_HGHT / 2,  1   },
        {   VFD_WIN_2, VFD_WDTH / 2, VFD_LINE_HGHT / 2, VFD_WDTH / 2,   VFD_LINE_HGHT / 2,  1   },
    }
};

static enum menu_id
process(char const *prompt, struct screen_layout_type const *layout,
        struct menu_type const *menu, int cnt, uint8_t cur, void *param)
{
    int cur_idx = cur;
    uint8_t btn;
    enum menu_id ret = MENU_NONE;

    if (cur_idx >= cnt)
        cur_idx = 0;

    while (ret == MENU_NONE) {
        // Draw menu
        draw_menu(layout, menu, cnt, cur_idx);

        if (prompt) {
            VFD_set_cursor(0, 0);
            VFD_printf("%s", prompt);
        }

        // Process input
        while (true) {
            btn = BTN_wait(TIMEOUT);

            if (!btn) {
                ret = MENU_TIMEOUT;
                goto done;
            }

            if (btn & BTN_L_PRESS) {
                // Decrement the current index if possible
                if (cur_idx > 0)
                    cur_idx--;
                else
                    cur_idx = cnt - 1;

                // Redraw the display
                break;
            }

            if (btn & BTN_R_PRESS) {
                if (cur_idx < cnt - 1)
                    cur_idx++;
                else
                    cur_idx = 0;

                //Redraw the display
                break;
            }

            if (btn & BTN_L_HOLD) {
                // Exit menu
                ret = MENU_BACK;
                goto done;
            }

            if (btn & BTN_R_HOLD) {
                // Enter menu procedure if defined, otherwise just return
                // the ID
                if (menu[cur_idx].proc)
                    ret = menu[cur_idx].proc(menu[cur_idx].id, param);
                else
                    ret = menu[cur_idx].id;

                // Redraw menu (or exit, depending on return code)
                break;
            }
        }

    }

done:
    VFD_soft_reset();
    return ret;
}

enum menu_id
menu_prompt(char const *prompt, struct menu_type const *menu, int cnt,
        uint8_t cur, void *param)
{
    return process(prompt, &menu_prompt_layout, menu, cnt, cur, param);
}

enum menu_id
menu_process(struct screen_layout_type const *layout,
        struct menu_type const *menu, int cnt, uint8_t cur, void *param)
{
    return process(NULL, layout, menu, cnt, cur, param);
}

enum menu_id
menu_yes_no(char const *prompt)
{
    static const struct menu_type data[] = {
        {   MENU_YES,   "Yes",  NULL    },
        {   MENU_NO,    "No",   NULL    },
    };

    return menu_prompt(prompt, data, cnt_of_array(data), 1, NULL);
}

