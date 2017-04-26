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
#ifndef _ELM327_H_
#define _ELM327_H_

#include <stdbool.h>

#include "obd_pid.h"

typedef void (*ELM327_data_clbk)(obd_pid_t8 pid, uint8_t const *data);
typedef void (*ELM327_no_data_clbk)(obd_pid_t8 pid);

typedef struct {
    char code[ 6 ];
} ELM327_dtc_type;

typedef uint8_t ELM327_proto_type; enum {
    /* 0 */ ELM327_PROTO_AUTO,
    /* 1 */ ELM327_PROTO_SAE_J1850_PWM,
    /* 2 */ ELM327_PROTO_SAE_J1850_VPW,
    /* 3 */ ELM327_PROTO_ISO_9141_2,
    /* 4 */ ELM327_PROTO_ISO_14230_4_KWP_SLOW,
    /* 5 */ ELM327_PROTO_ISO_14230_4_KWP_FAST,
    /* 6 */ ELM327_PROTO_ISO_15765_4_CAN_11_500_KBAUD,
    /* 7 */ ELM327_PROTO_ISO_15765_4_CAN_29_500_KBAUD,
    /* 8 */ ELM327_PROTO_ISO_15765_4_CAN_11_250_KBAUD,
    /* 9 */ ELM327_PROTO_ISO_15765_4_CAN_29_250_KBAUD,
    /* A */ ELM327_PROTO_SAE_J1939_CAN,
    /* B */ ELM327_PROTO_USER1_CAN,
    /* C */ ELM327_PROTO_USER2_CAN
};

bool
ELM327_connected(void);

bool
ELM327_searching(void);

void
ELM327_connect(void);

void
ELM327_init(void);

void
ELM327_process(bool block);

void
ELM327_set_echo(bool echo);

void
ELM327_set_linefeed(bool lf);

ELM327_proto_type
ELM327_get_proto(void);

char const *
ELM327_get_proto_str(void);

void
ELM327_set_proto(ELM327_proto_type p);

uint8_t
ELM327_get_dtc(ELM327_dtc_type **ptr);

void
ELM327_clear_dtcs(void);

float
ELM327_get_voltage(void);

void
ELM327_rqst_crnt_pid(obd_pid_t8 pid);

bool
ELM327_get_crnt_pid(obd_pid_t8 pid, uint8_t buffer[OBD_PID_MAX_LEN],
        size_t *len);

void
ELM327_rqst_freeze_pid(obd_pid_t8 pid);

bool
ELM327_get_freeze_pid(obd_pid_t8 pid, uint8_t buffer[OBD_PID_MAX_LEN],
        size_t *len);

void
ELM327_set_clbk(ELM327_data_clbk data_clbk, ELM327_no_data_clbk no_data_clbk);

bool
ELM327_is_ready(void);

void
ELM327_low_power_mode(void);

#endif /* _ELM327_H_ */
