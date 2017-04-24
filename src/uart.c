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
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <util/atomic.h>

#include "led.h"
#include "pin.h"
#include "queue.h"
#include "uart.h"
#include "utl.h"

#define TX_BUFFER_SZ (32)
#define RX_BUFFER_SZ (128)

#define RX_BUFFER_FULL  ((RX_BUFFER_SZ * 3) / 4)

#ifndef UDORD0
#   define UDORD0 UCSZ01
#endif

#ifndef UCPHA0
#   define UCPHA0 UCSZ00
#endif

#ifndef UCPOL
#   define UCPOL 0
#endif

struct uart_reg_def_type {
    volatile uint8_t ucsra;
    volatile uint8_t ucsrb;
    volatile uint8_t ucsrc;
    uint8_t          reserved_1;
    volatile uint8_t ubrrl;
    volatile uint8_t ubrrh;
    volatile uint8_t udr;
} __attribute__((__packed__));


struct uart {
    uart_dev_t8 dev;
    struct uart_reg_def_type *reg;
    enum pin xck;
    uint8_t opts;
    volatile uint8_t tx_pending;
    volatile uint8_t tx_done;

    struct queue tx_queue;
    uint8_t tx_buffer[TX_BUFFER_SZ];

    uint8_t rx_cnt;
    uint8_t rx_dropped;
    struct queue rx_queue;
    uint8_t rx_buffer[RX_BUFFER_SZ];
    uint8_t frame_error;
    uint8_t overrun;
    struct pin_change_handler cts_pin;
    bool cts_invert;
};

static struct uart uart_data[UART_CNT];

static struct {
    struct uart_reg_def_type *reg;
    enum pin xck;
} uart_def[UART_CNT] = {
    { (struct uart_reg_def_type *)&UCSR0A, PIN_B0 },
    { (struct uart_reg_def_type *)&UCSR1A, PIN_D4 },
};

struct uart *
get_uart(uart_dev_t8 dev)
{
    return &uart_data[dev];
}

static bool
get_tx_ready(struct uart const *uart)
{
    bool tx_ready = true;

    if (uart->cts_pin.pin != PIN_INVALID) {
        tx_ready = pin_read(uart->cts_pin.pin);

        if (uart->cts_invert)
            tx_ready = !tx_ready;
    }

    return tx_ready && !queue_is_empty(&uart->tx_queue);
}

void
UART_printf(uart_dev_t8 dev, char const *fmt, ...)
{
    char buffer[100];
    va_list args;

    va_start(args, fmt);

    vsnprintf(buffer, sizeof(buffer), fmt, args);

    va_end(args);

    UART_tx(dev, buffer, strlen(buffer));
}

void
UART_tx(uart_dev_t8 dev, void const *data, uint8_t len)
{
    uint8_t cur_pos = 0;
    struct uart *uart = get_uart(dev);

    while (cur_pos < len) {
        while (queue_is_full(&uart->tx_queue))
            ;

        cur_pos += UART_tx_nb(dev, &((uint8_t const *)data)[cur_pos], len - cur_pos);
    }
}

uint8_t
UART_tx_nb(uart_dev_t8 dev, void const *data, uint8_t len)
{
    uint8_t copy_len;
    struct uart *uart = get_uart(dev);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        copy_len = queue_push(&uart->tx_queue, data, len);

        if (!queue_is_empty(&uart->tx_queue))
            uart->reg->ucsrb |= _BV(UDRIE0);
    }

    return copy_len;
}

void
UART_tx_wait(uart_dev_t8 dev)
{
    struct uart *uart = get_uart(dev);
    while (!queue_is_empty(&uart->tx_queue) || !(uart->reg->ucsra & _BV(TXC0)))
        ;
}

bool
UART_tx_empty(uart_dev_t8 dev) {
    struct uart *uart = get_uart(dev);
    return queue_is_empty(&uart->tx_queue);
}


void
UART_rx(uart_dev_t8 dev, void *data, uint8_t len)
{
    uint8_t cur_pos = 0;
    struct uart *uart = get_uart(dev);

    while (cur_pos < len) {
        while (queue_is_empty(&uart->rx_queue))
            ;

        cur_pos += UART_rx_nb(dev, &((uint8_t *)data)[cur_pos], len - cur_pos);
    }
}

uint8_t
UART_rx_nb(uart_dev_t8 dev, void *data, uint8_t max_len)
{
    uint8_t copy_amt;
    struct uart *uart = get_uart(dev);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        copy_amt = queue_pop(&uart->rx_queue, data, max_len);
        uart->overrun = false;
        uart->frame_error = false;
    }

    return copy_amt;
}

static void
set_baud(struct uart *uart, uint32_t speed)
{
    uint16_t baud_reg;

    /*--------------------------------------------------------------------------
    Calculate Baud Register Value.
    First, calculate double the needed number,
    Then assign it to the baud_register
    If the LSB of the baud number is one, round up
    The baud register value.
    --------------------------------------------------------------------------*/
    speed = (2 * F_CPU / (16 * speed)) - 2;

    baud_reg = speed >> 1;

    if (speed & 0x00000001)
        baud_reg++;

    /* Write to Baud Rate Registers */
    uart->reg->ubrrh = (baud_reg >> 8) & 0x00FF;
    uart->reg->ubrrl = (uint8_t)(baud_reg & 0x00FF);
}


void
UART_init(uart_dev_t8 dev, uint8_t opts, uint32_t speed)
{
    uint8_t ucsra = 0;
    uint8_t ucsrb = 0;
    uint8_t ucsrc = 0;
    struct uart *uart = get_uart(dev);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        uart->dev = dev;
        uart->reg = uart_def[dev].reg;
        uart->xck = uart_def[dev].xck;
        uart->opts = opts;
        queue_init(&uart->rx_queue, uart->rx_buffer, sizeof(uart->rx_buffer));
        uart->frame_error = false;
        uart->overrun = false;
        uart->rx_cnt = 0;
        uart->rx_dropped = 0;
        queue_init(&uart->tx_queue, uart->tx_buffer, sizeof(uart->tx_buffer));
        uart->tx_pending = false;
        uart->cts_pin.pin = PIN_INVALID;
        uart->cts_invert = false;

        /* Get Control Register Values */
        if (opts & UART_RX)
            ucsrb |= _BV(RXEN0) | _BV(RXCIE0);

        if (opts & UART_TX)
            ucsrb |= _BV(TXEN0);


        if (opts & UART_SYNC) {
            ucsrc |= _BV(UMSEL00);

            if (opts & UART_SYNC_POL_HI)
                ucsrc |= _BV(UCPOL0);

            pin_set_direction(uart->xck, PIN_OUTPUT);
        } else if (opts & UART_SPI) {
            ucsrc |= _BV(UMSEL00) | _BV(UMSEL01);

            if (opts & _BV(5))
                ucsrc |= _BV(UCPHA0);

            if (opts & _BV(6))
                ucsrc |= _BV(UCPOL0);

            if (opts & UART_SPI_LSB)
                ucsrc |= _BV(UDORD0);

            pin_set_direction(uart->xck, PIN_OUTPUT);
        } else {
            ucsrc |= _BV(UCSZ01) | _BV(UCSZ00);
        }

        /*
         * Write control regisers
         */
        uart->reg->ucsra = ucsra;
        uart->reg->ucsrb = ucsrb;
        uart->reg->ucsrc = ucsrc;

        /*
         * NOTE: SPI mode requires that the baud be set after the transmitter is
         * enabled
         */
        set_baud(uart, speed);
    }
}

void
UART_change_baud(uart_dev_t8 dev, uint32_t speed)
{
    struct uart *uart = get_uart(dev);
    UART_tx_wait(dev);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        set_baud(uart, speed);
    }
}

static void
process_pcie(void *param)
{
    struct uart *uart = param;

    /*
     * If the UART is waiting for a transmit and is now ready, re-enable the TX
     * interrupt
     */
    if (get_tx_ready(uart))
        uart->reg->ucsrb |= _BV(UDRIE0);
}

void
UART_set_cts_in(uart_dev_t8 dev, enum pin pin, bool invert)
{
    struct uart *uart = get_uart(dev);

    uart->cts_invert = invert;

    pin_set_direction(pin, PIN_INPUT);

    uart->cts_pin.pin = pin;
    uart->cts_pin.callback = process_pcie;
    uart->cts_pin.param = uart;

    pin_enable_interrupt(&uart->cts_pin);
}

static void
process_rx(uart_dev_t8 dev)
{
    struct uart *uart = get_uart(dev);
    uint8_t data = uart->reg->udr;

    if (uart->reg->ucsra & _BV(FE0)) {
        uart->frame_error = true;
    } else if (queue_is_full(&uart->rx_queue)) {
        uart->overrun = true;
        uart->rx_dropped++;
    } else {
        queue_push(&uart->rx_queue, &data, 1);
    }

    if (uart->reg->ucsra & _BV(DOR0))
        uart->overrun = true;

    uart->rx_cnt++;
}


static void
process_tx(uart_dev_t8 dev)
{
    struct uart *uart = get_uart(dev);
    uint8_t data;

    if (get_tx_ready(uart)) {
        if (uart->reg->ucsra & _BV(UDRE0)) {
            queue_pop(&uart->tx_queue, &data, 1);
            uart->reg->udr = data;
        }
    } else {
        /*
         * Can't transmit. Disable the interrupt
         */
        uart->reg->ucsrb &= ~_BV(UDRIE0);
    }
}

ISR(USART0_RX_vect)
{
    process_rx( UART0 );
}

ISR(USART0_UDRE_vect)
{
    process_tx( UART0 );
}

ISR(USART1_RX_vect)
{
    process_rx( UART1 );
}

ISR(USART1_UDRE_vect)
{
    process_tx( UART1 );
}

