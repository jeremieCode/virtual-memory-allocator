#ifndef MEM_H
#define MEM_H

#ifdef __cplusplus
extern "C"
{
#endif

    void *emalloc(unsigned long size);
    void efree(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
