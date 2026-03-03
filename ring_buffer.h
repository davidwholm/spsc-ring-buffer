#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

#define CACHE_LINE_SIZE 64

struct ring_buffer_t {
    size_t capacity;
    size_t elt_size;
    uint8_t *data;
    atomic_size_t head __attribute__((aligned(CACHE_LINE_SIZE)));
    atomic_size_t tail __attribute__((aligned(CACHE_LINE_SIZE)));
};

typedef struct ring_buffer_t ring_buffer_t;

int ring_buffer_init(ring_buffer_t *rb, size_t capacity, size_t elt_size);
void ring_buffer_deinit(ring_buffer_t *rb);

int ring_buffer_resize(ring_buffer_t *rb, size_t capacity);
int _ring_buffer_resize(ring_buffer_t *rb, size_t capacity);

int ring_buffer_push(ring_buffer_t *rb, void *item);
int ring_buffer_pop(ring_buffer_t *rb, void *out);

#endif
