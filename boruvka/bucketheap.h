/***
 * Boruvka
 * --------
 * Copyright (c)2014 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.bhp>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_BUCKETHEAP_H__
#define __BOR_BUCKETHEAP_H__

#include <boruvka/core.h>
#include <boruvka/segmarr.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Bucket Based Heap
 * ==================
 *
 */

/** vvvv */

/**
 * Connector to the bucket heap.
 */
typedef bor_list_t bor_bucketheap_node_t;

struct _bor_bucketheap_t {
    bor_segmarr_t *bucket; /*!< Array of buckets, index of bucket is the
                                key value. */
    int bucket_size;       /*!< Number of buckets */
    size_t node_size;      /*!< Number of nodes in the heap */
    int lowest_key;        /*!< Lowest key so far */
};
typedef struct _bor_bucketheap_t bor_bucketheap_t;
/** ^^^^ */


/**
 * Functions
 * ----------
 */

/**
 * Creates new empty Bucket Heap.
 */
bor_bucketheap_t *borBucketHeapNew(void);


/**
 * Deletes bucket heap.
 * Note that individual nodes are not disconnected from heap.
 */
void borBucketHeapDel(bor_bucketheap_t *bh);

/**
 * Returns true if heap is empty.
 */
_bor_inline int borBucketHeapEmpty(const bor_bucketheap_t *bh);

/**
 * Returns minimal node. The nodes with identical key are returned in FIFO
 * order.
 * If the {key} is non NULL it is filled with the corresponding key value.
 */
_bor_inline bor_bucketheap_node_t *borBucketHeapMin(bor_bucketheap_t *bh,
                                                    int *key);

/**
 * Adds node to heap.
 */
void borBucketHeapAdd(bor_bucketheap_t *bh, int key,
                      bor_bucketheap_node_t *n);

/**
 * Removes and returns minimal node from heap.
 */
_bor_inline bor_bucketheap_node_t *borBucketHeapExtractMin(bor_bucketheap_t *bh,
                                                           int *key);

/**
 * Update position of node in heap in case its value was decreased.
 * If value wasn't decreased (or you are not sure) call borBucketHeapUpdate()
 * instead.
 */
_bor_inline void borBucketHeapDecreaseKey(bor_bucketheap_t *bh,
                                          bor_bucketheap_node_t *n,
                                          int new_key);

/**
 * Updates position of node in heap.
 */
_bor_inline void borBucketHeapUpdate(bor_bucketheap_t *bh,
                                     bor_bucketheap_node_t *n,
                                     int new_key);

/**
 * Del node from heap.
 */
_bor_inline void borBucketHeapRemove(bor_bucketheap_t *bh,
                                     bor_bucketheap_node_t *n);


/**** INLINES ****/
_bor_inline int borBucketHeapEmpty(const bor_bucketheap_t *bh)
{
    return bh->node_size == 0;
}

_bor_inline bor_bucketheap_node_t *borBucketHeapMin(bor_bucketheap_t *bh,
                                                    int *key)
{
    bor_list_t *bucket;

    if (borBucketHeapEmpty(bh))
        return NULL;

    bucket = borSegmArrGet(bh->bucket, bh->lowest_key);
    while (borListEmpty(bucket)){
        ++bh->lowest_key;
        bucket = borSegmArrGet(bh->bucket, bh->lowest_key);
    }

    if (key)
        *key = bh->lowest_key;

    return borListNext(bucket);
}

_bor_inline bor_bucketheap_node_t *borBucketHeapExtractMin(bor_bucketheap_t *bh,
                                                           int *key)
{
    bor_bucketheap_node_t *n;

    n = borBucketHeapMin(bh, key);
    borBucketHeapRemove(bh, n);

    return n;
}

_bor_inline void borBucketHeapDecreaseKey(bor_bucketheap_t *bh,
                                          bor_bucketheap_node_t *n,
                                          int key)
{
    borBucketHeapUpdate(bh, n, key);
}

_bor_inline void borBucketHeapUpdate(bor_bucketheap_t *bh,
                                     bor_bucketheap_node_t *n,
                                     int key)
{
    borBucketHeapRemove(bh, n);
    borBucketHeapAdd(bh, key, n);
}

_bor_inline void borBucketHeapRemove(bor_bucketheap_t *bh,
                                     bor_bucketheap_node_t *n)
{
    borListDel(n);
    --bh->node_size;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_BUCKETHEAP_H__ */
