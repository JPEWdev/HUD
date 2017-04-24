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
#ifndef _OBD_PID_H_
#define _OBD_PID_H_

#include <stdint.h>

typedef uint8_t obd_mode_t8; enum {
    OBD_SHOW_DATA = 0x01,
    OBD_FREEZE_DATA = 0x02,
    OBD_SHOW_DTC = 0x03,
    OBD_CLEAR_DTC = 0x04
};

typedef uint8_t obd_pid_t8; enum {
    OBD_PID_SUPPORT_1,                  /* 0x00 */
    OBD_PID_MONITOR_STATUS,             /* 0x01 */
    OBD_PID_FREEZE_DTC,                 /* 0x02 */
    OBD_PID_FUEL_SYS_STATUS,            /* 0x03 */
    OBD_PID_ENGN_LOAD,                  /* 0x04 */
    OBD_PID_ENGN_CLNT_TEMP,             /* 0x05 */
    OBD_PID_SHORT_FUEL_TRIM_1,          /* 0x06 */
    OBD_PID_LONG_FUEL_TRIM_1,           /* 0x07 */
    OBD_PID_SHORT_FUEL_TRIM_2,          /* 0x08 */
    OBD_PID_LONG_FUEL_TRIM_2,           /* 0x09 */
    OBD_PID_FUEL_PRES,                  /* 0x0A */
    OBD_PID_INTAKE_ABS_PRES,            /* 0x0B */
    OBD_PID_ENGN_RPM,                   /* 0x0C */
    OBD_PID_SPEED,                      /* 0x0D */
    OBD_PID_TIMING_ADV,                 /* 0x0E */
    OBD_PID_INTAKE_AIR_TEMP,            /* 0x0F */
    OBD_PID_MAF_RATE,                   /* 0x10 */
    OBD_PID_THROTTLE_POS,               /* 0x11 */
    OBD_PID_CMD_SEC_AIR_STAT,           /* 0x12 */
    OBD_PID_O2_PRESENT_1,               /* 0x13 */
    OBD_PID_O2_V_BANK_1_S_1,            /* 0x14 */
    OBD_PID_O2_V_BANK_1_S_2,            /* 0x15 */
    OBD_PID_O2_V_BANK_1_S_3,            /* 0x16 */
    OBD_PID_O2_V_BANK_1_S_4,            /* 0x17 */
    OBD_PID_O2_V_BANK_2_S_1,            /* 0x18 */
    OBD_PID_O2_V_BANK_2_S_2,            /* 0x19 */
    OBD_PID_O2_V_BANK_2_S_3,            /* 0x1A */
    OBD_PID_O2_V_BANK_2_S_4,            /* 0x1B */
    OBD_PID_OBD_STANDARDS,              /* 0x1C */
    OBD_PID_O2_PRESENT_2,               /* 0x1D */
    OBD_PID_AUX_INPUT_STAT,             /* 0x1E */
    OBD_PID_RUN_TIME,                   /* 0x1F */
    OBD_PID_SUPPORT_2,                  /* 0x20 */
    OBD_PID_DIST_W_MIL,                 /* 0x21 */
    OBD_PID_FUEL_RAIL_PRES,             /* 0x22 */
    OBD_PID_FUEL_RAIL_PRES_DSL,         /* 0x23 */
    OBD_PID_O2S1_WR_LAMBDA_V,           /* 0x24 */
    OBD_PID_O2S2_WR_LAMBDA_V,           /* 0x25 */
    OBD_PID_O2S3_WR_LAMBDA_V,           /* 0x26 */
    OBD_PID_O2S4_WR_LAMBDA_V,           /* 0x27 */
    OBD_PID_O2S5_WR_LAMBDA_V,           /* 0x28 */
    OBD_PID_O2S6_WR_LAMBDA_V,           /* 0x29 */
    OBD_PID_O2S7_WR_LAMBDA_V,           /* 0x2A */
    OBD_PID_O2S8_WR_LAMBDA_V,           /* 0x2B */
    OBD_PID_CMNDED_EGR,                 /* 0x2C */
    OBD_PID_EGR_ERROR,                  /* 0x2D */
    OBD_PID_CMND_EVAP_PURGE,            /* 0x2E */
    OBD_PID_FUEL_LVL_INPUT,             /* 0x2F */
    OBD_PID_NUM_WARM_UPS_SINCE_CLEAR,   /* 0x30 */
    OBD_PID_DIST_SINCE_CLEAR,           /* 0x31 */
    OBD_PID_EVAP_PRES,                  /* 0x32 */
    OBD_PID_BARO_PRES,                  /* 0x33 */
    OBD_PID_O2S1_WR_LAMBDA_I,           /* 0x34 */
    OBD_PID_O2S2_WR_LAMBDA_I,           /* 0x35 */
    OBD_PID_O2S3_WR_LAMBDA_I,           /* 0x36 */
    OBD_PID_O2S4_WR_LAMBDA_I,           /* 0x37 */
    OBD_PID_O2S5_WR_LAMBDA_I,           /* 0x38 */
    OBD_PID_O2S6_WR_LAMBDA_I,           /* 0x39 */
    OBD_PID_O2S7_WR_LAMBDA_I,           /* 0x3A */
    OBD_PID_O2S8_WR_LAMBDA_I,           /* 0x3B */
    OBD_PID_CAT_TEMP_BANK_1_S_1,        /* 0x3C */
    OBD_PID_CAT_TEMP_BANK_2_S_1,        /* 0x3D */
    OBD_PID_CAT_TEMP_BANK_1_S_2,        /* 0x3E */
    OBD_PID_CAT_TEMP_BANK_2_S_2,        /* 0x3F */
    OBD_PID_SUPPORT_3,                  /* 0x40 */
    OBD_PID_MONITOR_STATUS_THIS_DRIVE,  /* 0x41 */
    OBD_PID_CM_V,                       /* 0x42 */
    OBD_PID_ABS_LOAD,                   /* 0x43 */
    OBD_PID_FUEL_AIR_CMD_RATIO,         /* 0x44 */
    OBD_PID_REL_THROTTLE,               /* 0x45 */
    OBD_PID_AMBIENT_AIR_TEMP,           /* 0x46 */

    OBD_PID_CNT
};

#endif /* _OBD_PID_H_ */
