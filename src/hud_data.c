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
#include <avr/eeprom.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elm327.h"
#include "hud_data.h"
#include "obd_data.h"
#include "timer.h"
#include "utl.h"

#define _s(_a) _a, cnt_of_array( _a )

#define SPD_FILTER_CONSTANT (0.5)
#define INST_FUEL_CONSTANT (0.5)
#define MAX_AVG_SAMPLES (10000)

/* Save the average fuel economy every 10 minutes */
#define AVG_ECON_WRITE_INTVL (10l * 60l * 1000l)

#define KPH_TO_MPH (0.641371192f)
#define KPA_TO_MMHG (7.50061683f)
#define KPA_TO_PSI (0.145038f)

#define AVG_ECON_VALID_SIG (0xAA)

/*
 * EEPROM Varaibles
 */
EEMEM struct avg_econ_ee_type {
    uint8_t sig;
    uint16_t avg_samples;
    float avg_econ_rate;
    float avg_econ_spd;
} avg_econ_ee_mem[2] =
    {
    {   AVG_ECON_VALID_SIG,   0,  0.0f,   0.0f    },
    {   AVG_ECON_VALID_SIG,   0,  0.0f,   0.0f    }
    };

static uint8_t EEMEM fuel_econ_always_on = true;

struct data_def_type {
    obd_pid_t8 const *pids;
    uint8_t cnt;
    void (*calc_func)(hud_data_t8 idx);
    char const *name;
};

static struct {
    uint8_t watched;
    bool valid;
    bool updated;
    char value[HUD_DATA_LEN];
} hud_data[HUD_DATA_CNT];

static uint8_t pid_ref_cnt[OBD_PID_CNT];
static obd_pid_t8 last_pid;
static uint16_t avg_samples;
static float avg_econ_spd;
static float avg_econ_rate;

static bool avg_econ_updated;
static struct timer avg_econ_timer;
static uint32_t avg_econ_num_writes;

static float last_speed_kph = 0.0f;
static float last_inst_econ = 0.0f;
static uint8_t last_fuel_lvl = 100;

static float
cont_filter(float input, float old_val, float constant)
{
    return ((input + old_val) * (1.0f - constant) ) / 2.0f + constant * old_val;
}

static void
set_float(hud_data_t8 d, float v, uint8_t precision)
{
    char *cur;
    bool in_dec = false;
    uint8_t dec_cnt = 0;

    snprintf(hud_data[d].value, sizeof(hud_data[d].value), "%f", (double)v);

    cur = hud_data[d].value;

    while (*cur) {
        if (*cur == '.') {
            if (precision == 0) {
                *cur ='\0';
                break;
            }

            in_dec = true;
        } else if (in_dec) {
            if (dec_cnt == precision) {
                *cur = '\0';
                break;
            }
            dec_cnt++;
        }

        cur++;
    }
}

static void
set_int(hud_data_t8 d, uint32_t v)
{
    snprintf(hud_data[d].value, sizeof(hud_data[d].value), "%lu", v);
}

static float
get_fuel_econ(float speed, float maf)
{
    /*
     * 14.7 grams of air to 1 gram of gasoline - ideal air/fuel ratio
     * 6.073 pounds per gallon - density of gasoline
     * 453.59 grams per pound - conversion
     * 0.621371 miles per hour/kilometers per hour - conversion
     * 3600 seconds per hour - conversion
     */
    return (14.7f * 6.073f * 453.59 * speed * KPH_TO_MPH) / (float)(3600 * maf);
}

static void
calc_speed_mph(hud_data_t8 idx)
{
    set_int(idx, OBD_get_speed() * KPH_TO_MPH);
}

static void
calc_speed_kph(hud_data_t8 idx)
{
    last_speed_kph = cont_filter(OBD_get_speed(), last_speed_kph,
            SPD_FILTER_CONSTANT);
    set_int(idx, last_speed_kph);
}

static void
calc_uncal_spd(hud_data_t8 idx)
{
    set_int(idx, OBD_get_uncal_speed());
}

static void
calc_rpm(hud_data_t8 idx)
{
    set_int(idx, OBD_get_rpm());
}

static void
calc_inst_econ(hud_data_t8 idx)
{
    last_inst_econ = cont_filter(
            get_fuel_econ(OBD_get_speed(), OBD_get_MAF_rate()),
            last_inst_econ, INST_FUEL_CONSTANT);
    set_float(idx, last_inst_econ, 1);
}

static void
calc_avg_econ(hud_data_t8 idx)
{
    float ratio;

    if (avg_samples == 0) {
        avg_econ_spd = OBD_get_speed();
        avg_econ_rate = OBD_get_MAF_rate();
    } else {
        ratio = ((float)avg_samples) / (avg_samples + 1);
        avg_econ_spd = cont_filter(OBD_get_speed(), avg_econ_spd, ratio);
        avg_econ_rate = cont_filter(OBD_get_MAF_rate(), avg_econ_rate, ratio);
    }

    set_float(idx, get_fuel_econ(avg_econ_spd, avg_econ_rate), 1);

    avg_econ_updated = true;

    if (avg_samples + 1 < MAX_AVG_SAMPLES)
        avg_samples++;
}

static void
calc_timer(hud_data_t8 idx)
{
    set_int(idx, timer_get() / 1000);
}

static void
calc_fuel_lvl(hud_data_t8 idx)
{
    uint8_t l;

    l = OBD_get_fuel_lvl();

    if (l < last_fuel_lvl)
        last_fuel_lvl = l;

    set_int(idx, last_fuel_lvl);
}

static void
calc_econ_write(hud_data_t8 idx)
{
    set_int(idx, avg_econ_num_writes);
}

static void
calc_last_pid(hud_data_t8 idx)
{
    set_int(idx, last_pid);
}

static void
calc_baro_pres_kpa(hud_data_t8 idx)
{
    set_int(idx, OBD_get_baro_pres());
}

static void
calc_baro_pres_mmhg(hud_data_t8 idx)
{
    set_float(idx, OBD_get_baro_pres() * KPA_TO_MMHG, 0);
}

static void
calc_air_temp_C(hud_data_t8 idx)
{
    set_int(idx, OBD_get_air_temp());
}

static void
calc_air_temp_F(hud_data_t8 idx)
{
    set_int(idx, (int32_t)((float)OBD_get_air_temp() * 1.8f) + 32);
}

static void
calc_boost(hud_data_t8 idx)
{
    /*
     * Boost pressure is strange... Positive values are PSI, negative
     * are mmHg
     */
    float value = OBD_get_intake_manifold_pres() - OBD_get_baro_pres();

    if (value >= 0.0)
        set_float(idx, value * KPA_TO_PSI, 0);
    else
        set_float(idx, value * KPA_TO_MMHG, 0);
}

static void
avg_econ_write_clbk(void *param)
{
    uint8_t sig;
    uint8_t i;

    if (avg_econ_updated) {
        /*
         * Write all the copies of the data
         */
        for (i = 0; i < cnt_of_array(avg_econ_ee_mem); i++) {
            /*
             * Write the invalid signature
             */
            sig = 0xFF;

            eeprom_write_byte(&avg_econ_ee_mem[i].sig, sig);

            /*
             * Write the rest of the data
             */
            eeprom_write_word(&avg_econ_ee_mem[i].avg_samples,
                    avg_samples);
            eeprom_write_float(&avg_econ_ee_mem[i].avg_econ_rate,
                    avg_econ_rate);
            eeprom_write_float(&avg_econ_ee_mem[i].avg_econ_spd,
                    avg_econ_spd);

            /*
             * Write the valid signature
             */
            sig = AVG_ECON_VALID_SIG;

            eeprom_write_byte(&avg_econ_ee_mem[i].sig, sig);
        }

        avg_econ_num_writes++;

        avg_econ_updated = false;
    }

    /*
     * Create the timer for the next write cycle
     */
    timer_create(&avg_econ_timer, AVG_ECON_WRITE_INTVL, avg_econ_write_clbk,
            NULL);
}

static const obd_pid_t8 speed_pids[]        = { OBD_PID_SPEED };
static const obd_pid_t8 rpm_pids[]          = { OBD_PID_ENGN_RPM };
static const obd_pid_t8 fuel_econ_pids[]    = { OBD_PID_MAF_RATE, OBD_PID_SPEED };
static const obd_pid_t8 fuel_lvl_pids[]     = { OBD_PID_FUEL_LVL_INPUT };
static const obd_pid_t8 baro_pres_pids[]    = { OBD_PID_BARO_PRES };
static const obd_pid_t8 air_temp_pids[]     = { OBD_PID_AMBIENT_AIR_TEMP };
static const obd_pid_t8 boost_pids[]        = { OBD_PID_BARO_PRES, OBD_PID_INTAKE_ABS_PRES };

static const struct data_def_type data_def[] =
{
    /* HUD_DATA_NONE            */  { NULL, 0,              NULL,                   "None"          },
    /* HUD_DATA_SPEED_MPH       */  { _s(speed_pids),       calc_speed_mph,         "MPH"           },
    /* HUD_DATA_SPEED_KPH       */  { _s(speed_pids),       calc_speed_kph,         "KPH"           },
    /* HUD_DATA_SPEED_UNCAL     */  { _s(speed_pids),       calc_uncal_spd,         "raw KPH"       },
    /* HUD_DATA_RPM             */  { _s(rpm_pids),         calc_rpm,               "RPM"           },
    /* HUD_DATA_INST_ECON       */  { _s(fuel_econ_pids),   calc_inst_econ,         "Inst MPG"      },
    /* HUD_DATA_AVG_ECON        */  { _s(fuel_econ_pids),   calc_avg_econ,          "Avg MPG"       },
    /* HUD_DATA_TIMER           */  { NULL, 0,              calc_timer,             "Timer"         },
    /* HUD_DATA_FUEL_LVL        */  { _s(fuel_lvl_pids),    calc_fuel_lvl,          "Fuel lvl"      },
    /* HUD_DATA_ECON_WRITE      */  { NULL, 0,              calc_econ_write,        "Fuel wrts"     },
    /* HUD_DATA_LAST_PID        */  { NULL, 0,              calc_last_pid,          "Last PID"      },
    /* HUD_DATA_BARO_PRES_KPA   */  { _s(baro_pres_pids),   calc_baro_pres_kpa,     "BP KPA"        },
    /* HUD_DATA_BARO_PRES_MMHG  */  { _s(baro_pres_pids),   calc_baro_pres_mmhg,    "BP mmHG"       },
    /* HUD_DATA_AIR_TEMP_C      */  { _s(air_temp_pids),    calc_air_temp_C,        "Outside C"     },
    /* HUD_DATA_AIR_TEMP_F      */  { _s(air_temp_pids),    calc_air_temp_F,        "Outside F"     },
    /* HUD_DATA_BOOST           */  { _s(boost_pids),       calc_boost,             "Boost"         },
};

STATIC_ASSERT(cnt_of_array(data_def) == HUD_DATA_CNT);

static void
check_data(hud_data_t8 d, obd_pid_t8 pid)
{
    uint8_t k;
    bool valid;
    char old_data[HUD_DATA_LEN];

    valid = true;

    if (hud_data[d].watched) {
        if (data_def[d].cnt == 0) {
            valid = true;
        } else {
            for (k = 0; k < data_def[d].cnt; k++) {
                if (!OBD_is_valid(data_def[d].pids[k]))
                    valid = false;
            }
        }

        hud_data[d].valid = valid;

        /*
         * Calculate a new value if this data value is valid, and it either has
         * no dependant PID's, or the first PID in the list is the one we are
         * checking. Note that we don't check if any PID in the list is equal
         * to the updated one, because it would cause the calculation to be run
         * multiple times and each time only one data value could possibly
         * change
         */
        if (valid && (data_def[d].cnt == 0 || data_def[d].pids[0] == pid)) {
            if (data_def[d].calc_func) {
                memcpy(old_data, hud_data[d].value, HUD_DATA_LEN);

                data_def[d].calc_func(d);

                if (strcmp(old_data, hud_data[d].value) != 0)
                    hud_data[d].updated = true;
            }
        }
    }
}


void
HUD_data_add(hud_data_t8 d)
{
    uint8_t i;

    if (!hud_data[d].watched) {
        for (i = 0; i < data_def[d].cnt; i++)
            pid_ref_cnt[data_def[d].pids[i]]++;

        hud_data[d].valid = false;

        /*
         * Mark data as updated so we show it as soon as it is valid
         */
        hud_data[d].updated = true;
    }

    hud_data[d].watched++;
}

bool
HUD_data_remove(hud_data_t8 d)
{
    uint8_t i;

    hud_data[d].watched--;

    if (!hud_data[d].watched) {
        for (i = 0; i < data_def[d].cnt; i++)
            pid_ref_cnt[data_def[d].pids[i]]--;

        return true;
    }

    return false;
}

void
HUD_data_init(void)
{
    uint8_t i;
    uint8_t sig;

    OBD_data_init();

    for (i = 0; i < HUD_DATA_CNT; i++) {
        hud_data[i].watched = 0;
        hud_data[i].valid = false;
        hud_data[i].updated = false;
    }

    for (i = 0; i < OBD_PID_CNT; i++)
        pid_ref_cnt[i] = 0;

    last_pid = OBD_PID_CNT;

    avg_samples = 0;

    avg_econ_spd = 0;
    avg_econ_rate = 0;

    avg_econ_updated = false;

    /*
     * Try to read the EE memory
     */
    for (i = 0; i < cnt_of_array(avg_econ_ee_mem); i++) {
        /*
         * Read the signature
         */
        sig = eeprom_read_byte(&avg_econ_ee_mem[i].sig);

        /*
         * If the signature is valid, read the data
         */
        if (sig == AVG_ECON_VALID_SIG) {
            avg_samples = eeprom_read_word(
                    &avg_econ_ee_mem[i].avg_samples);
            avg_econ_spd = eeprom_read_float(
                    &avg_econ_ee_mem[i].avg_econ_spd);
            avg_econ_rate = eeprom_read_float(
                    &avg_econ_ee_mem[i].avg_econ_rate);
            break;
        }
    }

    timer_create(&avg_econ_timer, AVG_ECON_WRITE_INTVL, avg_econ_write_clbk,
            NULL);

    if (eeprom_read_byte(&fuel_econ_always_on))
        HUD_data_add(HUD_DATA_AVG_ECON);
}

void
HUD_set_fuel_econ_always_on(bool always_on)
{
    if (always_on != eeprom_read_byte(&fuel_econ_always_on)) {
        if (always_on)
            HUD_data_add(HUD_DATA_AVG_ECON);
        else
            HUD_data_remove(HUD_DATA_AVG_ECON);

        eeprom_write_byte(&fuel_econ_always_on, always_on);
    }
}

bool
HUD_get_fuel_econ_always_on(void)
{
    return eeprom_read_byte(&fuel_econ_always_on);
}

bool
HUD_data_get(hud_data_t8 d, char value[HUD_DATA_LEN])
{
    if (hud_data[d].watched && hud_data[d].valid) {
        memcpy(value, hud_data[d].value, HUD_DATA_LEN);
        hud_data[d].updated = false;
        return true;
    }

    value[0] = '\0';
    return false;
}

char const *
HUD_data_name(hud_data_t8 d)
{
    return data_def[d].name;
}

bool
HUD_data_valid(hud_data_t8 d)
{
    return hud_data[d].valid;
}

void
HUD_process(void)
{
    obd_pid_t8 i;
    obd_pid_t8 next_pid;

    ELM327_process(false);

    if (ELM327_is_ready()) {
        for (i = 0; i < HUD_DATA_CNT; i++)
            check_data(i, last_pid);

        for (i = 0; i < OBD_PID_CNT; i++) {
            next_pid = (last_pid + i + 1) % OBD_PID_CNT;
            if (pid_ref_cnt[next_pid]) {
                ELM327_rqst_crnt_pid(next_pid);
                break;
            }
        }

        if (i != OBD_PID_CNT)
            last_pid = next_pid;
        else
            last_pid = OBD_PID_CNT;
    }
}


bool
HUD_data_updated(hud_data_t8 d)
{
    return hud_data[d].watched && hud_data[d].valid && hud_data[d].updated;
}


