#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "tinycthread.h"
#include "lock_free_queue.h"

#define NUM_THREADS 30

#define OPERATIONS_PER_WORKER 100

struct lock_free_queue queue;

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

int consumer(void *arg)
{
    (void) arg;

    int sum = 0;

    for (int i = 0; i < OPERATIONS_PER_WORKER; i++) {

        int value;
        lock_free_queue_pop(&queue, &value);

        sum += value;
    }

    return sum;
}

int producer(void *arg)
{
    (void) arg;
    
    srand(time(NULL));

    int sum = 0;
    
    for (int i = 0; i < OPERATIONS_PER_WORKER; i++) {

        int value = rand() % 10000;

        lock_free_queue_push(&queue, &value);

        sum += value;
    }

    return sum;
}

#define MAX_ITEMS 10

int main()
{
    int items[MAX_ITEMS];
    lock_free_queue_INIT(&queue, items);

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

    return 0;
}