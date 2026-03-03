#include "ring_buffer.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_ITEMS (1UL << 16)

#define TEST(capacity, cons, prod)                                             \
    {                                                                          \
        if (_ring_buffer_resize(&rb, capacity) < 0) {                           \
            fprintf(stderr, "TEST: ring_buffer_resize\n");                     \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
        pthread_t cons_thread, prod_thread;                                    \
        pthread_create(&cons_thread, NULL, prod, NULL);                        \
        pthread_create(&prod_thread, NULL, cons, NULL);                        \
        pthread_join(cons_thread, NULL);                                       \
        pthread_join(prod_thread, NULL);                                       \
    }

ring_buffer_t rb;

static inline void chaos(void) {
    if (rand() % 7) {
        for (volatile int i = 0; i < (rand() & 255); i++) {
            // Burn cycles.
        }
    } else {
        sched_yield();
    }
}

void *producer(void *arg) {
    (void)arg;
    for (int i = 0; i < (int)NUM_ITEMS; i++) {
        chaos();
        
        while (ring_buffer_push(&rb, (int *) &i) < 0) {
            chaos();
        }
    }

    return NULL;
}

void *consumer(void *arg) {
    (void)arg;
    for (int i = 0; i < (int)NUM_ITEMS; i++) {
        int value;

        chaos();
        while (ring_buffer_pop(&rb, &value) < 0) {
            chaos();
        }

        if (value != i) {
            fprintf(stderr, "consumer: value != i\n");
            exit(EXIT_FAILURE);
        }
    }

    return NULL;
}

int main(void) {
    srand(time(NULL));

    rb.elt_size = sizeof(int);

    // Normal test.
    TEST(1024, producer, consumer);

    // Varying capacities (even).
    for (size_t i = 1; i < 100000; i *= 2) {
        TEST(i, producer, consumer);
    }

    // Varying capacities (odd).
    for (size_t i = 1; i < 100000; i = i * 2 + 1) {
        TEST(i, producer, consumer);
    }

    ring_buffer_deinit(&rb);

    return 0;
}
