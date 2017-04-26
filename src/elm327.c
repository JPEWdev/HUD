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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elm327.h"
#include "led.h"
#include "obd_pid.h"
#include "pin.h"
#include "timer.h"
#include "uart.h"
#include "utl.h"

#define IS_WHITESPACE(_c) (((_c) == ' ') || ((_c) == '\t'))

#define UART (UART0)
#define INIT_BAUD (38400)
#define BAUD (500000)
#define ELM_CLOCK_FREQ (4000000)

#define write_data(_data, _sz ) UART_tx(UART, _data, _sz)
#define printf( ... ) UART_printf(UART1, __VA_ARGS__)
#define ENDL "\n\r"

#ifndef DEBUG
    #ifdef printf
        #undef printf
    #endif

    #define printf(...)
#endif

#define LINE_BUFFER_SZ  (50)

#define TIMEOUT (10000)

#define RTS_PIN PIN_C2
#define POWER_CTRL_PIN PIN_C1

#define proto_is_ISO_15765(_p)    \
    (((_p) == ELM327_PROTO_ISO_15765_4_CAN_11_500_KBAUD) ||    \
     ((_p) == ELM327_PROTO_ISO_15765_4_CAN_29_500_KBAUD) ||    \
     ((_p) == ELM327_PROTO_ISO_15765_4_CAN_11_250_KBAUD) ||    \
     ((_p) == ELM327_PROTO_ISO_15765_4_CAN_29_250_KBAUD))

static const char endl[] = ENDL;

static const struct {
    uint8_t data_bytes;
} pid_table[] = {
    /* OBD_PID_SUPPORT_1                    0x00 */ { 4 },
    /* OBD_PID_MONITOR_STATUS               0x01 */ { 4 },
    /* OBD_PID_FREEZE_DTC                   0x02 */ { 8 },
    /* OBD_PID_FUEL_SYS_STATUS              0x03 */ { 2 },
    /* OBD_PID_ENGN_LOAD                    0x04 */ { 1 },
    /* OBD_PID_ENGN_CLNT_TEMP               0x05 */ { 1 },
    /* OBD_PID_SHORT_FUEL_TRIM_1            0x06 */ { 1 },
    /* OBD_PID_LONG_FUEL_TRIM_1             0x07 */ { 1 },
    /* OBD_PID_SHORT_FUEL_TRIM_2            0x08 */ { 1 },
    /* OBD_PID_LONG_FUEL_TRIM_2             0x09 */ { 1 },
    /* OBD_PID_FUEL_PRES                    0x0A */ { 1 },
    /* OBD_PID_INTAKE_ABS_PRES              0x0B */ { 1 },
    /* OBD_PID_ENGN_RPM                     0x0C */ { 2 },
    /* OBD_PID_SPEED                        0x0D */ { 1 },
    /* OBD_PID_TIMING_ADV                   0x0E */ { 1 },
    /* OBD_PID_INTAKE_AIR_TEMP              0x0F */ { 1 },
    /* OBD_PID_MAF_RATE                     0x10 */ { 2 },
    /* OBD_PID_THROTTLE_POS                 0x11 */ { 1 },
    /* OBD_PID_CMD_SEC_AIR_STAT             0x12 */ { 1 },
    /* OBD_PID_O2_PRESENT_1                 0x13 */ { 1 },
    /* OBD_PID_O2_V_BANK_1_S_1              0x14 */ { 2 },
    /* OBD_PID_O2_V_BANK_1_S_2              0x15 */ { 2 },
    /* OBD_PID_O2_V_BANK_1_S_3              0x16 */ { 2 },
    /* OBD_PID_O2_V_BANK_1_S_4              0x17 */ { 2 },
    /* OBD_PID_O2_V_BANK_2_S_1              0x18 */ { 2 },
    /* OBD_PID_O2_V_BANK_2_S_2              0x19 */ { 2 },
    /* OBD_PID_O2_V_BANK_2_S_3              0x1A */ { 2 },
    /* OBD_PID_O2_V_BANK_2_S_4              0x1B */ { 2 },
    /* OBD_PID_OBD_STANDARDS                0x1C */ { 1 },
    /* OBD_PID_O2_PRESENT_2                 0x1D */ { 1 },
    /* OBD_PID_AUX_INPUT_STAT               0x1E */ { 1 },
    /* OBD_PID_RUN_TIME                     0x1F */ { 2 },
    /* OBD_PID_SUPPORT_2                    0x20 */ { 4 },
    /* OBD_PID_DIST_W_MIL,                  0x21 */ { 2 },
    /* OBD_PID_FUEL_RAIL_PRES,              0x22 */ { 2 },
    /* OBD_PID_FUEL_RAIL_PRES_DSL,          0x23 */ { 2 },
    /* OBD_PID_O2S1_WR_LAMBDA_V,            0x24 */ { 4 },
    /* OBD_PID_O2S2_WR_LAMBDA_V,            0x25 */ { 4 },
    /* OBD_PID_O2S3_WR_LAMBDA_V,            0x26 */ { 4 },
    /* OBD_PID_O2S4_WR_LAMBDA_V,            0x27 */ { 4 },
    /* OBD_PID_O2S5_WR_LAMBDA_V,            0x28 */ { 4 },
    /* OBD_PID_O2S6_WR_LAMBDA_V,            0x29 */ { 4 },
    /* OBD_PID_O2S7_WR_LAMBDA_V,            0x2A */ { 4 },
    /* OBD_PID_O2S8_WR_LAMBDA_V,            0x2B */ { 4 },
    /* OBD_PID_CMNDED_EGR,                  0x2C */ { 1 },
    /* OBD_PID_EGR_ERROR,                   0x2D */ { 1 },
    /* OBD_PID_CMND_EVAP_PURGE,             0x2E */ { 1 },
    /* OBD_PID_FUEL_LVL_INPUT,              0x2F */ { 1 },
    /* OBD_PID_NUM_WARM_UPS_SINCE_CLEAR,    0x30 */ { 1 },
    /* OBD_PID_DIST_SINCE_CLEAR,            0x31 */ { 2 },
    /* OBD_PID_EVAP_PRES,                   0x32 */ { 2 },
    /* OBD_PID_BARO_PRES,                   0x33 */ { 1 },
    /* OBD_PID_O2S1_WR_LAMBDA_I,            0x34 */ { 4 },
    /* OBD_PID_O2S2_WR_LAMBDA_I,            0x35 */ { 4 },
    /* OBD_PID_O2S3_WR_LAMBDA_I,            0x36 */ { 4 },
    /* OBD_PID_O2S4_WR_LAMBDA_I,            0x37 */ { 4 },
    /* OBD_PID_O2S5_WR_LAMBDA_I,            0x38 */ { 4 },
    /* OBD_PID_O2S6_WR_LAMBDA_I,            0x39 */ { 4 },
    /* OBD_PID_O2S7_WR_LAMBDA_I,            0x3A */ { 4 },
    /* OBD_PID_O2S8_WR_LAMBDA_I,            0x3B */ { 4 },
    /* OBD_PID_CAT_TEMP_BANK_1_S_1,         0x3C */ { 4 },
    /* OBD_PID_CAT_TEMP_BANK_2_S_1,         0x3D */ { 4 },
    /* OBD_PID_CAT_TEMP_BANK_1_S_2,         0x3E */ { 4 },
    /* OBD_PID_CAT_TEMP_BANK_2_S_2,         0x3F */ { 4 },
    /* OBD_PID_SUPPORT_3,                   0x40 */ { 4 },
    /* OBD_PID_MONITOR_STATUS_THIS_DRIVE,   0x41 */ { 4 },
    /* OBD_PID_CM_V,                        0x42 */ { 2 },
    /* OBD_PID_ABS_LOAD,                    0x43 */ { 2 },
    /* OBD_PID_FUEL_AIR_CMD_RATIO,          0x44 */ { 2 },
    /* OBD_PID_REL_THROTTLE,                0x45 */ { 1 },
    /* OBD_PID_AMBIENT_AIR_TEMP,            0x46 */ { 1 },
};

STATIC_ASSERT(cnt_of_array(pid_table) == OBD_PID_CNT);

static const char *const dtc_prefix[] = {
    /* 0    */  "P0",
    /* 1    */  "P1",
    /* 2    */  "P2",
    /* 3    */  "P3",
    /* 4    */  "C0",
    /* 5    */  "C1",
    /* 6    */  "C2",
    /* 7    */  "C3",
    /* 8    */  "B0",
    /* 9    */  "B1",
    /* A    */  "B2",
    /* B    */  "B3",
    /* C    */  "U0",
    /* D    */  "U1",
    /* E    */  "U2",
    /* F    */  "U3"
    };

static bool s_searching;
static bool s_cannot_connect;
static ELM327_proto_type s_cur_proto;
static char s_cur_proto_str[50];
static bool s_auto_proto;

static bool s_elm_ready;

static uint8_t last_pid;

static ELM327_data_clbk my_data_clbk;
static ELM327_no_data_clbk my_no_data_clbk;
static bool my_echo_enabled;

static void
reset_state(void)
{
    s_searching = false;
    s_cannot_connect = false;
    s_cur_proto = ELM327_PROTO_AUTO;
    s_cur_proto_str[0] = '\0';
    s_auto_proto = true;
    last_pid = 0;
    my_echo_enabled = true;
    s_elm_ready = false;
}

static uint8_t
read_data(uint8_t *buffer, uint8_t size)
{
    uint8_t cur = 0;

    /*
     * Read one byte at a time. We have to do this to skip the occasional
     * NUL byte the ELM can insert into the output stream (listed errata)
     */
    while (cur < size) {
        if (UART_rx_nb(UART, &buffer[cur], 1) == 0)
            break;

        /*
         * If this byte isn't a NUL, advance the buffer
         */
        if (buffer[cur])
            cur++;
    }
    return cur;
}

static char const *
get_data(void)
{
    static char s_rspns_buffer[LINE_BUFFER_SZ + 1];
    static char s_line_buffer[LINE_BUFFER_SZ + 1];
    static uint8_t s_line_buffer_cur = 0;
    static uint8_t s_line_buffer_sz = 0;

    bool process;
    uint8_t read_sz;

    process = false;

    read_sz = read_data((void *)&s_line_buffer[s_line_buffer_sz],
            LINE_BUFFER_SZ - s_line_buffer_sz);

    s_line_buffer_sz += read_sz;

    if (s_line_buffer_cur < s_line_buffer_sz) {
        switch (s_line_buffer[s_line_buffer_cur]) {
        case '\n':
        case '\r':
            if (s_line_buffer_cur + 1 < s_line_buffer_sz) {
                /*
                 * Copy data to response buffer and NULL terminate
                 */
                memcpy(s_rspns_buffer, s_line_buffer, s_line_buffer_cur);
                s_rspns_buffer[s_line_buffer_cur] = '\0';

                /*
                 * Move remaining data to the beginning of the buffer
                 */
                s_line_buffer_cur++;
                memmove(s_line_buffer, &s_line_buffer[s_line_buffer_cur],
                        s_line_buffer_sz - s_line_buffer_cur);
                s_line_buffer_sz -= s_line_buffer_cur;
                s_line_buffer_cur = 0;

                process = true;
                printf("got newline" ENDL);
            }
            break;

        case '>':
            s_line_buffer_sz--;
            memmove(&s_line_buffer[s_line_buffer_cur],
                    &s_line_buffer[s_line_buffer_cur + 1], s_line_buffer_sz);
            s_elm_ready = true;
            printf("ELM327 Ready" ENDL);
            break;

        default:
            s_line_buffer_cur++;
            break;
        }
    }

    return process ? s_rspns_buffer : NULL;
}

static void write_string(char const *str)
{
    write_data(str, strlen(str));
}

static bool
wait_for_string(char const *s, size_t len, uint32_t max_time)
{
    char const *data;
    uint32_t start_time = timer_get();

    while (true) {
        while ((data = get_data()) == NULL && timer_get() - start_time < max_time)
            ;

        if (data == NULL)
            return false;

        if (len > 0 && strncmp(data, s, len) == 0)
            return true;

        if (strcmp(data, s) == 0)
            return true;
    }
}

static uint8_t
parse_hex_string(char const *str, uint8_t *dest_buf, uint8_t dest_sz,
        char const **end )
{
    char c;
    uint8_t nibble;
    uint8_t val;
    uint8_t dest_pos;
    bool done;

    nibble = 1;
    dest_pos = 0;

    if (dest_sz == 0)
        return 0;

    memset(dest_buf, 0, dest_sz);

    c = *str;
    val = 0xFF;
    done = false;

    while (c && !done) {
        val = 0xFF;

        if (c >= '0' && c <= '9')
            val = c - '0';
        else if (c >= 'A' && c <= 'F')
            val = (c - 'A') + 10;
        else if (c >= 'a' && c <= 'f')
            val = (c - 'a') + 10;
        else if (!IS_WHITESPACE(c))
            break;

        if (val != 0xFF) {
            dest_buf[dest_pos] |= val << (nibble * 4);

            if (nibble == 0) {
                nibble = 1;
                dest_pos++;

                if (dest_pos >= dest_sz)
                    done = true;
            } else {
                nibble--;
            }
        }

        str++;
        c = *str;
    }

    if (val != 0xFF && dest_pos < dest_sz)
        dest_buf[dest_pos] |= val << (nibble * 4);

    if (end)
        *end = str;

    return dest_pos;
}

static void
wake_up(void)
{
    /*
     * Strobe the RTS pin. This will ensure that we get a prompt if the ELM is
     * busy doing something else, or wake it up from low power mode
     */
    pin_set_output(RTS_PIN, PIN_LOW);
    timer_sleep(100);
    pin_set_output(RTS_PIN, PIN_HIGH);
}

static void
wait_ready(void)
{
    if (!s_elm_ready) {
        printf("Waiting for ready\n\r");
        ELM327_process(true);
    }
}

static char const *
send_command(char const *cmd, bool wait)
{
    char const *result;

    result = NULL;

    /*
     * Wait until the ELM327 is ready
     */
    wait_ready();

    write_string(cmd);
    write_string(endl);
    s_elm_ready = false;

    if (my_echo_enabled) {
        while(get_data() == NULL)
            ;
    }

    /*
     * If a wait was requested, get the data
     */
    if (wait) {
        printf("Waiting for response to %s...\n\r", cmd);
        while ((result = get_data()) == NULL)
            ;
    }

    return result;
}

static void
get_proto(void)
{
    char const *buf;

    if (s_auto_proto) {
        printf("Getting protocol...\n\r");

        /* Send command requesting the current protocol number */
        if ((buf = send_command("at dpn", true)) != NULL) {
            // Skip the auto flag if present
            if ((tolower(buf[0]) == 'a') && isxdigit(buf[1])) {
                buf++;
                s_auto_proto = true;
            } else {
                s_auto_proto = false;
            }

            if (isdigit(*buf))
                s_cur_proto = *buf - '0';
            else
                s_cur_proto = (tolower(*buf) - 'a') + 10;
        }

        /* Send command requesting the current protocol string */
        if ((buf = send_command("at dp", true)) != NULL) {
            strncpy(s_cur_proto_str, buf, sizeof(s_cur_proto_str));
            s_cur_proto_str[sizeof(s_cur_proto_str) - 1] = '\0';
        }
    }
}

void
ELM327_connect(void)
{
    static const char id_string[] = {'E', 'L', 'M', '3', '2'};

    UART_init(UART, UART_TX | UART_RX, BAUD);

    pin_set_direction(RTS_PIN, PIN_OUTPUT);
    pin_set_direction(POWER_CTRL_PIN, PIN_INPUT);

    wake_up();
    timer_sleep(500);

    /*
     * See if the board will identify itself. If so, it is already at the
     * correct baud rate
     */
    write_string("at i" ENDL);

    if (!wait_for_string(id_string, sizeof(id_string), 1000)) {
        while (true) {
            UART_change_baud(UART, INIT_BAUD);
            UART_printf(UART, "at brd %02x" ENDL, ELM_CLOCK_FREQ / BAUD);

            if (wait_for_string("OK", 0, 1000)) {
                UART_change_baud(UART, BAUD);
                if (wait_for_string(id_string, sizeof(id_string), 1000)) {
                    /*
                     * Acknowledge new baud rate with a single carriage return
                     */
                    write_string("\r");
                    if (wait_for_string("OK", 0, 1000))
                        break;
                }
            }
        }
    }

    reset_state();

    my_data_clbk = NULL;
    my_no_data_clbk = NULL;
}

void
ELM327_init(void)
{
    wake_up();

    send_command("at ws", true);

    reset_state();

    /*
     * Disable echo
     */
    send_command("at e0", false);
    my_echo_enabled = false;
}

bool
ELM327_connected(void)
{
    return s_cannot_connect ? false : true;
}

bool
ELM327_searching(void)
{
    return s_searching;
}

/**
 * Processes input from the ELM 327 (optionally blocking) until it is ready to
 * accept another command
 */

void
ELM327_process(bool block)
{
    uint8_t header[2];
    uint8_t data_buf[8];
    uint8_t data_bytes;
    char const *end;
    char const *buf;

    do {
        if ((buf = get_data()) != NULL) {
            if (buf[0] != '\0') {
                printf("Got Line '%s'" ENDL, buf);

                if (parse_hex_string(buf, header, cnt_of_array(header), &end)
                        == cnt_of_array(header)) {
                    printf("Header = %02x %02x" ENDL, header[0], header[1]);

                    if (my_data_clbk) {
                        if (header[1] == last_pid) {
                            data_bytes = pid_table[header[1]].data_bytes;

                            printf("Data Bytes = %i" ENDL, data_bytes);

                            if (parse_hex_string(end, data_buf, data_bytes,
                                        NULL) == data_bytes) {
                                printf("Got data for %02x" ENDL, header[1]);
                                my_data_clbk(header[1], data_buf);
                                s_searching = false;
                                s_cannot_connect = false;
                            } else {
                                printf("Error Parsing data from line %s" ENDL,
                                        buf);
                            }
                        } else {
                            printf("Error got data for unrequested PID 0x%02X"
                                    ENDL, header[1]);
                        }
                    }
                } else if (strcmp(buf, "NO DATA") == 0) {
                    if (my_no_data_clbk)
                        my_no_data_clbk( last_pid );
                    s_searching = false;
                    s_cannot_connect = false;
                } else if (strcmp(buf, "SEARCHING...") == 0) {
                    printf("Searching" ENDL);
                    s_searching = true;
                } else if (strcmp(buf, "OK") == 0) {
                    /* Nothing to do */
                } else if (strcmp(buf, "?") == 0) {
                    /* Nothing to do */
                } else if (strcmp(buf, "LV RESET") == 0) {
                    //LED_strobe( 500 );
                } else if (strcmp(buf, "UNABLE TO CONNECT") == 0) {
                    s_cannot_connect = true;
                } else {
                    printf("Unknown Line Buffer '%s'" ENDL, buf);
                }
            }
        }
        //timer_process();
    } while (block && !s_elm_ready);
}

void
ELM327_set_echo(bool echo)
{
    if (echo) {
        send_command("at e1", true);
        my_echo_enabled = true;
    } else {
        send_command("at e0", true);
        my_echo_enabled = false;
    }
}

void
ELM327_set_linefeed(bool lf)
{
    if (lf)
        send_command("at l1", true);
    else
        send_command("al l0", true);
}

ELM327_proto_type
ELM327_get_proto(void)
{
    get_proto();
    return s_cur_proto;
}

char const *
ELM327_get_proto_str(void)
{
    get_proto();
    return s_cur_proto_str;
}

void
ELM327_set_proto(ELM327_proto_type p)
{
    char cmd[10];

    snprintf(cmd, cnt_of_array(cmd), "at sp %x", p);
    send_command(cmd, true);
}

uint8_t
ELM327_get_dtc(ELM327_dtc_type **ptr)
{
    uint8_t hdr;
    uint8_t dtc_data[2];
    uint16_t dtc;
    char const *end;
    char const *tmp_end;
    char const *buf;
    uint8_t cnt;
    uint8_t num_dtc;
    ELM327_proto_type proto;

    proto = ELM327_get_proto();

    cnt = 0;
    num_dtc = 0;
    *ptr = NULL;

    if ((buf = send_command("03", true)) != NULL) {
        if (parse_hex_string(buf, &hdr, sizeof(hdr), &end) == 1) {
            if (hdr == 0x43) {
                if (proto_is_ISO_15765(proto)) {
                    // In ISO 15765 protocols, the number of DTC is stored in
                    // the first byte of the payload
                    if (parse_hex_string(end, &num_dtc, sizeof(num_dtc), &end)
                            != sizeof(num_dtc))
                        num_dtc = 0;
                } else {
                    // Count the number of DTC's
                    tmp_end = end;

                    while (parse_hex_string(tmp_end, (uint8_t*)&dtc, sizeof(dtc),
                                &tmp_end) == sizeof(dtc))
                        num_dtc++;
                }

                if (num_dtc) {
                    // Allocate memory
                    (*ptr) = malloc(sizeof(ELM327_dtc_type) * num_dtc);

                    //Construct DTC's
                    while ((cnt < num_dtc) &&
                           (parse_hex_string(end, dtc_data, sizeof(dtc_data),
                                             &end) == sizeof(dtc))) {
                        // Construct numeric DTC from data
                        dtc = (((uint16_t)dtc_data[0]) << 8) | dtc_data[1];

                        //Construct DTC string
                        sprintf((*ptr)[cnt].code, "%s%03X",
                                dtc_prefix[dtc >> 12], (uint16_t)(dtc & 0xFFF));
                        cnt++;
                    }
                }
            }
        }
    }

    return cnt;
}

void
ELM327_clear_dtcs(void)
{
    char buffer[10];
    snprintf(buffer, cnt_of_array(buffer), "%02u", OBD_CLEAR_DTC);
    send_command(buffer, false);
}

float
ELM327_get_voltage(void)
{
    char const *buf;

    if ((buf = send_command("at rv", true)) != NULL)
        return strtod(buf, NULL);

    return 0.0f;
}

static struct {
    bool found;
    size_t *data_len;
    uint8_t *buffer;
} my_get_pid_data;

static void
get_pid_data_clbk(obd_pid_t8 pid, uint8_t const *data)
{
    my_get_pid_data.found = true;
    memcpy(my_get_pid_data.buffer, data, pid_table[pid].data_bytes);
    *my_get_pid_data.data_len = pid_table[pid].data_bytes;
}

static void
get_pid_no_data_clbk(obd_pid_t8 pid)
{
    my_get_pid_data.found = false;
}

static bool
get_pid_helper(obd_pid_t8 pid, void (*rqst)(obd_pid_t8), uint8_t
        buffer[OBD_PID_MAX_LEN], size_t *len)
{
    ELM327_data_clbk saved_data_clbk;
    ELM327_no_data_clbk saved_no_data_clbk;

    wait_ready();

    my_get_pid_data.found = false;
    my_get_pid_data.buffer = buffer;
    my_get_pid_data.data_len = len;

    saved_data_clbk = my_data_clbk;
    saved_no_data_clbk = my_no_data_clbk;

    my_data_clbk = get_pid_data_clbk;
    my_no_data_clbk = get_pid_no_data_clbk;

    rqst(pid);
    ELM327_process(true);

    my_data_clbk = saved_data_clbk;
    my_no_data_clbk = saved_no_data_clbk;

    return my_get_pid_data.found;
}

void
ELM327_rqst_crnt_pid(obd_pid_t8 pid)
{
    char buffer[10];

    snprintf(buffer, sizeof(buffer), "%02u%02x", OBD_SHOW_DATA, pid);
    last_pid = pid;
    send_command(buffer, false);
}

bool
ELM327_get_crnt_pid(obd_pid_t8 pid, uint8_t buffer[OBD_PID_MAX_LEN],
        size_t *len)
{
    return get_pid_helper(pid, ELM327_rqst_crnt_pid, buffer, len);
}

void
ELM327_rqst_freeze_pid(obd_pid_t8 pid)
{
    char buffer[10];

    snprintf(buffer, sizeof(buffer), "%02u%02x", OBD_FREEZE_DATA, pid);
    last_pid = pid;
    send_command(buffer, false);
}

bool
ELM327_get_freeze_pid(obd_pid_t8 pid, uint8_t buffer[OBD_PID_MAX_LEN],
        size_t *len)
{
    return get_pid_helper(pid, ELM327_rqst_freeze_pid, buffer, len);
}

void
ELM327_set_clbk(ELM327_data_clbk data_clbk,
        ELM327_no_data_clbk no_data_clbk)
{
    my_data_clbk = data_clbk;
    my_no_data_clbk = no_data_clbk;
}

bool
ELM327_is_ready(void)
{
    return s_elm_ready;
}

void
ELM327_low_power_mode(void)
{
    my_data_clbk = NULL;
    my_no_data_clbk = NULL;

    UART_tx_wait(UART);
    send_command("at lp", false);
    UART_tx_wait(UART);
}

