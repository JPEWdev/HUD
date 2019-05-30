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
#include <stdlib.h>

#include "elm327.h"
#include "obd_data.h"
#include "utl.h"

#define RPM_ROUND (100 * 4)
#define SPEED_HYST (2)

typedef void (*data_proc_type)(uint8_t const *data, uint8_t len);

struct speed_cal_type {
    uint8_t in_spd;
    uint8_t out_spd;
};

static uint8_t my_engn_load;
static int16_t my_engn_clnt_temp;
static uint8_t my_uncal_speed;
static uint8_t my_speed;
static float my_MAF_rate;
static uint16_t my_rpm;
static uint8_t my_fuel_lvl;
static uint8_t my_baro_pres;
static int16_t my_air_temp;
static uint8_t my_intake_manifold_pres;
static int16_t my_engn_oil_temp;

static bool data_valid[ OBD_PID_CNT ];

uint8_t
OBD_get_engn_load(void)
{
    return my_engn_load;
}

int16_t
OBD_get_engn_clnt_temp(void)
{
    return my_engn_clnt_temp;
}

uint16_t
OBD_get_rpm(void)
{
    return my_rpm;
}

uint8_t
OBD_get_speed(void)
{
    return my_speed;
}

uint8_t
OBD_get_uncal_speed(void)
{
    return my_uncal_speed;
}

float
OBD_get_MAF_rate(void)
{
    return my_MAF_rate;
}

uint8_t
OBD_get_fuel_lvl(void)
{
    return my_fuel_lvl;
}

uint8_t
OBD_get_baro_pres(void)
{
    return my_baro_pres;
}

int16_t
OBD_get_air_temp(void)
{
    return my_air_temp;
}

uint8_t
OBD_get_intake_manifold_pres(void)
{
    return my_intake_manifold_pres;
}

int16_t
OBD_get_engn_oil_temp(void)
{
    return my_engn_oil_temp;
}

bool
OBD_is_valid(obd_pid_t8 pid)
{
    return data_valid[pid];
}

static void
set_engn_load(uint8_t const *data, uint8_t len)
{
    if (len < 1)
        return;
    my_engn_load = ((uint16_t)data[0] * 100) / 255;
}

static void
set_engn_clnt_temp(uint8_t const *data, uint8_t len)
{
    if (len < 1)
        return;
    my_engn_clnt_temp = (int16_t)data[0] - 40;
}

static void
set_rpm(uint8_t const *data, uint8_t len)
{
    if (len < 2)
        return;

    my_rpm = ((uint16_t)data[0] << 8) | data[1];
    my_rpm = my_rpm / (RPM_ROUND / 2);

    /*
     * If odd add 1 to round up
     */
    if (my_rpm & 0x01)
        my_rpm += 1;

    my_rpm *= RPM_ROUND / 2;
    my_rpm /= 4;
}

static void
set_speed(uint8_t const *data, uint8_t len)
{
    if (len < 1)
        return;
    my_speed = data[0];
}

static void
set_MAF_rate(uint8_t const *data, uint8_t len)
{
    if (len < 2)
        return;
    my_MAF_rate = (((uint16_t)data[0] << 8) | data[1]) / 100.0f;
}


static void
set_fuel_lvl(uint8_t const *data, uint8_t len)
{
    if (len < 1)
        return;
    my_fuel_lvl = (100 * (uint16_t)data[0]) / 255;
}

static void
set_baro_pres(uint8_t const *data, uint8_t len)
{
    if (len < 1)
        return;
    my_baro_pres = data[0];
}

static void
set_air_temp(uint8_t const *data, uint8_t len)
{
    if (len < 1)
        return;
    my_air_temp = ((int16_t)data[0]) - 40;
}

static void
set_intake_manifold_pres(uint8_t const *data, uint8_t len)
{
    if (len < 1)
        return;
    my_intake_manifold_pres = data[0];
}

static void
set_engn_oil_temp(uint8_t const *data, uint8_t len)
{
    if (len < 1)
        return;
    my_engn_oil_temp = (int16_t)data[0] - 40;
}

static const data_proc_type data_procs[OBD_PID_CNT] = {
    [OBD_PID_ENGN_LOAD] = set_engn_load,
    [OBD_PID_ENGN_CLNT_TEMP] = set_engn_clnt_temp,
    [OBD_PID_INTAKE_ABS_PRES] = set_intake_manifold_pres,
    [OBD_PID_ENGN_RPM] = set_rpm,
    [OBD_PID_SPEED] = set_speed,
    [OBD_PID_MAF_RATE ] = set_MAF_rate,
    [OBD_PID_FUEL_LVL_INPUT] = set_fuel_lvl,
    [OBD_PID_BARO_PRES] =  set_baro_pres,
    [OBD_PID_AMBIENT_AIR_TEMP] = set_air_temp,
    [OBD_PID_ENGN_OIL_TEMP] = set_engn_oil_temp,
    };

static void
data_clbk(obd_pid_t8 pid, uint8_t const *data, uint8_t len)
{
    if (pid < OBD_PID_CNT && data_procs[pid]) {
        data_procs[pid](data, len);
        data_valid[pid] = true;
    }
}

static void
no_data_clbk(obd_pid_t8 pid)
{
    if (pid < OBD_PID_CNT)
        data_valid[pid] = false;
}


void
OBD_data_init(void)
{
    obd_pid_t8 i;

    ELM327_set_echo(false);
    ELM327_set_clbk(data_clbk, no_data_clbk);

    for (i = 0; i < OBD_PID_CNT; i++)
        data_valid[i] = false;
}

