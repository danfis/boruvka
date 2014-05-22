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
    int resizable;
    size_t num_elements;

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
 * Creates a resizable version of hash table.
 */
bor_hmap_t *borHMapNewResizable(bor_hmap_hash_fn hash_func,
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
_bor_inline uint32_t borHMapID(const bor_hmap_t *m, bor_list_t *key1);

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
_bor_inline int borHMapIDRemove(bor_hmap_t *m, uint32_t id, bor_list_t *key1);

/**
 * Tries to resize hash table to the new size.
 */
void borHMapResize(bor_hmap_t *m, size_t size);

/**
 * Returns next prime that no lower than hint.
 */
_bor_inline size_t borHMapNextPrime(size_t hint);


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

_bor_inline uint32_t borHMapID(const bor_hmap_t *m, bor_list_t *key1)
{
    uint32_t id;

    id = m->hash(key1, m->data);
    id = id % m->size;

    return id;
}

_bor_inline void borHMapIDPut(bor_hmap_t *m, uint32_t id, bor_list_t *key1)
{
    size_t size;

    // resize table if necessary
    if (m->resizable && m->num_elements + 1 > m->size){
        size = borHMapNextPrime(m->num_elements + 1);
        if (size > m->size)
            borHMapResize(m, size);
    }

    // put item into table
    borListAppend(&m->table[id], key1);
    ++m->num_elements;
}

_bor_inline int borHMapIDRemove(bor_hmap_t *m, uint32_t id, bor_list_t *key1)
{
    bor_list_t *item;

    item = borHMapIDGet(m, id, key1);
    if (item){
        borListDel(item);
        --m->num_elements;
        return 0;
    }
    return -1;
}

_bor_inline size_t borHMapNextPrime(size_t hint)
{
    static size_t primes[] = {
        5ul,         53ul,         97ul,         193ul,       389ul,
        769ul,       1543ul,       3079ul,       6151ul,      12289ul,
        24593ul,     49157ul,      98317ul,      196613ul,    393241ul,
        786433ul,    1572869ul,    3145739ul,    6291469ul,   12582917ul,
        25165843ul,  50331653ul,   100663319ul,  201326611ul, 402653189ul,
        805306457ul, 1610612741ul, 3221225473ul, 4294967291ul
    };
    static size_t primes_size = sizeof(primes) / sizeof(size_t);

    size_t i;
    for (i = 0; i < primes_size; ++i){
        if (primes[i] >= hint)
            return primes[i];
    }
    return primes[primes_size - 1];
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_HMAP_H__ */
