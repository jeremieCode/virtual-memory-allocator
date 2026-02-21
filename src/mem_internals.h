#ifndef MEM_INTERNALS_H
#define MEM_INTERNALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define handle_fatalError(msg)						\
    do { char c[1048] = {}; snprintf(c, 1048, "%s %s %d",msg,           \
				     __FILE__, __LINE__);		\
	perror(c); exit(EXIT_FAILURE); } while(0)

  constexpr ssize_t SMALLALLOC = 64;
// SMALLALLOC + 2 MAGIC + 2 Tailles sur 8o == 96o 
  constexpr ssize_t CHUNKSIZE = 96;
// 128 Kio == 128 * 1024 == 2**17 == (1<<17)
  constexpr ssize_t LARGEALLOC = (1<<17); 

// 2**13o == 16Kio
  constexpr ssize_t FIRST_ALLOC_SMALL = (CHUNKSIZE <<7); // 96o * 128
  constexpr ssize_t FIRST_ALLOC_MEDIUM_EXPOSANT = 17;
  constexpr ssize_t FIRST_ALLOC_MEDIUM = (1<<FIRST_ALLOC_MEDIUM_EXPOSANT);

// values from 0 to 3 fit in 2 bit
typedef enum _MemKind { SMALL_KIND, MEDIUM_KIND, LARGE_KIND } MemKind;

  constexpr ssize_t TZL_SIZE = 48;
    
typedef struct _MemArena {
    void *chunkpool;
    void *TZL[TZL_SIZE];
    int small_next_exponant;
    int medium_next_exponant;
} MemArena;

typedef struct _Alloc {
    void *ptr;
    MemKind kind;
    unsigned long size;
} Alloc;

extern MemArena arena;

unsigned long knuth_mmix_one_round(unsigned long in);
[[nodiscard]]
void *mark_memarea_and_get_user_ptr(ssize_t size, uint8_t *ptr,  MemKind k); // C++-23 can not use ptr[static size]
Alloc mark_check_and_get_alloc(void *ptr);
unsigned int nb_TZL_entries();
    
unsigned long mem_realloc_small();
unsigned long mem_realloc_medium();

void *emalloc_small(unsigned long size);
void *emalloc_medium(unsigned long size);
void *emalloc_large(unsigned long size);

void efree_small(Alloc a);
void efree_medium(Alloc a);
void efree_large(Alloc a);

#ifdef __cplusplus
}
#endif


#endif
