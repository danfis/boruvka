/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#include <boruvka/hmap.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>


struct _bor_hmap_el_t {
    bor_list_t list;
    void *key;
};

static int _eq(const bor_list_t *key1, const bor_list_t *key2, void *userdata);

bor_hmap_t *ferHMapNew(size_t size,
                       fer_hmap_hash_fn hash_func,
                       fer_hmap_eq_fn eq_func,
                       void *userdata)
{
    bor_hmap_t *hmap;
    size_t i;

    hmap = BOR_ALLOC(bor_hmap_t);
    hmap->table = BOR_ALLOC_ARR(bor_list_t, size);
    hmap->size = size;

    hmap->hash = hash_func;
    hmap->eq   = eq_func;
    hmap->data = userdata;

    if (!hmap->eq)
        hmap->eq = _eq;

    for (i = 0; i < hmap->size; i++){
        ferListInit(hmap->table + i);
    }

    return hmap;
}

void ferHMapDel(bor_hmap_t *h)
{
    size_t i;
    bor_list_t *list, *item;

    for (i = 0; i < h->size; i++){
        list = &h->table[i];
        while (!ferListEmpty(list)){
            item = ferListNext(list);
            ferListDel(item);
        }
    }

    BOR_FREE(h->table);
    BOR_FREE(h);
}


uint32_t ferHMapID(const bor_hmap_t *m, bor_list_t *key1)
{
    uint32_t id;

    id = m->hash(key1, m->data);
    id = id % m->size;

    return id;
}

bor_list_t *ferHMapIDGet(const bor_hmap_t *m, uint32_t id, bor_list_t *key1)
{
    bor_list_t *item;

    BOR_LIST_FOR_EACH(&m->table[id], item){
        if (m->eq(key1, item, m->data))
            return item;
    }

    return NULL;
}

int ferHMapIDRemove(bor_hmap_t *m, uint32_t id, bor_list_t *key1)
{
    bor_list_t *item;

    item = ferHMapIDGet(m, id, key1);
    if (item){
        ferListDel(item);
        return 0;
    }
    return -1;
}

void ferHMapGather(bor_hmap_t *m, bor_list_t *list)
{
    size_t i;
    bor_list_t *item;

    for (i = 0; i < m->size; i++){
        while (!ferListEmpty(&m->table[i])){
            item = ferListNext(&m->table[i]);
            ferListDel(item);
            ferListAppend(list, item);
        }
    }
}


static int _eq(const bor_list_t *key1, const bor_list_t *key2, void *userdata)
{
    return key1 == key2;
}
