/***
 * Boruvka
 * --------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <stdio.h>
#include <boruvka/alloc.h>

#ifdef BOR_MEMCHECK
#include <boruvka/list.h>

/** currently allocated memory */
static unsigned long cur_alloc = 0L;
/* maximal amount of allocated mem during algorithm */
static unsigned long max_alloc = 0L;
static int reg_at_exit = 0;

struct _info_t {
    size_t size;
};
typedef struct _info_t info_t;

static void stats(void)
{
    fprintf(stderr, "MemCheck stats:\n");
    fprintf(stderr, "    max_alloc: % 10lu bytes\n", max_alloc);
    fprintf(stderr, "    unfreed:   % 10lu bytes\n", cur_alloc);
}

void borFreeCheck(void *_ptr)
{
    void *ptr;

    ptr = (void *)((char *)_ptr - sizeof(info_t));
    cur_alloc -= ((info_t *)ptr)->size;
    free(ptr);
}
#endif /* BOR_MEMCHECK */


void *borRealloc(void *ptr, size_t size)
{
    void *ret;
   
#ifndef BOR_MEMCHECK
    ret = realloc(ptr, size);
#else /* BOR_MEMCHECK */
    ret = realloc(ptr, size + sizeof(info_t));
    if (ret){
        ((info_t *)ret)->size = size;
        ret = (void *)((char *)ret + sizeof(info_t));

        cur_alloc += size;
        if (cur_alloc > max_alloc)
            max_alloc = cur_alloc;
    }
#endif /* BOR_MEMCHECK */

    if (ret == NULL && size != 0){
        fprintf(stderr, "Fatal error: Allocation of memory failed!\n");
        fflush(stderr);
        exit(-1);
    }

#ifdef BOR_MEMCHECK
    if (!reg_at_exit){
        atexit(stats);
        reg_at_exit = 1;
    }
#endif /* BOR_MEMCHECK */

    return ret;
}

void *borAllocAlign(size_t size, size_t alignment)
{
    void *mem;

    if (posix_memalign(&mem, alignment, size) != 0){
        fprintf(stderr, "Fatal error: Allocation of memory failed!\n");
        fflush(stderr);
        exit(-1);
    }

    return mem;
}

void *borCalloc(size_t nmemb, size_t size)
{
    void *ret;

    ret = calloc(nmemb, size);
    if (ret == NULL && size != 0){
        fprintf(stderr, "Fatal error: Allocation of memory failed!\n");
        fflush(stderr);
        exit(-1);
    }

    return ret;
}
