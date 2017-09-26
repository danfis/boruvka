/***
 * boruvka
 * -------
 * Copyright (c)2017 Daniel Fiser <danfis@danfis.cz>,
 * AIC, Department of Computer Science,
 * Faculty of Electrical Engineering, Czech Technical University in Prague.
 * All rights reserved.
 *
 * This file is part of boruvka.
 *
 * Distributed under the OSI-approved BSD License (the "License");
 * see accompanying file BDS-LICENSE for details or see
 * <http://www.opensource.org/licenses/bsd-license.php>.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#ifndef __BOR_APQ_H__
#define __BOR_APQ_H__

#include <boruvka/pairheap.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Adaptive Priority Queue
 * ========================
 */

/**
 * Number of buckets available in bucket-based queue.
 * Inserting key greater or equal then this constant will end up in program
 * termination.
 */
#define BOR_APQ_BUCKET_SIZE 1024

/**
 * Initial size of one bucket in bucket-queue.
 */
#define BOR_APQ_BUCKET_INIT_SIZE 32

/**
 * Expansion factor of a bucket.
 */
#define BOR_APQ_BUCKET_EXPANSION_FACTOR 2

struct _bor_apq_el_t {
    int key;
    union {
        int bucket;
        bor_pairheap_node_t heap;
    } conn;
};
typedef struct _bor_apq_el_t bor_apq_el_t;

/**
 * Bucket for storing int values.
 */
struct _bor_apq_bucket_t {
    bor_apq_el_t **el; /*!< Stored elements */
    int size;   /*!< Number of stored values */
    int alloc;  /*!< Size of the allocated array */
};
typedef struct _bor_apq_bucket_t bor_apq_bucket_t;

/**
 * Bucket based priority queue.
 */
struct _bor_apq_bucket_queue_t {
    bor_apq_bucket_t *bucket; /*!< Array of buckets */
    int bucket_size;          /*!< Number of buckets */
    int lowest_key;           /*!< Lowest key so far */
    int size;                 /*!< Number of elements stored in queue */
};
typedef struct _bor_apq_bucket_queue_t bor_apq_bucket_queue_t;

/**
 * Heap-based priority queue.
 */
struct _bor_apq_heap_queue_t {
    bor_pairheap_t *heap;
};
typedef struct _bor_apq_heap_queue_t bor_apq_heap_queue_t;


struct _bor_apq_t {
    bor_apq_bucket_queue_t bucket_queue;
    bor_apq_heap_queue_t heap_queue;
    int bucket;
};
typedef struct _bor_apq_t bor_apq_t;

/**
 * Initializes priority queue.
 */
void borAPQInit(bor_apq_t *q);

/**
 * Frees allocated resources.
 */
void borAPQFree(bor_apq_t *q);

/**
 * Inserts an element into queue.
 */
void borAPQPush(bor_apq_t *q, int key, bor_apq_el_t *el);

/**
 * Removes and returns the lowest element.
 */
bor_apq_el_t *borAPQPop(bor_apq_t *q, int *key);

/**
 * Update the element if it is already in the heap.
 */
void borAPQUpdate(bor_apq_t *q, int new_key, bor_apq_el_t *el);

/**
 * Returns true if the queue is empty.
 */
_bor_inline int borAPQIsEmpty(const bor_apq_t *q);



/**** INLINES ****/
_bor_inline int borAPQBucketQueueIsEmpty(const bor_apq_bucket_queue_t *q)
{
    return q->size == 0;
}

_bor_inline int borAPQHeapQueueIsEmpty(const bor_apq_heap_queue_t *q)
{
    return borPairHeapEmpty(q->heap);
}

_bor_inline int borAPQIsEmpty(const bor_apq_t *q)
{
    if (q->bucket){
        return borAPQBucketQueueIsEmpty(&q->bucket_queue);
    }else{
        return borAPQHeapQueueIsEmpty(&q->heap_queue);
    }
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_APQ_H__ */
