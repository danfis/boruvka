/***
 * mabor
 * -------
 * Copyright (c)2017 Daniel Fiser <danfis@danfis.cz>,
 * AIC, Department of Computer Science,
 * Faculty of Electrical Engineering, Czech Technical University in Prague.
 * All rights reserved.
 *
 * This file is part of mabor.
 *
 * Distributed under the OSI-approved BSD License (the "License");
 * see accompanying file BDS-LICENSE for details or see
 * <http://www.opensource.org/licenses/bsd-license.php>.
 *
 * This software is distributed WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the License for more information.
 */

#include <stdio.h>
#include <boruvka/alloc.h>
#include <boruvka/apq.h>

static void borAPQBucketQueueInit(bor_apq_bucket_queue_t *q);
static void borAPQBucketQueueFree(bor_apq_bucket_queue_t *q);
static void borAPQBucketQueuePush(bor_apq_bucket_queue_t *q,
                                  int key, bor_apq_el_t *el);
static bor_apq_el_t *borAPQBucketQueuePop(bor_apq_bucket_queue_t *q, int *key);
static void borAPQBucketQueueUpdate(bor_apq_bucket_queue_t *q,
                                    int key, bor_apq_el_t *el);
/** Convets bucket queue to heap queue */
static void borAPQBucketQueueToHeapQueue(bor_apq_bucket_queue_t *b,
                                         bor_apq_heap_queue_t *h);

static void borAPQHeapQueueInit(bor_apq_heap_queue_t *q);
static void borAPQHeapQueueFree(bor_apq_heap_queue_t *q);
static void borAPQHeapQueuePush(bor_apq_heap_queue_t *q,
                                int key, bor_apq_el_t *el);
static bor_apq_el_t *borAPQHeapQueuePop(bor_apq_heap_queue_t *q, int *key);
static void borAPQHeapQueueUpdate(bor_apq_heap_queue_t *q,
                                  int key, bor_apq_el_t *el);

void borAPQInit(bor_apq_t *q)
{
    borAPQBucketQueueInit(&q->bucket_queue);
    q->bucket = 1;
}

void borAPQFree(bor_apq_t *q)
{
    if (q->bucket){
        borAPQBucketQueueFree(&q->bucket_queue);
    }else{
        borAPQHeapQueueFree(&q->heap_queue);
    }
}

void borAPQPush(bor_apq_t *q, int key, bor_apq_el_t *el)
{
    if (q->bucket){
        if (key >= BOR_APQ_BUCKET_SIZE){
            borAPQHeapQueueInit(&q->heap_queue);
            borAPQBucketQueueToHeapQueue(&q->bucket_queue, &q->heap_queue);
            borAPQBucketQueueFree(&q->bucket_queue);
            q->bucket = 0;
            borAPQHeapQueuePush(&q->heap_queue, key, el);
        }else{
            borAPQBucketQueuePush(&q->bucket_queue, key, el);
        }
    }else{
        borAPQHeapQueuePush(&q->heap_queue, key, el);
    }
}

bor_apq_el_t *borAPQPop(bor_apq_t *q, int *key)
{
    if (q->bucket){
        return borAPQBucketQueuePop(&q->bucket_queue, key);
    }else{
        return borAPQHeapQueuePop(&q->heap_queue, key);
    }
}

void borAPQUpdate(bor_apq_t *q, int key, bor_apq_el_t *el)
{
    if (q->bucket){
        borAPQBucketQueueUpdate(&q->bucket_queue, key, el);
    }else{
        borAPQHeapQueueUpdate(&q->heap_queue, key, el);
    }
}

static void borAPQBucketQueueInit(bor_apq_bucket_queue_t *q)
{
    q->bucket_size = BOR_APQ_BUCKET_SIZE;
    q->bucket = BOR_CALLOC_ARR(bor_apq_bucket_t, q->bucket_size);
    q->lowest_key = q->bucket_size;
    q->size = 0;
}

static void borAPQBucketQueueFree(bor_apq_bucket_queue_t *q)
{
    int i;

    for (i = 0; i < q->bucket_size; ++i){
        if (q->bucket[i].el)
            BOR_FREE(q->bucket[i].el);
    }
    BOR_FREE(q->bucket);
}

static void borAPQBucketQueuePush(bor_apq_bucket_queue_t *q,
                                  int key, bor_apq_el_t *el)
{
    bor_apq_bucket_t *bucket;

    if (key >= BOR_APQ_BUCKET_SIZE){
        fprintf(stderr, "Error: borAPQBucketQueue: key %d is over a size of"
                        " the bucket queue, which is %d.",
                        key, BOR_APQ_BUCKET_SIZE);
        exit(-1);
    }

    bucket = q->bucket + key;
    if (bucket->size == bucket->alloc){
        if (bucket->alloc == 0){
            bucket->alloc = BOR_APQ_BUCKET_INIT_SIZE;
        }else{
            bucket->alloc *= BOR_APQ_BUCKET_EXPANSION_FACTOR;
        }
        bucket->el = BOR_REALLOC_ARR(bucket->el, bor_apq_el_t *,
                                     bucket->alloc);

    }
    el->key = key;
    el->conn.bucket = bucket->size;
    bucket->el[bucket->size++] = el;
    ++q->size;

    if (key < q->lowest_key)
        q->lowest_key = key;
}

static bor_apq_el_t *borAPQBucketQueuePop(bor_apq_bucket_queue_t *q, int *key)
{
    bor_apq_bucket_t *bucket;
    bor_apq_el_t *el;

    if (q->size == 0)
        return NULL;

    bucket = q->bucket + q->lowest_key;
    while (bucket->size == 0){
        ++q->lowest_key;
        bucket += 1;
    }

    el = bucket->el[--bucket->size];
    if (key)
        *key = q->lowest_key;
    --q->size;
    return el;
}

static void borAPQBucketQueueUpdate(bor_apq_bucket_queue_t *q,
                                    int key, bor_apq_el_t *el)
{
    bor_apq_bucket_t *bucket;

    bucket = q->bucket + el->key;
    bucket->el[el->conn.bucket] = bucket->el[--bucket->size];
    bucket->el[el->conn.bucket]->conn.bucket = el->conn.bucket;
    --q->size;
    if (q->size == 0)
        q->lowest_key = q->bucket_size;
    borAPQBucketQueuePush(q, key, el);
}

static void borAPQBucketQueueToHeapQueue(bor_apq_bucket_queue_t *b,
                                         bor_apq_heap_queue_t *h)
{
    bor_apq_bucket_t *bucket;
    int i, j;

    for (i = b->lowest_key; i < b->bucket_size; ++i){
        bucket = b->bucket + i;
        for (j = 0; j < bucket->size; ++j){
            borAPQHeapQueuePush(h, i, bucket->el[j]);
        }
        if (bucket->el != NULL)
            BOR_FREE(bucket->el);
        bucket->el = NULL;
        bucket->size = bucket->alloc = 0;
    }
    b->size = 0;
}


static int heapLT(const bor_pairheap_node_t *_n1,
                  const bor_pairheap_node_t *_n2, void *_)
{
    bor_apq_el_t *e1 = bor_container_of(_n1, bor_apq_el_t, conn.heap);
    bor_apq_el_t *e2 = bor_container_of(_n2, bor_apq_el_t, conn.heap);
    return e1->key <= e2->key;
}

static void borAPQHeapQueueInit(bor_apq_heap_queue_t *q)
{
    q->heap = borPairHeapNew(heapLT, NULL);
}

static void borAPQHeapQueueFree(bor_apq_heap_queue_t *q)
{
    borPairHeapDel(q->heap);
}

static void borAPQHeapQueuePush(bor_apq_heap_queue_t *q,
                                int key, bor_apq_el_t *el)
{
    el->key = key;
    borPairHeapAdd(q->heap, &el->conn.heap);
}

static bor_apq_el_t *borAPQHeapQueuePop(bor_apq_heap_queue_t *q, int *key)
{
    bor_pairheap_node_t *hn;
    bor_apq_el_t *el;

    hn = borPairHeapExtractMin(q->heap);
    el = bor_container_of(hn, bor_apq_el_t, conn.heap);
    if (key)
        *key = el->key;
    return el;
}

static void borAPQHeapQueueUpdate(bor_apq_heap_queue_t *q,
                                  int key, bor_apq_el_t *el)
{
    el->key = key;
    borPairHeapUpdate(q->heap, &el->conn.heap);
}
