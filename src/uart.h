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
#include <avr/io.h>
#include <avr/interrupt.h>

#include "pin.h"

typedef uint8_t uart_dev_t8; enum {
    UART0,
    UART1,

    UART_CNT
};

#define UART_RX _BV(0)
#define UART_TX _BV(1)

#define UART_ASYNC (0)

#define UART_SYNC _BV(2)
#define UART_SYNC_POL_HI _BV(3) // Sample on rising edge of Clock

#define UART_SPI _BV(4)
#define UART_SPI_MODE_0 (UART_SPI | (0 << 5))
#define UART_SPI_MODE_1 (UART_SPI | (1 << 5))
#define UART_SPI_MODE_2 (UART_SPI | (2 << 5))
#define UART_SPI_MODE_3 (UART_SPI | (3 << 5))
#define UART_SPI_MSB (0)
#define UART_SPI_LSB _BV(7)

void
UART_printf(uart_dev_t8 dev, char const *fmt, ...);

void
UART_tx(uart_dev_t8 dev, void const *data, uint8_t len);

uint8_t
UART_tx_nb(uart_dev_t8 dev, void const *data, uint8_t len);

bool
UART_tx_empty(uart_dev_t8 dev);

void
UART_tx_wait(uart_dev_t8 dev);

void
UART_rx(uart_dev_t8 dev, void *data, uint8_t len);

uint8_t
UART_rx_nb(uart_dev_t8 dev, void *data, uint8_t max_len);

void
UART_init(uart_dev_t8 dev, uint8_t flags, uint32_t speed);

void
UART_change_baud(uart_dev_t8 dev, uint32_t speed);

void
UART_set_cts_in(uart_dev_t8 dev, enum pin pin, bool invert);

