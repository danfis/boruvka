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

#ifndef __FER_HMAP_H__
#define __FER_HMAP_H__

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
typedef uint32_t (*fer_hmap_hash_fn)(bor_list_t *key, void *userdata);

/**
 * Returns true if two given keys are same.
 * If this callback is set to NULL, exact values of keys are compared.
 */
typedef int (*fer_hmap_eq_fn)(const bor_list_t *key1, const bor_list_t *key2,
                              void *userdata);

struct _bor_hmap_t {
    bor_list_t *table;
    size_t size;

    fer_hmap_hash_fn hash;
    fer_hmap_eq_fn eq;
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
bor_hmap_t *ferHMapNew(size_t size,
                       fer_hmap_hash_fn hash_func,
                       fer_hmap_eq_fn eq_func,
                       void *userdata);

/**
 * Deletes table - content of table is not touched.
 */
void ferHMapDel(bor_hmap_t *h);

/**
 * Returns size of hash map
 */
_fer_inline size_t ferHMapSize(const bor_hmap_t *t);

/**
 * Put key into hash map.
 */
_fer_inline void ferHMapPut(bor_hmap_t *m, bor_list_t *key1);

/**
 * Returns a key from hash map that equals to {key1} or NULL if there is no
 * such key.
 *
 * Note that {.eq} callback is used for this and {key1} is used as key1
 * argument.
 */
_fer_inline bor_list_t *ferHMapGet(const bor_hmap_t *m, bor_list_t *key1);

/**
 * Removes key from hash map.
 * Return 0 if such a key was stored in hash map and -1 otherwise.
 */
_fer_inline int ferHMapRemove(bor_hmap_t *m, bor_list_t *key1);

/**
 * Fill given {list} with all elements from hash map.
 * After calling this, hash map will be empty.
 */
void ferHMapGather(bor_hmap_t *m, bor_list_t *list);


/**
 * Returns ID of given key corresponding to the hash map
 */
uint32_t ferHMapID(const bor_hmap_t *m, bor_list_t *key1);

/**
 * Put key with specified id into hash map.
 */
_fer_inline void ferHMapIDPut(bor_hmap_t *m, uint32_t id, bor_list_t *key1);

/**
 * Returns a key from hash map that have given {id} and equals to {key1} or
 * NULL if there is no such key.
 *
 * Note that {.eq} callback is used for this and {key1} is used as key1
 * argument.
 */
bor_list_t *ferHMapIDGet(const bor_hmap_t *m, uint32_t id, bor_list_t *key1);

/**
 * Removes key with given {id} from hash map.
 * Return 0 if such a key was stored in hash map and -1 otherwise.
 */
int ferHMapIDRemove(bor_hmap_t *m, uint32_t id, bor_list_t *key1);

/**** INLINES ****/
_fer_inline size_t ferHMapSize(const bor_hmap_t *t)
{
    return t->size;
}

_fer_inline void ferHMapPut(bor_hmap_t *m, bor_list_t *key1)
{
    uint32_t id;
    id = ferHMapID(m, key1);
    ferHMapIDPut(m, id, key1);
}

_fer_inline bor_list_t *ferHMapGet(const bor_hmap_t *m, bor_list_t *key1)
{
    uint32_t id;
    id = ferHMapID(m, key1);
    return ferHMapIDGet(m, id, key1);
}

_fer_inline int ferHMapRemove(bor_hmap_t *m, bor_list_t *key1)
{
    uint32_t id;
    id = ferHMapID(m, key1);
    return ferHMapIDRemove(m, id, key1);
}

_fer_inline void ferHMapIDPut(bor_hmap_t *m, uint32_t id, bor_list_t *key1)
{
    // put item into table
    ferListAppend(&m->table[id], key1);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_HMAP_H__ */
