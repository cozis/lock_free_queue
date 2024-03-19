
struct lock_free_queue {

    _Atomic(int) head;
    _Atomic(int) tail;
    _Atomic(int) temp;

    // The following aren't written to by
    // push and pop operations, therefore
    // don't need to be atomic.
    int   size; // Capacity of the queue (item count)
    int   cell; // Size of a single item (in bytes)
    char *base; // Pointer to the first item
};
void lock_free_queue_init(struct lock_free_queue *q, void *arrptr, int arrlen, int item_size);
void lock_free_queue_push(struct lock_free_queue *q, void *src);
void lock_free_queue_pop(struct lock_free_queue *q, void *dst);

#define lock_free_queue_INIT(q, arr) lock_free_queue_init(q, arr, sizeof(arr), sizeof((arr)[0]))