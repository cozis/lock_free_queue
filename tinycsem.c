#include "tinycsem.h"

bool tinycsem_init(struct tinycsem *sem, int count, int max)
{
    #ifdef _WIN32
    SECURITY_ATTRIBUTES *attr = NULL; // Default
    const char          *name = NULL; // No name
    HANDLE handle = CreateSemaphore(attr, count, max, name);
    if (handle == NULL)
        return false;
    sem->data = handle;
    return true;
    #else
    (void) max; // POSIX doesn't use this
    return sem_init(&sem->data, 0, count) == 0;
    #endif
}

bool tinycsem_free(struct tinycsem *sem)
{
    #ifdef _WIN32
    CloseHandle(sem->data);
    return true;
    #else
    return sem_destroy(&sem->data) == 0;
    #endif
}

bool tinycsem_wait(struct tinycsem *sem)
{
    #ifdef _WIN32
    return WaitForSingleObject(sem->data, INFINITE) == WAIT_OBJECT_0;
    #else
    return sem_wait(&sem->data) == 0;
    #endif
}

bool tinycsem_signal(struct tinycsem *sem)
{
    #ifdef _WIN32
    return ReleaseSemaphore(sem->data, 1, NULL);
    #else
    return sem_post(&sem->data) == 0;
    #endif
}