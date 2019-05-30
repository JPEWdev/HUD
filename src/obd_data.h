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
#ifndef _OBD_DATA_H_
#define _OBD_DATA_H_

#include <stdbool.h>
#include <stdint.h>

#include "obd_pid.h"

void
OBD_data_init(void);

uint8_t
OBD_get_engn_load(void);

int16_t
OBD_get_engn_clnt_temp(void);

uint16_t
OBD_get_rpm(void);

uint8_t
OBD_get_speed(void);

uint8_t
OBD_get_uncal_speed(void);

float
OBD_get_MAF_rate(void);

uint8_t
OBD_get_fuel_lvl(void);

uint8_t
OBD_get_baro_pres(void);

int16_t
OBD_get_air_temp(void);

uint8_t
OBD_get_intake_manifold_pres(void);

int16_t
OBD_get_engn_oil_temp(void);

bool
OBD_is_valid(obd_pid_t8 pid);

#endif /* _OBD_DATA_H_ */
