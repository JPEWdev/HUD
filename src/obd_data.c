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

typedef void (*data_proc_type)(uint8_t const *data);

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

bool
OBD_is_valid(obd_pid_t8 pid)
{
    return data_valid[pid];
}

static void
set_engn_load(uint8_t const *data)
{
    my_engn_load = ((uint16_t)data[0] * 100) / 255;
}

static void
set_engn_clnt_temp(uint8_t const *data)
{
    my_engn_clnt_temp = (int16_t)data[0] - 40;
}

static void
set_rpm(uint8_t const *data)
{
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
set_speed(uint8_t const *data)
{
    my_speed = data[0];
}

static void
set_MAF_rate(uint8_t const *data)
{
    my_MAF_rate = (((uint16_t)data[0] << 8) | data[1]) / 100.0f;
}


static void
set_fuel_lvl(uint8_t const *data)
{
    my_fuel_lvl = (100 * (uint16_t)data[0]) / 255;
}

static void
set_baro_pres(uint8_t const *data)
{
    my_baro_pres = data[0];
}

static void
set_air_temp(uint8_t const *data)
{
    my_air_temp = ((int16_t)data[0]) - 40;
}

static void
set_intake_manifold_pres(uint8_t const *data)
{
    my_intake_manifold_pres = data[0];
}

static const data_proc_type data_procs[] = {
    /* OBD_PID_SUPPORT_1                */  NULL,
    /* OBD_PID_MONITOR_STATUS           */  NULL,
    /* OBD_PID_FREEZE_DTC               */  NULL,
    /* OBD_PID_FUEL_SYS_STATUS          */  NULL,
    /* OBD_PID_ENGN_LOAD                */  set_engn_load,
    /* OBD_PID_ENGN_CLNT_TEMP           */  set_engn_clnt_temp,
    /* OBD_PID_SHORT_FUEL_TRIM_1        */  NULL,
    /* OBD_PID_LONG_FUEL_TRIM_1         */  NULL,
    /* OBD_PID_SHORT_FUEL_TRIM_2        */  NULL,
    /* OBD_PID_LONG_FUEL_TRIM_2         */  NULL,
    /* OBD_PID_FUEL_PRES                */  NULL,
    /* OBD_PID_INTAKE_ABS_PRES          */  set_intake_manifold_pres,
    /* OBD_PID_ENGN_RPM                 */  set_rpm,
    /* OBD_PID_SPEED                    */  set_speed,
    /* OBD_PID_TIMING_ADV               */  NULL,
    /* OBD_PID_INTAKE_AIR_TEMP          */  NULL,
    /* OBD_PID_MAF_RATE                 */  set_MAF_rate,
    /* OBD_PID_THROTTLE_POS             */  NULL,
    /* OBD_PID_CMD_SEC_AIR_STAT         */  NULL,
    /* OBD_PID_O2_PRESENT_1             */  NULL,
    /* OBD_PID_O2_V_BANK_1_S_1          */  NULL,
    /* OBD_PID_O2_V_BANK_1_S_2          */  NULL,
    /* OBD_PID_O2_V_BANK_1_S_3          */  NULL,
    /* OBD_PID_O2_V_BANK_1_S_4          */  NULL,
    /* OBD_PID_O2_V_BANK_2_S_1          */  NULL,
    /* OBD_PID_O2_V_BANK_2_S_2          */  NULL,
    /* OBD_PID_O2_V_BANK_2_S_3          */  NULL,
    /* OBD_PID_O2_V_BANK_2_S_4          */  NULL,
    /* OBD_PID_OBD_STANDARDS            */  NULL,
    /* OBD_PID_O2_PRESENT_2             */  NULL,
    /* OBD_PID_AUX_INPUT_STAT           */  NULL,
    /* OBD_PID_RUN_TIME                 */  NULL,
    /* OBD_PID_SUPPORT_2                */  NULL,
    /* OBD_PID_DIST_W_MIL               */  NULL,
    /* OBD_PID_FUEL_RAIL_PRES           */  NULL,
    /* OBD_PID_FUEL_RAIL_PRES_DSL       */  NULL,
    /* OBD_PID_O2S1_WR_LAMBDA_V         */  NULL,
    /* OBD_PID_O2S2_WR_LAMBDA_V         */  NULL,
    /* OBD_PID_O2S3_WR_LAMBDA_V         */  NULL,
    /* OBD_PID_O2S4_WR_LAMBDA_V         */  NULL,
    /* OBD_PID_O2S5_WR_LAMBDA_V         */  NULL,
    /* OBD_PID_O2S6_WR_LAMBDA_V         */  NULL,
    /* OBD_PID_O2S7_WR_LAMBDA_V         */  NULL,
    /* OBD_PID_O2S8_WR_LAMBDA_V         */  NULL,
    /* OBD_PID_CMNDED_EGR               */  NULL,
    /* OBD_PID_EGR_ERROR                */  NULL,
    /* OBD_PID_CMND_EVAP_PURGE          */  NULL,
    /* OBD_PID_FUEL_LVL_INPUT           */  set_fuel_lvl,
    /* OBD_PID_NUM_WARM_UPS_SINCE_CLEAR */  NULL,
    /* OBD_PID_DIST_SINCE_CLEAR         */  NULL,
    /* OBD_PID_EVAP_PRES                */  NULL,
    /* OBD_PID_BARO_PRES                */  set_baro_pres,
    /* OBD_PID_O2S1_WR_LAMBDA_I         */  NULL,
    /* OBD_PID_O2S2_WR_LAMBDA_I         */  NULL,
    /* OBD_PID_O2S3_WR_LAMBDA_I         */  NULL,
    /* OBD_PID_O2S4_WR_LAMBDA_I         */  NULL,
    /* OBD_PID_O2S5_WR_LAMBDA_I         */  NULL,
    /* OBD_PID_O2S6_WR_LAMBDA_I         */  NULL,
    /* OBD_PID_O2S7_WR_LAMBDA_I         */  NULL,
    /* OBD_PID_O2S8_WR_LAMBDA_I         */  NULL,
    /* OBD_PID_CAT_TEMP_BANK_1_S_1      */  NULL,
    /* OBD_PID_CAT_TEMP_BANK_2_S_1      */  NULL,
    /* OBD_PID_CAT_TEMP_BANK_1_S_2      */  NULL,
    /* OBD_PID_CAT_TEMP_BANK_2_S_2      */  NULL,
    /* OBD_PID_SUPPORT_3                */  NULL,
    /* OBD_PID_MONITOR_STATUS           */  NULL,
    /* OBD_PID_CM_V                     */  NULL,
    /* OBD_PID_ABS_LOAD                 */  NULL,
    /* OBD_PID_FUEL_AIR_CMD_RATIO       */  NULL,
    /* OBD_PID_REL_THROTTLE             */  NULL,
    /* OBD_PID_AMBIENT_AIR_TEMP         */  set_air_temp,
    };

STATIC_ASSERT(cnt_of_array(data_procs) == OBD_PID_CNT);
STATIC_ASSERT(OBD_PID_CNT == 0x47);

static void
data_clbk(obd_pid_t8 pid, uint8_t const *data)
{
    if (pid < OBD_PID_CNT && data_procs[pid]) {
        data_procs[pid](data);
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

