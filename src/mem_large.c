#include <sys/mman.h>
#include "mem.h"
#include "mem_internals.h"

void *
emalloc_large(unsigned long size)
{
    unsigned long taille= size + 32;
    void *newmem =  mmap(0,
			 taille,
			 PROT_READ | PROT_WRITE | PROT_EXEC,
			 MAP_PRIVATE | MAP_ANONYMOUS,
			 -1,
			 0);
     if (newmem == MAP_FAILED)
	handle_fatalError("large alloc fails");

     return mark_memarea_and_get_user_ptr(taille, newmem, LARGE_KIND);
}

void efree_large(Alloc a) {
    int ret = munmap(a.ptr, a.size);

    if (ret == -1)
	handle_fatalError("large free fails");
}
