/***
 * Boruvka
 * --------
 * Copyright (c)2015 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_RING_QUEUE_H__
#define __BOR_RING_QUEUE_H__

#include <pthread.h>
#include <semaphore.h>
#include <boruvka/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _bor_ring_queue_t {
    void **buf; /*!< Ring buffer */
    int size;   /*!< Size of the buffer */
    int head;   /*!< Pointer to the head */
    int tail;   /*!< Pointer to the tail */
    pthread_mutex_t lock;
    sem_t full;
    sem_t empty;
};
typedef struct _bor_ring_queue_t bor_ring_queue_t;

/**
 * Creates a new ring queue with buffer of the defined size.
 */
void borRingQueueInit(bor_ring_queue_t *q, int ring_size);

/**
 * Deletes ring queue.
 */
void borRingQueueFree(bor_ring_queue_t *q);

/**
 * Push a data pointer to the queue.
 * Returns 0 on success, -1 if the queue is full.
 */
void borRingQueuePush(bor_ring_queue_t *q, void *data);

/**
 * Pop next data element from queue.
 */
void *borRingQueuePop(bor_ring_queue_t *q);

/**
 * Block until a data are not available.
 */
void *borRingQueuePopBlock(bor_ring_queue_t *q);

/**
 * Blocking with timeout.
 */
void *borRingQueuePopBlockTimeout(bor_ring_queue_t *q, int time_in_ms);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_RING_QUEUE_H__ */
