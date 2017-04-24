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
#include "queue.h"

static size_t
queue_next(struct queue const *queue, size_t idx)
{
    size_t next = idx + 1;
    if (next >= queue->size)
        next = 0;
    return next;
}

void
queue_init(struct queue *queue, void *buffer, size_t size)
{
    queue->buffer = buffer;
    queue->size = size;
    queue->head = 0;
    queue->tail = 0;
}

bool
queue_is_empty(struct queue const *queue)
{
    return queue->head == queue->tail;
}

bool
queue_is_full(struct queue const *queue)
{
    return queue_next(queue, queue->head) == queue->tail;
}

size_t
queue_push(struct queue *queue, void const *ptr, size_t len)
{
    size_t cnt = 0;
    size_t next_head = queue_next(queue, queue->head);

    while (next_head != queue->tail && cnt < len) {
        queue->buffer[queue->head] = ((uint8_t const *)ptr)[cnt];
        queue->head = next_head;
        next_head = queue_next(queue, next_head);
        cnt++;
    }
    return cnt;
}

size_t
queue_pop(struct queue *queue, void *ptr, size_t len)
{
    size_t cnt = 0;
    while (!queue_is_empty(queue) && cnt < len) {
        ((uint8_t *)ptr)[cnt] = queue->buffer[queue->tail];
        queue->tail = queue_next(queue, queue->tail);
        cnt++;
    }
    return cnt;
}

