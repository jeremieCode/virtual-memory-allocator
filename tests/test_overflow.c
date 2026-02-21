#include <stdio.h>
#include "../src/mem.h"

int main() {
    printf("Allocating...\n");

    void *p = emalloc(200000);
    if (!p) {
        printf("Allocation failed\n");
        return 1;
    }

    printf("Writing out of bounds...\n");

    char *c = (char*)p;

    c[200000] = 42;
    printf("Freeing...\n");

    efree(p);

    printf("If you see this message, it's a bad sign.\n");

    return 0;
}