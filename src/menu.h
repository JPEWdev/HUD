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
#ifndef _MENU_H_
#define _MENU_H_

#include <stdint.h>

#include "layout.h"

enum menu_id {
    MENU_RESERVED = 10000,
    MENU_NONE = MENU_RESERVED,
    MENU_YES,
    MENU_NO,
    MENU_BACK,
    MENU_TIMEOUT
};

struct menu_type {
    enum menu_id id;
    char const *string;
    enum menu_id (*proc)(enum menu_id id, void *param);
};

enum menu_id
menu_prompt(char const *prompt, struct menu_type const *menu, int cnt,
        uint8_t cur, void *param);

enum menu_id
menu_process(struct screen_layout_type const *layout,
        struct menu_type const *menu, int cnt, uint8_t cur, void *param);

enum menu_id
menu_yes_no(char const *prompt);

#endif /* _MENU_H_ */

