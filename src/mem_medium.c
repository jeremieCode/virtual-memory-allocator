#include <stdint.h>
#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

unsigned int puiss2(unsigned long size)
{
    unsigned int p = 0;
    size = size - 1;
    while (size)
    {
        p++;
        size >>= 1;
    }
    return p;
}

void *emalloc_medium(unsigned long size)
{
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);

    unsigned int index = puiss2(size + 32);
    unsigned int max_index = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;

    /* find lowest available index */
    unsigned int found = index;
    while (found <= max_index && arena.TZL[found] == NULL)
        found++;

    /* no block available: ask the OS for a new one */
    if (found > max_index)
    {
        mem_realloc_medium();
        found = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant - 1;
    }

    /* pop block from TZL[found] */
    void *block = arena.TZL[found];
    arena.TZL[found] = *(void **)block;

    /* split down to target index, pushing upper halves into TZL */
    while (found > index)
    {
        found--;
        void *upper = (void *)((uintptr_t)block + (1UL << found));
        *(void **)upper = arena.TZL[found];
        arena.TZL[found] = upper;
        /* block keeps pointing to the lower half */
    }

    return mark_memarea_and_get_user_ptr(1UL << index, (uint8_t *)block, MEDIUM_KIND);
}

void efree_medium(Alloc a)
{
    unsigned int index = puiss2(a.size);
    void *block = a.ptr;
    unsigned int max_index = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant - 1;

    while (index < max_index)
    {
        void *buddy = (void *)((uintptr_t)block ^ (uintptr_t)a.size);

        /* walk TZL[index] to find the buddy */
        void **prev = (void **)&arena.TZL[index];
        while (*prev != NULL && *prev != buddy)
            prev = (void **)(*prev);

        if (*prev == NULL)
            break; /* buddy not free, stop merging */

        /* remove buddy from list */
        *prev = *(void **)buddy;

        /* merged block starts at the lower address */
        if ((uintptr_t)buddy < (uintptr_t)block)
            block = buddy;

        a.size *= 2;
        index++;
    }

    /* insert merged (or original) block into TZL */
    *(void **)block = arena.TZL[index];
    arena.TZL[index] = block;
}
