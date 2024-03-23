#include <stdint.h>
#include <stdbool.h>

/*
 * +--------------+-----------------+----------------+-------------------+-------------+
 * | unused       | Pop in progress |     Items      | Push in progress  | Unused      |
 * +--------------+-----------------+----------------+-------------------+-------------+
 *                ^                 ^                ^                   ^
 *                temp_head         head             tail                temp_tail
 */

struct lock_free_queue {

    _Atomic(uint64_t) head;
    _Atomic(uint64_t) tail;
    _Atomic(uint64_t) temp_head;
    _Atomic(uint64_t) temp_tail;

    // The following aren't written to by
    // push and pop operations, therefore
    // don't need to be atomic.
    uint64_t size; // Capacity of the queue (item count)
    uint64_t cell; // Size of a single item (in bytes)
    char *base; // Pointer to the first item
};

void lock_free_queue_init(struct lock_free_queue *q, void *arrptr, int arrlen, int item_size);
void lock_free_queue_push(struct lock_free_queue *q, void *src);
void lock_free_queue_pop(struct lock_free_queue *q, void *dst);
bool lock_free_queue_try_push(struct lock_free_queue *q, void *src);
bool lock_free_queue_try_pop(struct lock_free_queue *q, void *dst);

#define lock_free_queue_INIT(q, arr) lock_free_queue_init(q, arr, sizeof(arr), sizeof((arr)[0]))