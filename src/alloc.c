#include <stdio.h>
#include <mg/alloc.h>

void *mgRealloc(void *ptr, size_t size)
{
    void *ret = realloc(ptr, size);
    if (ret == NULL && size != 0){
        fprintf(stderr, "Fatal error: Allocation of memory failed!\n");
        fflush(stderr);
        exit(-1);
    }

    return ret;
}


