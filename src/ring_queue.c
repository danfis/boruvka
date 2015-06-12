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

#include <limits.h>
#include <stdio.h>
#include "boruvka/alloc.h"
#include "boruvka/ring_queue.h"

static int queueInitSem(bor_ring_queue_t *q)
{
    if (pthread_mutex_init(&q->lock, NULL) != 0){
        fprintf(stderr, "Ring Queue Error: Could not initialize mutex!\n");
        return -1;
    }

    if (sem_init(&q->full, 0, 0) != 0){
        fprintf(stderr, "Ring Queue Error: Could not initialize semaphore (full)!\n");
        return -1;
    }

    if (sem_init(&q->empty, 0, SEM_VALUE_MAX) != 0){
        fprintf(stderr, "Ring Queue Error: Could not initialize semaphore (empty)!\n");
        return -1;
    }

    return 0;
}

_bor_inline int queueNextId(int id, int size)
{
    return (id + 1) % size;
}

void borRingQueueInit(bor_ring_queue_t *q, int ring_size)
{
    q->buf = BOR_ALLOC_ARR(void *, ring_size);
    q->size = ring_size;
    q->head = q->tail = 0;
    queueInitSem(q);
}

void borRingQueueFree(bor_ring_queue_t *q)
{
    if (q->buf)
        BOR_FREE(q->buf);
    pthread_mutex_destroy(&q->lock);
    sem_destroy(&q->full);
    sem_destroy(&q->empty);
}

static void queueExtend(bor_ring_queue_t *q)
{
    void **buf;
    int i;

    buf = BOR_ALLOC_ARR(void *, q->size * 2);
    for (i = 0; q->head != q->tail; ++i){
        buf[i] = q->buf[q->head];
        q->head = queueNextId(q->head, q->size);
    }
    BOR_FREE(q->buf);
    q->buf = buf;
    q->head = 0;
    q->tail = i;
    q->size *= 2;
}

void borRingQueuePush(bor_ring_queue_t *q, void *data)
{
    // reserve item in queue
    sem_wait(&q->empty);

    // add message to the queue
    pthread_mutex_lock(&q->lock);
    if (queueNextId(q->tail, q->size) == q->head)
        queueExtend(q);
    q->buf[q->tail] = data;
    q->tail = queueNextId(q->tail, q->size);
    pthread_mutex_unlock(&q->lock);

    // unblock waiting thread
    sem_post(&q->full);
}

_bor_inline void *queuePost(bor_ring_queue_t *q)
{
    void *d = NULL;

    // pick up data pointer and remove it from queue
    pthread_mutex_lock(&q->lock);
    if (q->head != q->tail){
        d = q->buf[q->head];
        q->head = queueNextId(q->head, q->size);
    }
    pthread_mutex_unlock(&q->lock);

    // free room for next data pointer
    sem_post(&q->empty);
    return d;
}

void *borRingQueuePop(bor_ring_queue_t *q)
{
    if (sem_trywait(&q->full) != 0)
        return NULL;
    return queuePost(q);
}

void *borRingQueuePopBlock(bor_ring_queue_t *q)
{
    if (sem_wait(&q->full) != 0)
        return NULL;
    return queuePost(q);
}

void *borRingQueuePopBlockTimeout(bor_ring_queue_t *q, int time_in_ms)
{
    struct timespec tm;
    int sec;
    long nsec;

    sec = time_in_ms / 1000;
    nsec = (time_in_ms % 1000L) * 1000L * 1000L;

    clock_gettime(CLOCK_REALTIME, &tm);
    tm.tv_sec += sec;
    tm.tv_nsec += nsec;
    if (tm.tv_nsec > 1000L * 1000L * 1000L){
        tm.tv_nsec %= 1000L * 1000L * 1000L;
        tm.tv_sec += 1;
    }

    if (sem_timedwait(&q->full, &tm) != 0)
        return NULL;
    return queuePost(q);
}
