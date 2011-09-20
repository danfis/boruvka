/***
 * fermat
 * -------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
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
#include <fermat/alloc.h>

#ifdef FER_MEMCHECK
#include <fermat/list.h>

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

void ferFreeCheck(void *_ptr)
{
    void *ptr;

    ptr = (void *)((char *)_ptr - sizeof(info_t));
    cur_alloc -= ((info_t *)ptr)->size;
    free(ptr);
}
#endif /* FER_MEMCHECK */


void *ferRealloc(void *ptr, size_t size)
{
    void *ret;
   
#ifndef FER_MEMCHECK
    ret = realloc(ptr, size);
#else /* FER_MEMCHECK */
    ret = realloc(ptr, size + sizeof(info_t));
    if (ret){
        ((info_t *)ret)->size = size;
        ret = (void *)((char *)ret + sizeof(info_t));

        cur_alloc += size;
        if (cur_alloc > max_alloc)
            max_alloc = cur_alloc;
    }
#endif /* FER_MEMCHECK */

    if (ret == NULL && size != 0){
        fprintf(stderr, "Fatal error: Allocation of memory failed!\n");
        fflush(stderr);
        exit(-1);
    }

#ifdef FER_MEMCHECK
    if (!reg_at_exit){
        atexit(stats);
        reg_at_exit = 1;
    }
#endif /* FER_MEMCHECK */

    return ret;
}

void *ferAllocAlign(size_t size, size_t alignment)
{
    void *mem;

    if (posix_memalign(&mem, alignment, size) != 0){
        fprintf(stderr, "Fatal error: Allocation of memory failed!\n");
        fflush(stderr);
        exit(-1);
    }

    return mem;
}

