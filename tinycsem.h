
#include <stddef.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <semaphore.h>
#endif

/*
 * Semaphore structure
 */
struct tinycsem {
#ifdef _WIN32
    HANDLE data;
#else
    sem_t  data;
#endif
};

/*
 * Initialize a semaphore with a value of "count".
 *
 * The count of the semaphore must always be lower
 * than "max", else behaviour is undefined.
 */
bool tinycsem_init(struct tinycsem *sem, int count, int max);

/*
 * Releases the resources held by a semaphore previously
 * initialized with "tinycsem_init". 
 * 
 * After this function is used on a semaphore, you can't
 * reuse it unless you initialize it again.
 */
bool tinycsem_free(struct tinycsem *sem);

/*
 * The usual semaphore wait operation.
 */
bool tinycsem_wait(struct tinycsem *sem);

/*
 * The usual semaphore signal operation.
 */
bool tinycsem_signal(struct tinycsem *sem);