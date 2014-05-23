/***
 * Boruvka
 * --------
 * Copyright (c)2014 Daniel Fiser <danfis@danfis.cz>
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

#include <boruvka/htable.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

#define BOR_HTABLE_INITIAL_SIZE 100


struct _bor_htable_el_t {
    bor_list_t list;
    void *key;
};

static int _eq(const bor_list_t *key1, const bor_list_t *key2, void *userdata);

bor_htable_t *borHTableNew(bor_htable_hash_fn hash_func,
                           bor_htable_eq_fn eq_func,
                           void *userdata)
{
    bor_htable_t *htable;
    size_t i;

    htable = BOR_ALLOC(bor_htable_t);
    htable->size = BOR_HTABLE_INITIAL_SIZE;
    htable->table = BOR_ALLOC_ARR(bor_list_t, htable->size);
    htable->num_elements = 0;

    htable->hash = hash_func;
    htable->eq   = eq_func;
    htable->data = userdata;

    if (!htable->eq)
        htable->eq = _eq;

    for (i = 0; i < htable->size; i++){
        borListInit(htable->table + i);
    }

    return htable;
}

void borHTableDel(bor_htable_t *h)
{
    size_t i;
    bor_list_t *list, *item;

    for (i = 0; i < h->size; i++){
        list = &h->table[i];
        while (!borListEmpty(list)){
            item = borListNext(list);
            borListDel(item);
        }
    }

    BOR_FREE(h->table);
    BOR_FREE(h);
}

void borHTableGather(bor_htable_t *m, bor_list_t *list)
{
    size_t i;
    bor_list_t *item;

    for (i = 0; i < m->size; i++){
        while (!borListEmpty(&m->table[i])){
            item = borListNext(&m->table[i]);
            borListDel(item);
            borListAppend(list, item);
        }
    }
    m->num_elements = 0;
}

size_t borHTableFindAll(const bor_htable_t *m, const bor_list_t *key1,
                        bor_list_t ***out_arr, size_t *size)
{
    bor_list_t *item;
    size_t found_size, bucket;
    int reallocate = 0;

    bucket = borHTableBucket(m, key1);

    if (*out_arr == 0x0)
        reallocate = 1;

    found_size = 0;
    BOR_LIST_FOR_EACH(&m->table[bucket], item){
        if (m->eq(key1, item, m->data)){
            ++found_size;
            if (reallocate){
                (*out_arr) = BOR_REALLOC_ARR(*out_arr, bor_list_t *,
                                             found_size);
                (*out_arr)[found_size - 1] = item;

            }else if (found_size <= *size){
                (*out_arr)[found_size - 1] = item;
            }
        }
    }

    if (reallocate)
        *size = found_size;

    return found_size;
}

void borHTableResize(bor_htable_t *m, size_t size)
{
    bor_list_t *old_table, *item;
    size_t old_size;
    size_t i;

    // remember old table and old size
    old_table = m->table;
    old_size  = m->size;

    // create a new empty table
    m->table = BOR_ALLOC_ARR(bor_list_t, size);
    m->size  = size;
    m->num_elements = 0;

    for (i = 0; i < m->size; i++){
        borListInit(m->table + i);
    }

    for (i = 0; i < old_size; i++){
        while (!borListEmpty(&old_table[i])){
            // remove item from the old table
            item = borListNext(&old_table[i]);
            borListDel(item);

            // insert it into a new table
            borHTableInsertNoResize(m, item);
        }
    }

    BOR_FREE(old_table);
}

static int _eq(const bor_list_t *key1, const bor_list_t *key2, void *userdata)
{
    return key1 == key2;
}

