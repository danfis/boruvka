/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#include <fermat/hmap.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>


struct _fer_hmap_el_t {
    fer_list_t list;
    void *key;
};

static int _eq(const fer_list_t *key1, const fer_list_t *key2, void *userdata);

fer_hmap_t *ferHMapNew(size_t size,
                       fer_hmap_hash_fn hash_func,
                       fer_hmap_eq_fn eq_func,
                       void *userdata)
{
    fer_hmap_t *hmap;
    size_t i;

    hmap = FER_ALLOC(fer_hmap_t);
    hmap->table = FER_ALLOC_ARR(fer_list_t, size);
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

void ferHMapPut(fer_hmap_t *m, fer_list_t *key1)
{
    uint32_t id;

    // get hash value
    id = m->hash(key1, m->data);
    id = id % m->size;

    // put item into table
    ferListAppend(&m->table[id], key1);
}

fer_list_t *ferHMapGet(const fer_hmap_t *m, fer_list_t *key1)
{
    uint32_t id;
    fer_list_t *item;

    // get hash value
    id = m->hash(key1, m->data);
    id = id % m->size;

    FER_LIST_FOR_EACH(&m->table[id], item){
        if (m->eq(key1, item, m->data))
            return item;
    }

    return NULL;
}

int ferHMapRemove(fer_hmap_t *m, fer_list_t *key1)
{
    fer_list_t *item;

    item = ferHMapGet(m, key1);
    if (item){
        ferListDel(item);
        return 0;
    }
    return -1;
}


static int _eq(const fer_list_t *key1, const fer_list_t *key2, void *userdata)
{
    return key1 == key2;
}
