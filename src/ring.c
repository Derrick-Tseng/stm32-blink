#include "ring.h"

void rb_init(ringbuf_t *rb){
    rb->head = 0;
    rb->tail = 0;
}

bool rb_put(ringbuf_t *rb, uint8_t data){
    uint16_t next = (rb->head + 1u) & RB_MASK;
    if(next == rb->tail){
        return false;
    }
    rb->buf[rb->head] = data;
    rb->head = next;
    return true;
}

bool rb_get(ringbuf_t *rb, uint8_t *data){
    // empty
    if(rb->tail == rb->head){
        return false;
    }
    *data = rb->buf[rb->tail];
    rb->tail = (rb->tail +1u) & RB_MASK;
    return true;
}


uint16_t rb_count(const ringbuf_t *rb){
    return (rb->head - rb->tail) & RB_MASK;
}

