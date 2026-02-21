#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <pthread.h>
#include "mem.h"
#include "mem_internals.h"

MemArena arena = {};

static pthread_mutex_t arena_lock = PTHREAD_MUTEX_INITIALIZER;

void *emalloc(unsigned long size)
{
    if (size == 0)
        return NULL;

    pthread_mutex_lock(&arena_lock);

    void *result;
    if (size >= LARGEALLOC)
        result = emalloc_large(size);
    else if (size <= SMALLALLOC)
        result = emalloc_small(size);
    else
        result = emalloc_medium(size);

    pthread_mutex_unlock(&arena_lock);
    return result;
}

void efree(void *ptr)
{
    pthread_mutex_lock(&arena_lock);

    Alloc a = mark_check_and_get_alloc(ptr);
    switch (a.kind)
    {
    case SMALL_KIND:
        efree_small(a);
        break;
    case MEDIUM_KIND:
        efree_medium(a);
        break;
    case LARGE_KIND:
        efree_large(a);
        break;
    default:
        assert(0);
    }

    pthread_mutex_unlock(&arena_lock);
}
