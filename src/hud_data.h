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
#ifndef _HUD_DATA_H_
#define _HUD_DATA_H_

#include <stdbool.h>

#define HUD_DATA_LEN (6)

typedef uint8_t hud_data_t8; enum {
    HUD_DATA_NONE,
    HUD_DATA_SPEED_MPH,
    HUD_DATA_SPEED_KPH,
    HUD_DATA_SPEED_UNCAL,
    HUD_DATA_RPM,
    HUD_DATA_INST_ECON,
    HUD_DATA_AVG_ECON,
    HUD_DATA_TIMER,
    HUD_DATA_FUEL_LVL,
    HUD_DATA_ECON_WRITE,
    HUD_DATA_LAST_PID,
    HUD_DATA_BARO_PRES_KPA,
    HUD_DATA_BARO_PRES_MMHG,
    HUD_DATA_AIR_TEMP_C,
    HUD_DATA_AIR_TEMP_F,
    HUD_DATA_BOOST,
    HUD_DATA_COOLANT_TEMP_C,
    HUD_DATA_COOLANT_TEMP_F,
    HUD_DATA_OIL_TEMP_C,
    HUD_DATA_OIL_TEMP_F,

    HUD_DATA_CNT
};

void
HUD_data_add(hud_data_t8 d);

bool
HUD_data_remove(hud_data_t8 d);

void
HUD_data_init(void);

void
HUD_set_fuel_econ_always_on(bool always_on);

bool
HUD_get_fuel_econ_always_on(void);

bool
HUD_data_get(hud_data_t8, char value[HUD_DATA_LEN]);

bool
HUD_data_valid(hud_data_t8 d);

void
HUD_process(void);

bool
HUD_data_updated(hud_data_t8 d);

char const *
HUD_data_name(hud_data_t8 d);

#endif /* _HUD_DATA_H_ */
