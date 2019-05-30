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
    OBD_PID_SUPPORT_1,                  /* 0x00, 4 bytes */
    OBD_PID_MONITOR_STATUS,             /* 0x01, 4 bytes */
    OBD_PID_FREEZE_DTC,                 /* 0x02, 8 bytes */
    OBD_PID_FUEL_SYS_STATUS,            /* 0x03, 2 bytes */
    OBD_PID_ENGN_LOAD,                  /* 0x04, 1 bytes */
    OBD_PID_ENGN_CLNT_TEMP,             /* 0x05, 1 bytes */
    OBD_PID_SHORT_FUEL_TRIM_1,          /* 0x06, 1 bytes */
    OBD_PID_LONG_FUEL_TRIM_1,           /* 0x07, 1 bytes */
    OBD_PID_SHORT_FUEL_TRIM_2,          /* 0x08, 1 bytes */
    OBD_PID_LONG_FUEL_TRIM_2,           /* 0x09, 1 bytes */
    OBD_PID_FUEL_PRES,                  /* 0x0A, 1 bytes */
    OBD_PID_INTAKE_ABS_PRES,            /* 0x0B, 1 bytes */
    OBD_PID_ENGN_RPM,                   /* 0x0C, 2 bytes */
    OBD_PID_SPEED,                      /* 0x0D, 1 bytes */
    OBD_PID_TIMING_ADV,                 /* 0x0E, 1 bytes */
    OBD_PID_INTAKE_AIR_TEMP,            /* 0x0F, 1 bytes */
    OBD_PID_MAF_RATE,                   /* 0x10, 2 bytes */
    OBD_PID_THROTTLE_POS,               /* 0x11, 1 bytes */
    OBD_PID_CMD_SEC_AIR_STAT,           /* 0x12, 1 bytes */
    OBD_PID_O2_PRESENT_1,               /* 0x13, 1 bytes */
    OBD_PID_O2_V_BANK_1_S_1,            /* 0x14, 2 bytes */
    OBD_PID_O2_V_BANK_1_S_2,            /* 0x15, 2 bytes */
    OBD_PID_O2_V_BANK_1_S_3,            /* 0x16, 2 bytes */
    OBD_PID_O2_V_BANK_1_S_4,            /* 0x17, 2 bytes */
    OBD_PID_O2_V_BANK_2_S_1,            /* 0x18, 2 bytes */
    OBD_PID_O2_V_BANK_2_S_2,            /* 0x19, 2 bytes */
    OBD_PID_O2_V_BANK_2_S_3,            /* 0x1A, 2 bytes */
    OBD_PID_O2_V_BANK_2_S_4,            /* 0x1B, 2 bytes */
    OBD_PID_OBD_STANDARDS,              /* 0x1C, 1 bytes */
    OBD_PID_O2_PRESENT_2,               /* 0x1D, 1 bytes */
    OBD_PID_AUX_INPUT_STAT,             /* 0x1E, 1 bytes */
    OBD_PID_RUN_TIME,                   /* 0x1F, 2 bytes */
    OBD_PID_SUPPORT_2,                  /* 0x20, 4 bytes */
    OBD_PID_DIST_W_MIL,                 /* 0x21, 2 bytes */
    OBD_PID_FUEL_RAIL_PRES,             /* 0x22, 2 bytes */
    OBD_PID_FUEL_RAIL_PRES_DSL,         /* 0x23, 2 bytes */
    OBD_PID_O2S1_WR_LAMBDA_V,           /* 0x24, 4 bytes */
    OBD_PID_O2S2_WR_LAMBDA_V,           /* 0x25, 4 bytes */
    OBD_PID_O2S3_WR_LAMBDA_V,           /* 0x26, 4 bytes */
    OBD_PID_O2S4_WR_LAMBDA_V,           /* 0x27, 4 bytes */
    OBD_PID_O2S5_WR_LAMBDA_V,           /* 0x28, 4 bytes */
    OBD_PID_O2S6_WR_LAMBDA_V,           /* 0x29, 4 bytes */
    OBD_PID_O2S7_WR_LAMBDA_V,           /* 0x2A, 4 bytes */
    OBD_PID_O2S8_WR_LAMBDA_V,           /* 0x2B, 4 bytes */
    OBD_PID_CMNDED_EGR,                 /* 0x2C, 1 bytes */
    OBD_PID_EGR_ERROR,                  /* 0x2D, 1 bytes */
    OBD_PID_CMND_EVAP_PURGE,            /* 0x2E, 1 bytes */
    OBD_PID_FUEL_LVL_INPUT,             /* 0x2F, 1 bytes */
    OBD_PID_NUM_WARM_UPS_SINCE_CLEAR,   /* 0x30, 1 bytes */
    OBD_PID_DIST_SINCE_CLEAR,           /* 0x31, 2 bytes */
    OBD_PID_EVAP_PRES,                  /* 0x32, 2 bytes */
    OBD_PID_BARO_PRES,                  /* 0x33, 1 bytes */
    OBD_PID_O2S1_WR_LAMBDA_I,           /* 0x34, 4 bytes */
    OBD_PID_O2S2_WR_LAMBDA_I,           /* 0x35, 4 bytes */
    OBD_PID_O2S3_WR_LAMBDA_I,           /* 0x36, 4 bytes */
    OBD_PID_O2S4_WR_LAMBDA_I,           /* 0x37, 4 bytes */
    OBD_PID_O2S5_WR_LAMBDA_I,           /* 0x38, 4 bytes */
    OBD_PID_O2S6_WR_LAMBDA_I,           /* 0x39, 4 bytes */
    OBD_PID_O2S7_WR_LAMBDA_I,           /* 0x3A, 4 bytes */
    OBD_PID_O2S8_WR_LAMBDA_I,           /* 0x3B, 4 bytes */
    OBD_PID_CAT_TEMP_BANK_1_S_1,        /* 0x3C, 4 bytes */
    OBD_PID_CAT_TEMP_BANK_2_S_1,        /* 0x3D, 4 bytes */
    OBD_PID_CAT_TEMP_BANK_1_S_2,        /* 0x3E, 4 bytes */
    OBD_PID_CAT_TEMP_BANK_2_S_2,        /* 0x3F, 4 bytes */
    OBD_PID_SUPPORT_3,                  /* 0x40, 4 bytes */
    OBD_PID_MONITOR_STATUS_THIS_DRIVE,  /* 0x41, 4 bytes */
    OBD_PID_CM_V,                       /* 0x42, 2 bytes */
    OBD_PID_ABS_LOAD,                   /* 0x43, 2 bytes */
    OBD_PID_FUEL_AIR_CMD_RATIO,         /* 0x44, 2 bytes */
    OBD_PID_REL_THROTTLE,               /* 0x45, 1 bytes */
    OBD_PID_AMBIENT_AIR_TEMP,           /* 0x46, 1 bytes */
    OBD_PID_ABSOLUTE_THROTTLE_POSITION_B,   /* 0x47, 1 bytes */
    OBD_PID_ABSOLUTE_THROTTLE_POSITION_C,   /* 0x48, 1 bytes */
    OBD_PID_ACCELERATOR_PEDAL_POSITION_D,   /* 0x49, 1 bytes */
    OBD_PID_ACCELERATOR_PEDAL_POSITION_E,   /* 0x4A, 1 bytes */
    OBD_PID_ACCELERATOR_PEDAL_POSITION_F,   /* 0x4B, 1 bytes */
    OBD_PID_COMMANDED_THROTTLE_ACTUATOR,    /* 0x4C, 1 bytes */
    OBD_PID_TIME_RUN_WITH_MIL,              /* 0x4D, 2 bytes */
    OBD_PID_TIME_SINCE_DTC_CLEAR,           /* 0x4E, 2 bytes */
    OBD_PID_MAX_SENSOR_VALUES_1,            /* 0x4F, 4 bytes */
    OBD_PID_MAX_SENSOR_VALUES_2,            /* 0x50, 4 bytes */
    OBD_PID_FUEL_TYPE,                      /* 0x51, 1 bytes */
    OBD_PID_ETHANOL_PERCENT,                /* 0x52, 1 bytes */
    OBD_PID_ABS_EVAP_VAPOR_PRESSURE,        /* 0x53, 2 bytes */
    OBD_PID_EVAP_VAPOR_PRESSURE,            /* 0x54, 2 bytes */
    OBD_PID_O2_SHORT_TRIM_BANK_1_3,         /* 0x55, 2 bytes */
    OBD_PID_O2_LONG_TRIM_BANK_1_3,          /* 0x56, 2 bytes */
    OBD_PID_O2_SHORT_TRIM_BANK_2_4,         /* 0x57, 2 bytes */
    OBD_PID_O2_LONG_TRIM_BANK_2_4,          /* 0x58, 2 bytes */
    OBD_PID_FUEL_RAIL_PRESSURE,             /* 0x59, 2 bytes */
    OBD_PID_REL_ACCELERATOR_PEDAL_POS,      /* 0x5A, 1 bytes */
    OBD_PID_HYBRID_BATTER_REMAINING,        /* 0x5B, 1 bytes */
    OBD_PID_ENGN_OIL_TEMP,                  /* 0x5C, 1 bytes */

    OBD_PID_CNT
};

/*
 * The maximum number of data bytes in a given PID
 */
#define OBD_PID_MAX_LEN (8)

#endif /* _OBD_PID_H_ */
