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

#ifndef __BOR_HMAP_H__
#define __BOR_HMAP_H__

#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * HMap - Hash Table
 * ====================
 *
 * See bor_hmap_t.
 */

/**
 * Example
 * --------
 *
 * TODO
 */


/**
 * Callbacks
 * ----------
 */

/**
 * Hash function.
 */
typedef uint32_t (*bor_hmap_hash_fn)(bor_list_t *key, void *userdata);

/**
 * Returns true if two given keys are same.
 * If this callback is set to NULL, exact values of keys are compared.
 */
typedef int (*bor_hmap_eq_fn)(const bor_list_t *key1, const bor_list_t *key2,
                              void *userdata);

struct _bor_hmap_t {
    bor_list_t *table;
    size_t size;

    bor_hmap_hash_fn hash;
    bor_hmap_eq_fn eq;
    void *data;
};
typedef struct _bor_hmap_t bor_hmap_t;

/**
 * Functions
 * ----------
 */

/**
 * Creates hash table of given size
 */
bor_hmap_t *borHMapNew(size_t size,
                       bor_hmap_hash_fn hash_func,
                       bor_hmap_eq_fn eq_func,
                       void *userdata);

/**
 * Deletes table - content of table is not touched.
 */
void borHMapDel(bor_hmap_t *h);

/**
 * Returns size of hash map
 */
_bor_inline size_t borHMapSize(const bor_hmap_t *t);

/**
 * Put key into hash map.
 */
_bor_inline void borHMapPut(bor_hmap_t *m, bor_list_t *key1);

/**
 * Returns a key from hash map that equals to {key1} or NULL if there is no
 * such key.
 *
 * Note that {.eq} callback is used for this and {key1} is used as key1
 * argument.
 */
_bor_inline bor_list_t *borHMapGet(const bor_hmap_t *m, bor_list_t *key1);

/**
 * Removes key from hash map.
 * Return 0 if such a key was stored in hash map and -1 otherwise.
 */
_bor_inline int borHMapRemove(bor_hmap_t *m, bor_list_t *key1);

/**
 * Fill given {list} with all elements from hash map.
 * After calling this, hash map will be empty.
 */
void borHMapGather(bor_hmap_t *m, bor_list_t *list);


/**
 * Returns ID of given key corresponding to the hash map
 */
uint32_t borHMapID(const bor_hmap_t *m, bor_list_t *key1);

/**
 * Put key with specified id into hash map.
 */
_bor_inline void borHMapIDPut(bor_hmap_t *m, uint32_t id, bor_list_t *key1);

/**
 * Returns a key from hash map that have given {id} and equals to {key1} or
 * NULL if there is no such key.
 *
 * Note that {.eq} callback is used for this and {key1} is used as key1
 * argument.
 */
bor_list_t *borHMapIDGet(const bor_hmap_t *m, uint32_t id, bor_list_t *key1);

/**
 * Removes key with given {id} from hash map.
 * Return 0 if such a key was stored in hash map and -1 otherwise.
 */
int borHMapIDRemove(bor_hmap_t *m, uint32_t id, bor_list_t *key1);

/**** INLINES ****/
_bor_inline size_t borHMapSize(const bor_hmap_t *t)
{
    return t->size;
}

_bor_inline void borHMapPut(bor_hmap_t *m, bor_list_t *key1)
{
    uint32_t id;
    id = borHMapID(m, key1);
    borHMapIDPut(m, id, key1);
}

_bor_inline bor_list_t *borHMapGet(const bor_hmap_t *m, bor_list_t *key1)
{
    uint32_t id;
    id = borHMapID(m, key1);
    return borHMapIDGet(m, id, key1);
}

_bor_inline int borHMapRemove(bor_hmap_t *m, bor_list_t *key1)
{
    uint32_t id;
    id = borHMapID(m, key1);
    return borHMapIDRemove(m, id, key1);
}

_bor_inline void borHMapIDPut(bor_hmap_t *m, uint32_t id, bor_list_t *key1)
{
    // put item into table
    borListAppend(&m->table[id], key1);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_HMAP_H__ */
