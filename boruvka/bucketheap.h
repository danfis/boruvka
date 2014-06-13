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
typedef bor_list_t bor_bucketheap_node_t;

/**
 * Callback that returns the key under which the node should be stored.
 */
typedef unsigned long (*bor_bucketheap_key)(const bor_bucketheap_node_t *node,
                                            void *data);

struct _bor_bucketheap_t {
    bor_segmarr_t *bucket;     /*!< Array of buckets, index of bucket is
                                    the key value. */
    unsigned long bucket_size; /*!< Number of buckets */
    size_t node_size;          /*!< Number of nodes in the heap */
    unsigned long lowest_key;  /*!< Lowest key so far */

    bor_bucketheap_key key;    /*!< Key callback */
    void *data;                /*!< Data for the .key callback */
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
bor_bucketheap_t *borBucketHeapNew(bor_bucketheap_key key, void *data);


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
 * Returns minimal node.
 */
_bor_inline bor_bucketheap_node_t *borBucketHeapMin(bor_bucketheap_t *bh);

/**
 * Adds node to heap.
 */
void borBucketHeapAdd(bor_bucketheap_t *bh, bor_bucketheap_node_t *n);

/**
 * Removes and returns minimal node from heap.
 */
_bor_inline bor_bucketheap_node_t *borBucketHeapExtractMin(bor_bucketheap_t *bh);

/**
 * Update position of node in heap in case its value was decreased.
 * If value wasn't decreased (or you are not sure) call borBucketHeapUpdate()
 * instead.
 */
_bor_inline void borBucketHeapDecreaseKey(bor_bucketheap_t *bh,
                                          bor_bucketheap_node_t *n);

/**
 * Updates position of node in heap.
 */
_bor_inline void borBucketHeapUpdate(bor_bucketheap_t *bh,
                                     bor_bucketheap_node_t *n);

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

_bor_inline bor_bucketheap_node_t *borBucketHeapMin(bor_bucketheap_t *bh)
{
    bor_list_t *bucket;

    if (borBucketHeapEmpty(bh))
        return NULL;

    bucket = borSegmArrGet(bh->bucket, bh->lowest_key);
    while (borListEmpty(bucket)){
        ++bh->lowest_key;
        bucket = borSegmArrGet(bh->bucket, bh->lowest_key);
    }

    return borListPrev(bucket);
}

_bor_inline bor_bucketheap_node_t *borBucketHeapExtractMin(bor_bucketheap_t *bh)
{
    bor_bucketheap_node_t *n;

    n = borBucketHeapMin(bh);
    borBucketHeapRemove(bh, n);

    return n;
}

_bor_inline void borBucketHeapDecreaseKey(bor_bucketheap_t *bh,
                                          bor_bucketheap_node_t *n)
{
    borBucketHeapUpdate(bh, n);
}

_bor_inline void borBucketHeapUpdate(bor_bucketheap_t *bh,
                                     bor_bucketheap_node_t *n)
{
    borBucketHeapRemove(bh, n);
    borBucketHeapAdd(bh, n);
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
