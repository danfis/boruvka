/***
 * Boruvka
 * --------
 * Copyright (c)2017 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_BUCKETQ_H__
#define __BOR_BUCKETQ_H__

#include <boruvka/compiler.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Number of buckets available in bucket-based queue.
 * Inserting key greater or equal then this constant will end up in program
 * termination.
 */
#define BOR_BUCKETQ_SIZE 1024

/**
 * Initial size of one bucket in bucket-queue.
 */
#define BOR_BUCKETQ_BUCKET_INIT_SIZE 32

/**
 * Expansion factor of a bucket.
 */
#define BOR_BUCKETQ_BUCKET_EXPANSION_FACTOR 2

/**
 * Bucket for storing values.
 */
struct bor_bucketq_bucket {
    TYPE *value; /*!< Stored values */
    int size;   /*!< Number of stored values */
    int alloc;  /*!< Size of the allocated array */
};
typedef struct bor_bucketq_bucket bor_bucketq_bucket_t;

/**
 * Bucket based priority queue.
 */
struct bor_bucketq {
    bor_bucketq_bucket_t *bucket; /*!< Array of buckets */
    int bucket_size;              /*!< Number of buckets */
    int lowest_key;               /*!< Lowest key so far */
    int size;                     /*!< Number of elements stored in queue */
};
typedef struct bor_bucketq bor_bucketq_t;

/**
 * Initializes priority queue.
 */
void borBucketQInit(bor_bucketq_t *pq);

/**
 * Frees allocated resources.
 */
void borBucketQFree(bor_bucketq_t *pq);

/**
 * Inserts an element into queue.
 * If the key >= BOR_BUCKET_SIZE the program terminates.
 */
void borBucketQPush(bor_bucketq_t *pq, int key, TYPE value);

/**
 * Removes and returns the lowest element.
 * If the queue is empty the program terminates.
 */
TYPE borBucketQPop(bor_bucketq_t *pq, int *key);

/**
 * Returns true if the queue is empty.
 */
_bor_inline int borBucketQIsEmpty(const bor_bucketq_t *pq);

/**** INLINES ****/
_bor_inline int borBucketQIsEmpty(const bor_bucketq_t *pq)
{
    return pq->size == 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_BUCKETQ_H__ */
