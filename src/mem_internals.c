#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"

unsigned long knuth_mmix_one_round(unsigned long in)
{
    return in * 6364136223846793005UL % 1442695040888963407UL;
}

static inline uint64_t compute_magic(const uint8_t *base, MemKind k)
{
    uint64_t raw = knuth_mmix_one_round((unsigned long)base);
    return (raw & ~0x3UL) | (unsigned long)k;
}

void *mark_memarea_and_get_user_ptr(ssize_t size, uint8_t *ptr, MemKind k)
{
    uint64_t magic = compute_magic(ptr, k);

    uint64_t *header = (uint64_t *)ptr;
    header[0] = (uint64_t)size;
    header[1] = magic;

    uint64_t *footer = (uint64_t *)(ptr + size - 16);
    footer[0] = magic;
    footer[1] = (uint64_t)size;

    return (void *)(ptr + 16);
}

Alloc mark_check_and_get_alloc(void *ptr)
{
    uint8_t *base = (uint8_t *)ptr - 16;
    uint64_t *header = (uint64_t *)base;

    uint64_t stored_size = header[0];
    uint64_t stored_magic = header[1];

    MemKind kind = (MemKind)(stored_magic & 0x3UL);
    uint64_t expected_magic = compute_magic(base, kind);

    assert(stored_magic == expected_magic && "Header magic mismatch: probable buffer overflow before user zone");

    uint64_t *footer = (uint64_t *)(base + stored_size - 16);
    assert(footer[0] == stored_magic && "Footer magic mismatch: probable buffer overflow past user zone");
    assert(footer[1] == stored_size && "Footer size mismatch: probable buffer overflow past user zone");

    Alloc a;
    a.ptr = (void *)base;
    a.size = (unsigned long)stored_size;
    a.kind = kind;
    return a;
}

unsigned long mem_realloc_small(void)
{
    assert(arena.chunkpool == 0);
    unsigned long size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
    arena.chunkpool = mmap(0,
                           size,
                           PROT_READ | PROT_WRITE | PROT_EXEC,
                           MAP_PRIVATE | MAP_ANONYMOUS,
                           -1,
                           0);
    if (arena.chunkpool == MAP_FAILED)
        handle_fatalError("small realloc");
    arena.small_next_exponant++;
    return size;
}

unsigned long mem_realloc_medium(void)
{
    uint32_t indice = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    assert(arena.TZL[indice] == 0);
    unsigned long size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
    assert(size == (1UL << indice));
    arena.TZL[indice] = mmap(0,
                             size * 2,
                             PROT_READ | PROT_WRITE | PROT_EXEC,
                             MAP_PRIVATE | MAP_ANONYMOUS,
                             -1,
                             0);
    if (arena.TZL[indice] == MAP_FAILED)
        handle_fatalError("medium realloc");
    arena.TZL[indice] += (size - (((intptr_t)arena.TZL[indice]) % size));
    arena.medium_next_exponant++;
    return size;
}

unsigned int nb_TZL_entries(void)
{
    int nb = 0;
    for (int i = 0; i < TZL_SIZE; i++)
        if (arena.TZL[i])
            nb++;
    return nb;
}
