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
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "btn.h"
#include "diagnostics.h"
#include "elm327.h"
#include "hud_data.h"
#include "layout.h"
#include "led.h"
#include "menu.h"
#include "obd_data.h"
#include "timer.h"
#include "uart.h"
#include "utl.h"
#include "vfd.h"

#define ROW0_Y      (0)
#define ROW1_Y      (VFD_LINE_HGHT / 2)
#define MAX_PAGES   (4)

#define SEARCH_X    (VFD_WDTH - VFD_CHAR_WDTH)
#define SEARCH_Y    (ROW0_Y)

#define CONN_X      (VFD_WDTH - VFD_CHAR_WDTH)
#define CONN_Y      (ROW1_Y)

#define DISPLAY_UART UART1
#define IGNMON_PIN PIN_C3
#define POWER_CTL_PIN PIN_C1
#define EXT_SLEEP PIN_A1

#define MAX_IDLE_TIME (10000)

#ifndef NO_VFD
    #define display_ready() VFD_ready()
#else
    #define display_ready() ( UART_tx_level( DISPLAY_UART ) == 0 )
#endif

enum layout_idx {
    LAYOUT_2,
    LAYOUT_3L,
    LAYOUT_3R,
    LAYOUT_4,
    LAYOUT_CNT,
};

struct watch_state_type {
    bool updated;
    bool valid;
    char text_buffer[ HUD_DATA_LEN ];
};

struct data_page_type {
    enum layout_idx layout;
    hud_data_t8 data[DATA_CNT];
};

static struct data_page_type EEMEM watch_pages_eemem[MAX_PAGES] = {
    {
        LAYOUT_3L,
        {
        HUD_DATA_SPEED_MPH,
        HUD_DATA_RPM,
        HUD_DATA_INST_ECON,
        }
    },
    {
        LAYOUT_4,
        {
        HUD_DATA_AVG_ECON,
        HUD_DATA_FUEL_LVL,
        HUD_DATA_BARO_PRES_MMHG,
        HUD_DATA_AIR_TEMP_F,
        }
    },
};

static const struct screen_layout_type *const layouts[] = {
    &layout_2_LR,
    &layout_3L,
    &layout_3R,
    &layout_4
};

STATIC_ASSERT(cnt_of_array(layouts) == LAYOUT_CNT);

static uint8_t EEMEM num_pages_eemem = 2;

static uint8_t EEMEM brightness_eemem = VFD_BRIGHT_DEFAULT;

static struct data_page_type watch_pages[MAX_PAGES];

static const char splash_str[] = "Hello Joshua";
static const char invalid_data_str[] = "---";

static struct data_page_type const *my_cur_page = NULL;
static uint8_t my_cur_page_idx = 0;
static uint8_t my_num_pages;
static bool my_searching = false;
static bool my_connected = true;
static uint8_t my_last_updt = 0;
static struct watch_state_type my_state[DATA_CNT];

static void
show_dtcs(bool wait)
{
    ELM327_dtc_type *dtcs;
    uint8_t dtc_cnt;
    uint8_t i;

    dtc_cnt = ELM327_get_dtc(&dtcs);

    VFD_clear();
    VFD_char_width(VFD_CHAR_WDTH_PROP_1);

    if (dtc_cnt > 0) {
        for (i = 0; i < dtc_cnt; i++) {
            VFD_clear();
            VFD_set_cursor(0, 0);
            VFD_printf("DTC %u of %u", (int)(i + 1), (int)dtc_cnt);
            VFD_set_cursor(0, 1);
            VFD_printf("%s", dtcs[i].code);

            /* Wait 30 seconds for a button press */
            BTN_wait(wait ? 0 : 30000);
        }
    } else {
        VFD_set_cursor(0, 0);
        VFD_printf("No DTCs");
        BTN_wait(wait ? 0 : 1000);
    }

    free(dtcs);
}

static enum menu_id
clear_dtc_menu(enum menu_id id, void *param)
{
    enum menu_id m;

    VFD_soft_reset();
    m = menu_yes_no("Are you sure?");

    if (m == MENU_YES) {
        VFD_char_width(VFD_CHAR_WDTH_PROP_1);
        VFD_printf("Clearing DTCs");
        ELM327_clear_dtcs();
        timer_sleep(1000);
    }

    return (m == MENU_TIMEOUT) ? m : MENU_NONE;
}

static enum menu_id
view_dtc_menu(enum menu_id id, void *param)
{
    VFD_soft_reset();
    show_dtcs(true);
    return MENU_NONE;
}

static void
choose_data_items(struct data_page_type *dp)
{
    struct menu_type data_menu_items[DATA_CNT];
    struct menu_type hud_menu[HUD_DATA_CNT];
    enum menu_id m;
    int i;
    uint8_t data_idx;

    // Construct HUD menu
    for (i = 0; i < HUD_DATA_CNT; i++) {
        hud_menu[i].id = i;
        hud_menu[i].string = HUD_data_name(i);
        hud_menu[i].proc = NULL;
    }

    for (i = 0; i < DATA_CNT; i++) {
        if (dp->data[i] >= HUD_DATA_CNT)
            dp->data[i] = HUD_DATA_NONE;
    }

    while (true) {
        for (i = 0; i < layouts[dp->layout]->wndw_cnt; i++) {
            data_menu_items[i].id = i;
            data_menu_items[i].string = HUD_data_name(dp->data[i]);
            data_menu_items[i].proc = NULL;
        }

        m = menu_process(layouts[dp->layout], data_menu_items,
                layouts[dp->layout]->wndw_cnt, 0, &dp);

        if (m == MENU_BACK || m == MENU_TIMEOUT)
            break;

        if (m < layouts[dp->layout]->wndw_cnt) {
            data_idx = m;

            m = menu_process(&layout_4, hud_menu, cnt_of_array(hud_menu),
                    dp->data[data_idx], NULL);

            if (m == MENU_TIMEOUT)
                break;

            if ((int)m < HUD_DATA_CNT)
                dp->data[data_idx] = m;
        }
    }
}

static enum menu_id
page_menu(enum menu_id id, void *param)
{
    struct menu_type layout_menu[ LAYOUT_CNT ];
    enum menu_id m;
    uint8_t idx = id;
    struct data_page_type dp;
    int i;

    // Copy data page settings
    dp = watch_pages[idx];

    // Choose layout
    for (i = 0; i < LAYOUT_CNT; i++) {
        layout_menu[i].id = i;
        layout_menu[i].string = layouts[i]->name;
        layout_menu[i].proc = NULL;
    }

    m = menu_process(layouts[LAYOUT_4], layout_menu, cnt_of_array(layout_menu),
            dp.layout, NULL);

    if (m < cnt_of_array(layout_menu)) {
        dp.layout = m;

        choose_data_items(&dp);

        // Save new settings
        watch_pages[idx] = dp;
        eeprom_write_block(&dp, &watch_pages_eemem[idx], sizeof(dp));
    }

    return (m == MENU_TIMEOUT) ? m : MENU_NONE;
}

static enum menu_id
layout_configure_menu(enum menu_id id, void *param)
{
    static struct menu_type page_menu_items[MAX_PAGES + 1];
    static char page_names[MAX_PAGES][10];
    uint8_t i;
    enum menu_id m;

    for (i = 0; i < my_num_pages; i++) {
        page_menu_items[i].id = i;
        page_menu_items[i].string = page_names[i];
        sprintf(page_names[i], "Page %u", i + 1);
        page_menu_items[i].proc = page_menu;
    }
    page_menu_items[i].id = MENU_BACK;
    page_menu_items[i].string = "Done";
    page_menu_items[i].proc = NULL;

    m = menu_process(&layout_4, page_menu_items, my_num_pages + 1,
            my_cur_page_idx, NULL);

    return (m == MENU_TIMEOUT) ? m : MENU_NONE;
}

static enum menu_id
brightness_menu(enum menu_id id, void *param)
{
    static const struct menu_type menu[] = {
    {   VFD_BRIGHT_12_5,    "12.5%",    NULL    },
    {   VFD_BRIGHT_25,      "25%",      NULL    },
    {   VFD_BRIGHT_37_5,    "37.5%",    NULL    },
    {   VFD_BRIGHT_50,      "50%",      NULL    },
    {   VFD_BRIGHT_62_5,    "62.5%",    NULL    },
    {   VFD_BRIGHT_75,      "75%",      NULL    },
    {   VFD_BRIGHT_87_5,    "87.5%",    NULL    },
    {   VFD_BRIGHT_100,     "100%",     NULL    },
    };

    enum menu_id m;
    uint8_t i;
    VFD_bright_lvl_t8 lvl;

    for (i = 0; i < cnt_of_array(menu); i++) {
        if (menu[i].id == VFD_brightness_get())
            break;
    }

    m = menu_process(&layout_4, menu, cnt_of_array(menu), i, NULL);

    if (m < MENU_RESERVED) {
        lvl = (VFD_bright_lvl_t8)m;

        VFD_brightness_set(lvl);
        eeprom_update_byte(&brightness_eemem, lvl);
    }

    return (m == MENU_TIMEOUT) ? m : MENU_NONE;
}

static enum menu_id
num_pages_menu(enum menu_id id, void *param)
{
    static struct menu_type menu[MAX_PAGES];
    static char menu_names[MAX_PAGES][10];

    uint8_t i;
    enum menu_id m;

    for (i = 0; i < MAX_PAGES; i++) {
        menu[i].id = i;
        menu[i].string = menu_names[i];
        menu[i].proc = NULL;

        sprintf(menu_names[i], "%u Pages", i + 1);
    }

    m = menu_process(&layout_4, menu, cnt_of_array(menu), my_num_pages - 1, NULL);

    if (m < MAX_PAGES) {
        my_num_pages = m + 1;

        eeprom_update_byte( &num_pages_eemem, my_num_pages );

        if (my_cur_page_idx >= my_num_pages)
            my_cur_page_idx = 0;
    }

    return (m == MENU_TIMEOUT) ? m : MENU_NONE;
}

static enum menu_id
fuel_econ_menu(enum menu_id id, void *param)
{
    static const struct menu_type options[] = {
        {   false,  "Only when shown",  NULL    },
        {   true,   "Always",           NULL    },
    };

    enum menu_id m;

    m = menu_process(&layout_2_TB, options, cnt_of_array(options),
            HUD_get_fuel_econ_always_on(), NULL);

    if (m == true || m == false) {
        HUD_set_fuel_econ_always_on(!!m);
    }

    return (m == MENU_TIMEOUT) ? m : MENU_NONE;
}

static enum menu_id
settings_menu(enum menu_id id, void *param)
{
    static const struct menu_type menu[] = {
        {   MENU_NONE,  "Fuel Econ",    fuel_econ_menu      },
        {   MENU_BACK,  "Back",         NULL                },
    };

    enum menu_id m;

    m = menu_process(&layout_4, menu, cnt_of_array(menu), 0, NULL);

    return (m == MENU_TIMEOUT) ? m : MENU_NONE;
}

static const struct menu_type main_menu[] = {
    {   MENU_NONE,  "Num Pages",    num_pages_menu          },
    {   MENU_NONE,  "Page Layout",  layout_configure_menu   },
    {   MENU_NONE,  "Settings",     settings_menu           },
    {   MENU_NONE,  "Brightness",   brightness_menu         },
    {   MENU_NONE,  "View DTCs",    view_dtc_menu           },
    {   MENU_NONE,  "Clear DTCs",   clear_dtc_menu          },
    {   MENU_NONE,  "Diagnostics",  diagnostics_menu        },

    {   MENU_BACK,  "Back",         NULL                    },
};

static void
dec_idx(void)
{
    if (my_cur_page_idx == 0)
        my_cur_page_idx = my_num_pages - 1;
    else
        my_cur_page_idx--;
}

static void
inc_idx(void)
{
    my_cur_page_idx = (my_cur_page_idx + 1) % my_num_pages;
}

static void
destroy_layout(void)
{
    uint8_t i;

    /*
     * Remove existing data items
     */
    if (my_cur_page) {
        for (i = 0; i < layouts[my_cur_page->layout]->wndw_cnt; i++) {
            HUD_data_remove(my_cur_page->data[i]);
            VFD_delete_window(layouts[my_cur_page->layout]->windows[i].window);
        }

        my_cur_page = NULL;
    }
}

static void
create_layout(struct data_page_type const *page)
{
    uint8_t i;

    if (my_cur_page != page) {
        my_cur_page = page;

        for (i = 0; i < layouts[my_cur_page->layout]->wndw_cnt; i++) {
            /*
             * Register for HUD Data
             */
            my_state[i].updated = true;
            my_state[i].valid = true;
            strcpy(my_state[i].text_buffer, invalid_data_str);
            HUD_data_add(my_cur_page->data[i]);
        }

        create_layout_windows(layouts[my_cur_page->layout]);
    }
}

static void
change_layout(void)
{
    struct data_page_type const *page = &watch_pages[my_cur_page_idx];
    uint8_t btns;

    do {
        destroy_layout();
        create_layout(page);

        VFD_win_select(VFD_WIN_BASE);
        VFD_clear();
        VFD_char_width(VFD_CHAR_WDTH_PROP_1);
        VFD_font_size(2, 2);
        VFD_printf("Page %u", my_cur_page_idx + 1);
        VFD_char_width(VFD_CHAR_WDTH_DEFAULT);

        btns = BTN_wait(1000);

        if (btns & BTN_L_PRESS)
            dec_idx();
        else if (btns & BTN_R_PRESS)
            inc_idx();
    } while (btns != 0);
}

static void
low_power_mode(void)
{
    struct pin_change_handler ignmon_int_handler;
    struct pin_change_handler power_ctl_int_handler;
    uint8_t btn;

    VFD_soft_reset();
    VFD_char_width(VFD_CHAR_WDTH_PROP_1);
    VFD_printf("Powering Down...");

    ELM327_low_power_mode();

    btn = BTN_wait(3000);

    VFD_power_down();

    if (btn)
        return;

    LED_off(LED_STATUS);
    LED_off(LED_POWER);
    pin_set_output(EXT_SLEEP, PIN_LOW);

    /*
     * Register a pin change interrupt for the ignition and power control pins.
     * We don't need the handler to actually do anything, just wake us up from
     * sleep mode
     */
    ignmon_int_handler.pin = IGNMON_PIN;
    ignmon_int_handler.callback = NULL;
    ignmon_int_handler.param = NULL;
    pin_enable_interrupt(&ignmon_int_handler);

    power_ctl_int_handler.pin = POWER_CTL_PIN;
    power_ctl_int_handler.callback = NULL;
    power_ctl_int_handler.param = NULL;
    pin_enable_interrupt(&power_ctl_int_handler);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    /*
     * Disable interrupts to ensure atomic checking of pin
     */
    cli();
    while(true) {
        LED_on(LED_STATUS);
        if (!pin_read(IGNMON_PIN) || !pin_read(POWER_CTL_PIN) || BTN_check())
            break;
        LED_off(LED_STATUS);

        sleep_enable();

        /*
         * The instruction immediately after SEI is guaranteed to before any
         * interrupts are handled, ensuring the device is asleep before the
         * interrupt that could wake it up would occur
         */
        sei();
        sleep_cpu();

        sleep_disable();
        cli();
    }
    sei();

    pin_disable_interrupt(&ignmon_int_handler);
    pin_disable_interrupt(&power_ctl_int_handler);

    pin_set_output(EXT_SLEEP, PIN_HIGH);

    LED_on(LED_POWER);
}

static void
process_display(void)
{
    uint8_t i;
    uint8_t cur = 0;

    if (my_searching != ELM327_searching()) {
        VFD_win_select(VFD_WIN_BASE);
        my_searching = ELM327_searching();
        VFD_font_size(1, 1);
        VFD_set_cursor(SEARCH_X, SEARCH_Y);
        VFD_write_string(my_searching ? "S" : " ");
    } else if (my_connected != ELM327_connected()) {
#ifndef NO_VFD
        VFD_win_select(VFD_WIN_BASE);
        my_connected = ELM327_connected();
        VFD_font_size(1, 1);
        VFD_set_cursor(CONN_X, CONN_Y);
        VFD_write_string(my_connected ? " " : "X");
#else
        UART_printf(DISPLAY_UART, "Not Connected\n\r");
#endif
    } else {
        for (i = 1; i < layouts[my_cur_page->layout]->wndw_cnt; i++) {
            cur = (my_last_updt + i) % DATA_CNT;
            if (my_state[cur].updated) {
                my_state[cur].updated = false;
#ifndef NO_VFD
                VFD_win_select(layouts[my_cur_page->layout]->windows[cur].window);
                VFD_clear();
                VFD_set_cursor(0, 0);
                /*--------------------------------------------------------
                    * VFD_set_cursor( watch_data[cur].x, watch_data[cur].y );
                    *------------------------------------------------------*/
                VFD_font_size(layouts[my_cur_page->layout]->windows[cur].font_sz,
                        layouts[my_cur_page->layout]->windows[cur].font_sz);
                VFD_write_string(my_state[ cur ].text_buffer);
#endif
                break;
            }
        }
#ifdef NO_VFD
        if (i != layouts[my_cur_page->layout]->wndw_cnt) {
            // The display was updated. Draw it again
            for (i = 0; i < layouts[my_cur_page->layout]->wndw_cnt; i++)
                UART_printf(DISPLAY_UART, "%u: %s\n\r", i, my_state[i].text_buffer);
        }
#endif

        my_last_updt = cur;
    }
}

static void
process_hud_data(void)
{
    uint8_t i;
    char temp_buffer[HUD_DATA_LEN];

    /*
     * Check if there are any changes in the HUD data module. If any data has
     * changed, update it and mark it as changed
     */
    for (i = 0; i < layouts[my_cur_page->layout]->wndw_cnt; i++) {
        if (HUD_data_updated(my_cur_page->data[i])
                && HUD_data_get(my_cur_page->data[i], temp_buffer)) {

            if (strcmp(temp_buffer, my_state[i].text_buffer) != 0) {
                my_state[i].updated = true;
                strcpy(my_state[i].text_buffer, temp_buffer);
            }
        } else if (!HUD_data_valid(my_cur_page->data[i]) && my_state[i].valid) {
            strcpy(my_state[i].text_buffer, invalid_data_str);
            my_state[i].updated = true;
            my_state[i].valid = false;
        }
    }
}

static bool
process_buttons(void)
{
    uint8_t btns = BTN_process();

    if (btns & BTN_L_PRESS) {
        dec_idx();
        change_layout();
        return true;
    }

    if (btns & BTN_R_PRESS) {
        inc_idx();
        change_layout();
        return true;
    }

    if (btns & BTN_R_HOLD) {
        destroy_layout();

        // Process menu
        menu_process(&layout_4, main_menu, cnt_of_array(main_menu), 0, NULL);

        // Re-create layout
        VFD_soft_reset();
        change_layout();
        return true;
    }

    if (btns & BTN_L_HOLD) {
        destroy_layout();

        choose_data_items(&watch_pages[my_cur_page_idx]);

        eeprom_write_block(&watch_pages[my_cur_page_idx],
                &watch_pages_eemem[my_cur_page_idx],
                sizeof(watch_pages[my_cur_page_idx]));

        // Re-create layout
        VFD_soft_reset();
        change_layout();
        return true;
    }

    return false;
}

static void
main_loop(void)
{
    uint32_t last_btn_time;

    VFD_connect();
    ELM327_connect();

    while(true) {
        #ifndef NO_VFD
            VFD_init();
            VFD_brightness_set(eeprom_read_byte(&brightness_eemem));
            VFD_char_width(VFD_CHAR_WDTH_PROP_1);

            VFD_write_string(splash_str);
            timer_sleep(5000);
        #else
            UART_init(DISPLAY_UART, UART_TX | UART_RX, 38400);

            UART_printf(DISPLAY_UART, "Hello... ");
            timer_sleep(5000);
            UART_printf(DISPLAY_UART, "ready\n\r");
        #endif

        LED_strobe(LED_STATUS, 1000);

        ELM327_init();
        ELM327_set_linefeed(false);

        LED_strobe(LED_STATUS, 10);

        #ifndef NO_VFD
            VFD_clear();
            VFD_set_cursor(0, 0);
            VFD_printf("%s (%X)", ELM327_get_proto_str(), (unsigned)ELM327_get_proto());
            timer_sleep(1000);
        #else
            UART_printf(DISPLAY_UART, "Protocol %u\n\r", proto);
        #endif

        show_dtcs(false);

        HUD_data_init();

        my_connected = true;
        my_searching = false;
        my_last_updt = 0;

        LED_strobe(LED_STATUS, 500);

        change_layout();

        last_btn_time = timer_get();

        while(true) {
            if (timer_get() - last_btn_time > MAX_IDLE_TIME
                    && pin_read(IGNMON_PIN)) {
                /*
                 * Debounce
                 */
                timer_sleep(1000);
                if (pin_read(IGNMON_PIN))
                    break;
            }

            timer_process();
            HUD_process();

            if (process_buttons())
                last_btn_time = timer_get();

            process_hud_data();

            if (display_ready())
                process_display();
        }

    /*
     * Enter low power mode
     */
    destroy_layout();
    low_power_mode();
    }
}

void
bypass_loop(void)
{
    uint8_t out;
    uint8_t in;
    while (true) {
        out = 0;
        in = PIND;
        if (in & _BV(2))
            out |= _BV(1);
        if (in & _BV(0))
            out |= _BV(3);
        PORTD = out;
    }
}

int
main(int argc, char **argv)
{
    uint8_t btns;

    timer_init();
    LED_init();
    BTN_init();

    pin_set_direction(IGNMON_PIN, PIN_INPUT);
    //pin_set_pullup(IGNMON_PIN, true);

    pin_set_direction(POWER_CTL_PIN, PIN_INPUT);

    pin_set_direction(EXT_SLEEP, PIN_OUTPUT);
    pin_set_output(EXT_SLEEP, PIN_HIGH);

    LED_on(LED_POWER);
    LED_strobe(LED_STATUS, 250);

    eeprom_read_block(watch_pages, watch_pages_eemem, sizeof(watch_pages));
    my_num_pages = eeprom_read_byte(&num_pages_eemem);

    sei();
    /*
     * Wait for external devices to settle down
     */
    timer_sleep( 1000 );

    btns = BTN_process();

    if (btns & BTN_L_HOLD) {
        LED_on(LED_STATUS);
        pin_set_direction(PIN_D1, PIN_OUTPUT);
        pin_set_direction(PIN_D0, PIN_INPUT);

        pin_set_direction(PIN_D3, PIN_OUTPUT);
        pin_set_direction(PIN_D2, PIN_INPUT);

        /*
         * RTS
         */
        pin_set_direction(PIN_C2, PIN_OUTPUT);
        pin_set_output(PIN_C2, PIN_HIGH);

        /*
         * Display Power
         */
        pin_set_direction(PIN_A7, PIN_OUTPUT);
        pin_set_output(PIN_A7, PIN_HIGH);

        bypass_loop();
    } else {
        main_loop();
    }

    return 0;
}


