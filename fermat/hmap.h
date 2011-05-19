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

#ifndef __FER_HMAP_H__
#define __FER_HMAP_H__

#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * HMap - Hash Table
 * ====================
 *
 * See fer_hmap_t.
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
typedef uint32_t (*fer_hmap_hash_fn)(fer_list_t *key, void *userdata);

/**
 * Returns true if two given keys are same.
 * If this callback is set to NULL, exact values of keys are compared.
 */
typedef int (*fer_hmap_eq_fn)(const fer_list_t *key1, const fer_list_t *key2,
                              void *userdata);

struct _fer_hmap_t {
    fer_list_t *table;
    size_t size;

    fer_hmap_hash_fn hash;
    fer_hmap_eq_fn eq;
    void *data;
};
typedef struct _fer_hmap_t fer_hmap_t;

/**
 * Functions
 * ----------
 */

/**
 * Creates hash table of given size
 */
fer_hmap_t *ferHMapNew(size_t size,
                       fer_hmap_hash_fn hash_func,
                       fer_hmap_eq_fn eq_func,
                       void *userdata);

/**
 * Deletes table - content of table is not touched.
 */
void ferHMapDel(fer_hmap_t *h);

/**
 * Returns size of hash map
 */
_fer_inline size_t ferHMapSize(const fer_hmap_t *t);

/**
 * Put key into hash map.
 */
void ferHMapPut(fer_hmap_t *m, fer_list_t *key1);

/**
 * Returns a key from hash map that equals to {key1} or NULL if there is no
 * such key.
 *
 * Note that {.eq} callback is used for this and {key1} is used as key1
 * argument.
 */
fer_list_t *ferHMapGet(const fer_hmap_t *m, fer_list_t *key1);

/**
 * Removes key from hash map.
 * Return 0 if such a key was stored in hash map and -1 otherwise.
 */
int ferHMapRemove(fer_hmap_t *m, fer_list_t *key1);

/**
 * Fill given {list} with all elements from hash map.
 * After calling this, hash map will be empty.
 */
void ferHMapGather(fer_hmap_t *m, fer_list_t *list);


/**** INLINES ****/
_fer_inline size_t ferHMapSize(const fer_hmap_t *t)
{
    return t->size;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_HMAP_H__ */
