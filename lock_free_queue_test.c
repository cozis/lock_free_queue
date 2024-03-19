#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "tinycthread.h"
#include "tinycsem.h"
#include "lock_free_queue.h"

#define NUM_THREADS 30

struct lock_free_queue queue;

struct tinycsem consumed;
struct tinycsem produced;

static int 
kind_of_atomic_fprintf(FILE *stream, const char *fmt, ...)
{
    char msg[256];

    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    if (len < 0) return 0;

    return fwrite(msg, 1, len, stream);
}

#define OPERATIONS_PER_WORKER 10000

int consumer(void *arg)
{
    (void) arg;

    int sum = 0;

    for (int i = 0; i < OPERATIONS_PER_WORKER; i++) {

        tinycsem_wait(&produced);

        int value;
        lock_free_queue_pop(&queue, &value);

        sum += value;

        kind_of_atomic_fprintf(stderr, "Thread consumed %d (i=%d)\n", value, i);

        tinycsem_signal(&consumed);
    }

    kind_of_atomic_fprintf(stderr, "Consumer stopped (sum=%d)\n", sum);
    return sum;
}

int producer(void *arg)
{
    (void) arg;
    
    srand(time(NULL));

    int sum = 0;
    
    for (int i = 0; i < OPERATIONS_PER_WORKER; i++) {
        tinycsem_wait(&consumed);

        int value = rand() % 10000;

        lock_free_queue_push(&queue, &value);

        kind_of_atomic_fprintf(stderr, "Thread produced %d (i=%d)\n", value, i);

        sum += value;

        tinycsem_signal(&produced);
    }

    kind_of_atomic_fprintf(stderr, "Producer stopped (sum=%d)\n", sum);
    return sum;
}

#define MAX_ITEMS 100

int main()
{
    int items[MAX_ITEMS];
    lock_free_queue_INIT(&queue, items);

    tinycsem_init(&consumed, MAX_ITEMS, MAX_ITEMS);
    tinycsem_init(&produced, 0,         MAX_ITEMS);

    thrd_t threads[NUM_THREADS];
    int created = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        int (*routine)(void*) = i & 1 ? producer : consumer;
        if (thrd_create(&threads[i], routine, NULL) == thrd_success)
            created++;
    }
    
    if (created == 0) {
        fprintf(stderr, "No thread was created\n");
        return -1;
    }
    
    if (created < NUM_THREADS)
        fprintf(stderr, "Only %d threads were created out of %d\n", created, NUM_THREADS);
    else
        fprintf(stderr, "Threads created\n");

    int produced_sum = 0;
    int consumed_sum = 0;
    for (int i = 0; i < created; i++) {
        int sum;
        thrd_join(threads[i], &sum);
        if (i & 1)
            produced_sum += sum;
        else
            consumed_sum += sum;
    }

    fprintf(stderr, "produced=%d, consumed=%d -- %s\n", 
        produced_sum, consumed_sum,
        (produced_sum == consumed_sum) ? "OK" : "Don't match!!");

    tinycsem_free(&consumed);
    tinycsem_free(&produced);
    return 0;
}