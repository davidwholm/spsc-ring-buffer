#include "ring_buffer.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef DEBUG
#define DBG(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#else
#define DBG(fmt, ...)
#endif

int ring_buffer_init(ring_buffer_t *rb, size_t capacity, size_t elt_size) {
    if (capacity == 0) {
        DBG("ring_buffer_init: capacity == 0\n");
        return -1;
    }

    if (elt_size == 0) {
        DBG("ring_buffer_init: elt_size == 0\n");
        return -1;
    }

    // Make sure that cap * sz + 1 ≤ SIZE_MAX so that there's no overflow.
    // cap * sz < SIZE_MAX ⇔ cap < SIZE_MAX / sz
    // To report such an error, check for ¬(cap < SIZE_MAX / sz) ⇔ cap ≥ SIZE_MAX / sz
    
    if (capacity >= SIZE_MAX / elt_size) {
        DBG("ring_buffer_init: capacity >= SIZE_MAX / elt_size\n");
        return -1;
    }

    rb->capacity = capacity;
    rb->elt_size = elt_size;

    if (!(rb->data = malloc((rb->capacity * rb->elt_size) + 1))) {
        DBG("ring_buffer_init: malloc (%s)\n", strerror(errno));
        return -1;
    }

    atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);

    return 0;
}

void ring_buffer_deinit(ring_buffer_t *rb) {
    if (rb) {
        free(rb->data);
    }
}

int _ring_buffer_resize(ring_buffer_t *rb, size_t capacity) {
    if (capacity == 0) {
        DBG("ring_buffer_resize: capacity == 0\n");
        return -1;
    }

    if (capacity >= SIZE_MAX / rb->elt_size) {
        DBG("ring_buffer_resize: capacity >= SIZE_MAX / elt_size\n");
        return -1;
    }

    uint8_t *tmp = realloc(rb->data, (capacity * rb->elt_size) + 1);
    if (!tmp) {
        DBG("ring_buffer_resize: realloc (%s\n)", strerror(errno));
        return -1;
    }

    rb->capacity = capacity;
    rb->data = tmp;

    atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
    atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
    
    return 0;
}

int ring_buffer_resize(ring_buffer_t *rb, size_t capacity) {
    if (capacity == 0) {
        DBG("ring_buffer_resize: capacity == 0\n");
        return -1;
    }

    if (capacity >= SIZE_MAX / rb->elt_size) {
        DBG("ring_buffer_resize: capacity >= SIZE_MAX / elt_size\n");
        return -1;
    }

    uint8_t *tmp = realloc(rb->data, (capacity * rb->elt_size) + 1);
    if (!tmp) {
        DBG("ring_buffer_resize: realloc (%s\n)", strerror(errno));
        return -1;
    }

    bool should_reset = capacity < rb->capacity;

    rb->capacity = capacity;
    rb->data = tmp;

    if (should_reset) {
        atomic_store_explicit(&rb->head, 0, memory_order_relaxed);
        atomic_store_explicit(&rb->tail, 0, memory_order_relaxed);
    }
    
    return 0;
}

int ring_buffer_push(ring_buffer_t *rb, void *value) {
    size_t head = atomic_load_explicit(&rb->head, memory_order_acquire);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_relaxed);

    if ((tail + 1) % ((rb->capacity * rb->elt_size) + 1) == head) {
        DBG("ring_buffer_push: buffer full\n");
        return -1;
    }

    memcpy(rb->data + tail, value, rb->elt_size);
    atomic_store_explicit(&rb->tail, (tail + rb->elt_size) % ((rb->capacity * rb->elt_size) + 1),
                          memory_order_release);

    return 0;
}

int ring_buffer_pop(ring_buffer_t *rb, void *out) {
    size_t head = atomic_load_explicit(&rb->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&rb->tail, memory_order_acquire);

    if (head == tail) {
        DBG("ring_buffer_pop: buffer empty\n");
        return -1;
    }

    memcpy(out, rb->data + head, rb->elt_size);
    atomic_store_explicit(&rb->head, (head + rb->elt_size) % ((rb->capacity * rb->elt_size) + 1),
                          memory_order_release);
    return 0;
}
