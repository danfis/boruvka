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

void *ferRealloc(void *ptr, size_t size)
{
    void *ret = realloc(ptr, size);
    if (ret == NULL && size != 0){
        fprintf(stderr, "Fatal error: Allocation of memory failed!\n");
        fflush(stderr);
        exit(-1);
    }

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

