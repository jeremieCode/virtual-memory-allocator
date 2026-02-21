#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

static void build_chunkpool(uint8_t *region, unsigned long region_size)
{
    unsigned long nb_chunks = region_size / CHUNKSIZE;
    for (unsigned long i = 0; i < nb_chunks; i++)
    {
        void **current = (void **)(region + i * CHUNKSIZE);
        void *next = (i + 1 < nb_chunks)
                         ? (void *)(region + (i + 1) * CHUNKSIZE)
                         : NULL;
        *current = next;
    }
    arena.chunkpool = (void *)region;
}

void *emalloc_small(unsigned long size)
{
    (void)size; /* actual allocation is always CHUNKSIZE */

    if (arena.chunkpool == NULL)
    {
        unsigned long region_size = mem_realloc_small();
        build_chunkpool((uint8_t *)arena.chunkpool, region_size);
    }

    /* pop head */
    void *chunk = arena.chunkpool;
    arena.chunkpool = *(void **)chunk;

    return mark_memarea_and_get_user_ptr(CHUNKSIZE, (uint8_t *)chunk, SMALL_KIND);
}

void efree_small(Alloc a)
{
    *(void **)a.ptr = arena.chunkpool;
    arena.chunkpool = a.ptr;
}