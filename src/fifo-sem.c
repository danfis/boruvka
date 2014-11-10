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

#include <limits.h>
#include <stdio.h>

#include "boruvka/alloc.h"
#include "boruvka/fifo-sem.h"

static int fifoInitSem(bor_fifo_sem_t *fifo)
{
    if (pthread_mutex_init(&fifo->lock, NULL) != 0){
        fprintf(stderr, "Fifo-Sem Error: Could not initialize mutex!\n");
        return -1;
    }

    if (sem_init(&fifo->full, 0, 0) != 0){
        fprintf(stderr, "Fifo-Sem Error: Could not initialize semaphore (full)!\n");
        return -1;
    }

    if (sem_init(&fifo->empty, 0, SEM_VALUE_MAX) != 0){
        fprintf(stderr, "Fifo-Sem Error: Could not initialize semaphore (empty)!\n");
        return -1;
    }

    return 0;
}

bor_fifo_sem_t *borFifoSemNew(size_t el_size)
{
    bor_fifo_sem_t *fifo;
    fifo = BOR_ALLOC(bor_fifo_sem_t);
    if (borFifoSemInit(fifo, el_size) != 0){
        BOR_FREE(fifo);
        return NULL;
    }
    return fifo;
}

bor_fifo_sem_t *borFifoSemNewSize(size_t el_size, size_t buf_size)
{
    bor_fifo_sem_t *fifo;
    fifo = BOR_ALLOC(bor_fifo_sem_t);
    if (borFifoSemInitSize(fifo, el_size, buf_size) != 0){
        BOR_FREE(fifo);
        return NULL;
    }
    return fifo;
}

void borFifoSemDel(bor_fifo_sem_t *fifo)
{
    borFifoSemFree(fifo);
    BOR_FREE(fifo);
}

int borFifoSemInit(bor_fifo_sem_t *fifo, size_t el_size)
{
    borFifoInit(&fifo->fifo, el_size);
    return fifoInitSem(fifo);
}

int borFifoSemInitSize(bor_fifo_sem_t *fifo, size_t el_size, size_t buf_size)
{
    borFifoInitSize(&fifo->fifo, el_size, buf_size);
    return fifoInitSem(fifo);
}

void borFifoSemFree(bor_fifo_sem_t *fifo)
{
    pthread_mutex_destroy(&fifo->lock);
    sem_destroy(&fifo->full);
    sem_destroy(&fifo->empty);
    borFifoFree(&fifo->fifo);
}


int borFifoSemEmpty(bor_fifo_sem_t *fifo)
{
    int empty;

    pthread_mutex_lock(&fifo->lock);
    empty = (fifo->fifo.front == NULL);
    pthread_mutex_unlock(&fifo->lock);
    return empty;
}

void borFifoSemPush(bor_fifo_sem_t *fifo, void *el_data)
{
    // reserve item in queue
    sem_wait(&fifo->empty);

    // add message to the queue
    pthread_mutex_lock(&fifo->lock);
    borFifoPush(&fifo->fifo, el_data);
    pthread_mutex_unlock(&fifo->lock);

    // unblock waiting thread
    sem_post(&fifo->full);
}


_bor_inline int popPost(bor_fifo_sem_t *fifo, void *dst)
{
    void *front;

    // pick up message and remove the message from fifo
    pthread_mutex_lock(&fifo->lock);
    front = borFifoFront(&fifo->fifo);
    memcpy(dst, front, fifo->fifo.el_size);
    borFifoPop(&fifo->fifo);
    pthread_mutex_unlock(&fifo->lock);

    // free room for next message
    sem_post(&fifo->empty);
    return 0;
}

int borFifoSemPop(bor_fifo_sem_t *fifo, void *dst)
{
    // wait for available messages or exit if there is none
    if (sem_trywait(&fifo->full) != 0)
        return -1;
    return popPost(fifo, dst);
}

int borFifoSemPopBlock(bor_fifo_sem_t *fifo, void *dst)
{
    sem_wait(&fifo->full);
    return popPost(fifo, dst);
}

int borFifoSemPopBlockTimeout(bor_fifo_sem_t *fifo, int time_in_ms, void *dst)
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

    if (sem_timedwait(&fifo->full, &tm) != 0)
        return -1;
    return popPost(fifo, dst);
}
