#include <string.h>
#include <assert.h>
#include <stdatomic.h>
#include "lock_free_queue.h"

void
lock_free_queue_init(struct lock_free_queue *q, 
                     void *arrptr, int arrlen,
                     int cell)
{
    assert(arrlen % cell == 0);

    q->head = 0;
    q->tail = 0;
    q->temp = 0;
    q->size = arrlen / cell;
    q->base = arrptr;
    q->cell = cell;
}

static void*
item_addr(struct lock_free_queue *q, int index)
{
    assert(index >= 0 && index < q->size);
    return q->base + q->cell * index;
}

static int
acquire_push_location(struct lock_free_queue *q)
{
    int old_temp;
    int new_temp;
    do {
        old_temp = q->temp;
        new_temp = (old_temp + 1) % q->size;
    } while (!atomic_compare_exchange_strong(&q->temp, &old_temp, new_temp));
    return old_temp;
}

static void 
release_push_location(struct lock_free_queue *q, int index)
{
    // The current thread already inserted an
    // item at position "index", which comes
    // after the queue's tail.
    //
    // It may be possible that other threads
    // inserted items before this one, buf still
    // after the tail
    //
    // Before being able to move the tail over
    // this element, we need to wait for other
    // threads to do it.
    while (q->tail != index);

    // At this point only this thread is allowed
    // to push the tail forward

    q->tail = (q->tail + 1) % q->size;
}

void
lock_free_queue_push(struct lock_free_queue *q, void *src)
{
    int index = acquire_push_location(q);

    void *dst = item_addr(q, index);
    memcpy(dst, src, q->cell);

    release_push_location(q, index);
}

void
lock_free_queue_pop(struct lock_free_queue *q, void *dst)
{
    int old_head;
    int new_head;
    do {
        old_head = q->head;
        new_head = (old_head + 1) % q->size;
        void *src = item_addr(q, old_head);
        memcpy(dst, src, q->cell);
    } while (!atomic_compare_exchange_strong(&q->head, &old_head, new_head));
}
