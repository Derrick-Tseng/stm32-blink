#ifndef RING_H
#define RING_H

#include <stdint.h>
#include <stdbool.h>

#define RB_SIZE 128u
#define RB_MASK (RB_SIZE - 1u)

typedef struct{
    volatile uint8_t buf[RB_SIZE];
    volatile uint16_t head;  // Write loc
    volatile uint16_t tail;  // Read loc
}ringbuf_t;

void rb_init(ringbuf_t *rb);
bool rb_put(ringbuf_t *rb, uint8_t data); //滿了就丟
bool rb_get(ringbuf_t *rb, uint8_t *data);
uint16_t rb_count(const ringbuf_t *rb);

#endif