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
    q->temp_head = 0;
    q->temp_tail = 0;
    q->size = arrlen / cell;
    q->base = arrptr;
    q->cell = cell;
}

static void*
item_addr(struct lock_free_queue *q, int index)
{
    return q->base + q->cell * (index % q->size);
}

static uint64_t
acquire_push_location(struct lock_free_queue *q)
{
    uint64_t old_temp_tail;
    uint64_t new_temp_tail;

    do {

        uint64_t old_temp_head;

        old_temp_tail = q->temp_tail;
        old_temp_head = q->temp_head;

        if (old_temp_head + q->size == old_temp_tail)
            return UINT64_MAX; // Queue is full

        assert(old_temp_head + q->size > old_temp_tail);

        new_temp_tail = old_temp_tail + 1;
    } while (!atomic_compare_exchange_weak(&q->temp_tail, &old_temp_tail, new_temp_tail));

    return old_temp_tail;
}

static void
release_push_location(struct lock_free_queue *q, uint64_t index)
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

    q->tail++;
}

bool
lock_free_queue_try_push(struct lock_free_queue *q, void *src)
{
    uint64_t index = acquire_push_location(q);
    if (index == UINT64_MAX) return false;

    void *dst = item_addr(q, index);
    memcpy(dst, src, q->cell);

    //kind_of_atomic_fprintf(stdout, "push at %d\n", index % q->size);

    release_push_location(q, index);
    return true;
}

void
lock_free_queue_push(struct lock_free_queue *q, void *src)
{
    while (!lock_free_queue_try_push(q, src));
}

static uint64_t
acquire_pop_location(struct lock_free_queue *q)
{
    uint64_t old_head;
    uint64_t new_head;
    do {
        uint64_t old_tail;

        // It's important to get head before tail
        // or head may be incremented over tail before
        // querying it.
        old_head = q->head;
        old_tail = q->tail;

        if (old_head == old_tail)
            return UINT64_MAX;

        assert(old_tail > old_head);

        new_head = old_head + 1;
    } while (!atomic_compare_exchange_weak(&q->head, &old_head, new_head));
    return old_head;
}

static void
release_pop_location(struct lock_free_queue *q, uint64_t index)
{
    while (q->temp_head != index);

    q->temp_head++;
}

bool
lock_free_queue_try_pop(struct lock_free_queue *q, void *dst)
{
    uint64_t index = acquire_pop_location(q);
    if (index == UINT64_MAX)
        return false;

    void *src = item_addr(q, index);
    memcpy(dst, src, q->cell);

    //kind_of_atomic_fprintf(stdout, "pop at %d\n", index % q->size);

    release_pop_location(q, index);
    return true;
}

void
lock_free_queue_pop(struct lock_free_queue *q, void *dst)
{
    while (!lock_free_queue_try_pop(q, dst));
}
