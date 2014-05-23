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

#ifndef __BOR_HASH_TABLE_H__
#define __BOR_HASH_TABLE_H__

#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Hash Table
 * ====================
 *
 * See bor_hash_table_t.
 */


/**
 * Definition of the hash key type.
 */
typedef uint64_t bor_htable_key_t;

/**
 * Callbacks
 * ----------
 */

/**
 * Hash function.
 */
typedef bor_htable_key_t (*bor_htable_hash_fn)(const bor_list_t *key,
                                               void *userdata);

/**
 * Returns true if two given keys are same.
 * If this callback is set to NULL, exact values of keys are compared.
 */
typedef int (*bor_htable_eq_fn)(const bor_list_t *key1, const bor_list_t *key2,
                                void *userdata);


/**
 * Hash table structure.
 */
struct _bor_htable_t {
    bor_list_t *table;
    size_t size;
    size_t num_elements;

    bor_htable_hash_fn hash;
    bor_htable_eq_fn eq;
    void *data;
};
typedef struct _bor_htable_t bor_htable_t;

/**
 * Functions
 * ----------
 */

/**
 * Creates hash table
 */
bor_htable_t *borHTableNew(bor_htable_hash_fn hash_func,
                           bor_htable_eq_fn eq_func,
                           void *userdata);

/**
 * Deletes a table.
 * Content of the given table is not touched, i.e., it is responsibility of
 * a caller to free inserted elements.
 */
void borHTableDel(bor_htable_t *h);

/**
 * Returns size of hash map
 */
_bor_inline size_t borHTableSize(const bor_htable_t *t);

/**
 * Returns number of elements stored in hash table.
 */
_bor_inline size_t borHTableNumElements(const bor_htable_t *t);

/**
 * Fill given {list} with all elements from hash table.
 * After calling this, hash table will be empty.
 */
void borHTableGather(bor_htable_t *m, bor_list_t *list);

/**
 * Insert an element into the hash table.
 */
_bor_inline void borHTableInsert(bor_htable_t *m, bor_list_t *key1);

/**
 * Insert an element into the hash table only if the same element isn't
 * already there.
 * Returns the equal element if already on hash table or NULL the given
 * element was inserted.
 */
_bor_inline bor_list_t *borHTableInsertUnique(bor_htable_t *m,
                                              bor_list_t *key1);

/**
 * Same as borHTableInsert() but does not resize hash table.
 */
_bor_inline void borHTableInsertNoResize(bor_htable_t *m, bor_list_t *key1);

/**
 * Insert the key into the specified bucket.
 * The bucket must be smaller then the size of the table.
 */
_bor_inline void borHTableInsertBucket(bor_htable_t *m, size_t bucket,
                                       bor_list_t *key1);

/**
 * Same as borHTableInsertBucket() but does not resize hash table.
 */
_bor_inline void borHTableInsertBucketNoResize(bor_htable_t *m,
                                               size_t bucket,
                                               bor_list_t *key1);

/**
 * Removes an element from the hash table.
 * Return 0 if such a key was stored in hash table and -1 otherwise.
 */
_bor_inline int borHTableErase(bor_htable_t *m, bor_list_t *key1);

/**
 * Removes an element from the specified bucket.
 * Return 0 if such an element was stored in the hash table and -1
 * otherwise.
 */
_bor_inline int borHTableEraseBucket(bor_htable_t *m, size_t bucket,
                                     bor_list_t *key1);

/**
 * Returns a key from hash table that equals to {key1} or NULL if there is no
 * such key. The first found element is returned.
 *
 * Note that {.eq} callback is used for this and {key1} is used as key1
 * argument.
 */
_bor_inline bor_list_t *borHTableFind(const bor_htable_t *m,
                                      const bor_list_t *key1);

/**
 * Same as borHTableFind() but searches only the specified bucket.
 */
_bor_inline bor_list_t *borHTableFindBucket(const bor_htable_t *m,
                                            size_t bucket,
                                            const bor_list_t *key1);

/**
 * Searches for all elements that are equal to the provided.
 * The elements from the hash table are stored in out_arr array which can
 * be provided in two ways.
 * 1. Either preallocated array can be provided and then the size argument
 *    must equal to the size of that array (and thus the maximal number of
 *    elements that can be stored in the array).
 * 2. Or *out_arr equals to NULL and in that case the function allocates
 *    enough memory for the array and stores its size in size argument.
 *    Then it is the caller's responsibility to call BOR_FREE() on the
 *    *out_arr array.
 *
 * Returns number of elements were found (regardless of number of elements
 * that were actually stored in output array).
 */
size_t borHTableFindAll(const bor_htable_t *m, const bor_list_t *key1,
                        bor_list_t ***out_arr, size_t *size);


/**
 * Returns a bucket number corresponding to the given element.
 */
_bor_inline size_t borHTableBucket(const bor_htable_t *m,
                                   const bor_list_t *key1);

/**
 * Resize hash table to the specifed size.
 */
void borHTableResize(bor_htable_t *m, size_t size);

/**
 * Returns next prime suitable for the hash table size that is not lower
 * than hint.
 */
_bor_inline size_t borHTableNextPrime(size_t hint);


/**** INLINES ****/
_bor_inline size_t borHTableSize(const bor_htable_t *t)
{
    return t->size;
}

_bor_inline size_t borHTableNumElements(const bor_htable_t *t)
{
    return t->num_elements;
}

_bor_inline void borHTableInsert(bor_htable_t *m, bor_list_t *key1)
{
    size_t bucket;
    bucket = borHTableBucket(m, key1);
    borHTableInsertBucket(m, bucket, key1);
}

_bor_inline bor_list_t *borHTableInsertUnique(bor_htable_t *m,
                                              bor_list_t *key1)
{
    size_t bucket;
    bor_list_t *item;

    bucket = borHTableBucket(m, key1);
    item = borHTableFindBucket(m, bucket, key1);
    if (item == NULL){
        borHTableInsertBucket(m, bucket, key1);
    }

    return item;
}

_bor_inline void borHTableInsertNoResize(bor_htable_t *m, bor_list_t *key1)
{
    size_t bucket;
    bucket = borHTableBucket(m, key1);
    borHTableInsertBucketNoResize(m, bucket, key1);
}

_bor_inline void borHTableInsertBucket(bor_htable_t *m, size_t bucket,
                                       bor_list_t *key1)
{
    size_t size;

    // resize table if necessary
    if (m->num_elements + 1 > m->size){
        size = borHTableNextPrime(m->num_elements + 1);
        if (size > m->size){
            borHTableResize(m, size);

            // re-compute bucket id because of resize
            bucket = borHTableBucket(m, key1);
        }
    }

    // put item into table
    borHTableInsertBucketNoResize(m, bucket, key1);
}

_bor_inline void borHTableInsertBucketNoResize(bor_htable_t *m,
                                               size_t bucket,
                                               bor_list_t *key1)
{
    borListAppend(&m->table[bucket], key1);
    ++m->num_elements;
}

_bor_inline int borHTableErase(bor_htable_t *m, bor_list_t *key1)
{
    size_t bucket;
    bucket = borHTableBucket(m, key1);
    return borHTableEraseBucket(m, bucket, key1);
}

_bor_inline int borHTableEraseBucket(bor_htable_t *m, size_t bucket,
                                     bor_list_t *key1)
{
    bor_list_t *item;

    item = borHTableFindBucket(m, bucket, key1);
    if (item){
        borListDel(item);
        --m->num_elements;
        return 0;
    }
    return -1;
}

_bor_inline bor_list_t *borHTableFind(const bor_htable_t *m,
                                      const bor_list_t *key1)
{
    size_t bucket;
    bucket = borHTableBucket(m, key1);
    return borHTableFindBucket(m, bucket, key1);
}

_bor_inline bor_list_t *borHTableFindBucket(const bor_htable_t *m,
                                            size_t bucket,
                                            const bor_list_t *key1)
{
    bor_list_t *item;

    BOR_LIST_FOR_EACH(&m->table[bucket], item){
        if (m->eq(key1, item, m->data))
            return item;
    }

    return NULL;
}






_bor_inline size_t borHTableBucket(const bor_htable_t *m,
                                   const bor_list_t *key1)
{
    return m->hash(key1, m->data) % (bor_htable_key_t)m->size;
}

_bor_inline size_t borHTableNextPrime(size_t hint)
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

#endif /* __BOR_HASH_TABLE_H__ */
