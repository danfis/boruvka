/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
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

#include <boruvka/varr.h>
#include <boruvka/alloc.h>

#define BOR_VARR_INIT_SIZE 2

void borVArrInit(bor_varr_t *arr, size_t init_size, size_t elemsize)
{
    arr->size  = 0;
    arr->alloc = init_size;
    arr->arr   = BOR_MALLOC(arr->alloc * elemsize);
}

void borVArrDestroy(bor_varr_t *arr)
{
    BOR_FREE(arr->arr);
}

void borVArrExpand(bor_varr_t *arr, size_t factor, size_t elemsize)
{
    if (arr->alloc == 0)
        arr->alloc = BOR_VARR_INIT_SIZE;
    arr->alloc *= factor;
    arr->arr = BOR_REALLOC(arr->arr, arr->alloc * elemsize);
}
