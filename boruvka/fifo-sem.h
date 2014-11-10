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

#ifndef __BOR_FIFO_SEM_H__
#define __BOR_FIFO_SEM_H__

#include <pthread.h>
#include <semaphore.h>
#include <boruvka/fifo.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Concurrent FIFO with semaphores
 * ================================
 *
 */

struct _bor_fifo_sem_t {
    bor_fifo_t fifo;
    pthread_mutex_t lock;
    sem_t full;
    sem_t empty;
};
typedef struct _bor_fifo_sem_t bor_fifo_sem_t;


/**
 * Functions
 * ----------
 */

/**
 * Creates a new sem-fifo with default size of the segment.
 */
bor_fifo_sem_t *borFifoSemNew(size_t el_size);

/**
 * Same as borFifoSemNew(), but size of segment can be provided.
 */
bor_fifo_sem_t *borFifoSemNewSize(size_t el_size, size_t buf_size);

/**
 * Deletes a fifo.
 */
void borFifoSemDel(bor_fifo_sem_t *fifo);

/**
 * In-place initialization of fifo structure
 */
int borFifoSemInit(bor_fifo_sem_t *fifo, size_t el_size);

/**
 * In-place initialization with specified buffer size.
 */
int borFifoSemInitSize(bor_fifo_sem_t *fifo, size_t el_size, size_t buf_size);

/**
 * Frees allocated resources.
 */
void borFifoSemFree(bor_fifo_sem_t *fifo);

/**
 * Returns true if the FIFO is empty.
 */
int borFifoSemEmpty(bor_fifo_sem_t *fifo);

/**
 * Pushes a next element data at the back of the FIFO, the provided data
 * must have size as specified in *New*() function.
 */
void borFifoSemPush(bor_fifo_sem_t *fifo, void *el_data);

/**
 * Removes an element from the front of FIFO and stores it into dst
 * argument, i.e., dst must point to a memory of size at least as
 * specified in *New() function.
 * Returns 0 if an element was removed from the queue and -1 if there is
 * no element in queue.
 */
int borFifoSemPop(bor_fifo_sem_t *fifo, void *dst);

/**
 * Same as borFifoSemPop() but blocks until there is available element.
 */
int borFifoSemPopBlock(bor_fifo_sem_t *fifo, void *dst);

/**
 * Same as borFifoSemPopBlock() but block only for a specified amount of
 * time.
 */
int borFifoSemPopBlockTimeout(bor_fifo_sem_t *fifo, int time_in_ms, void *dst);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_FIFO_SEM_H__ */
