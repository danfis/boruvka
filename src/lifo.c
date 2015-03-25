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

#include "boruvka/alloc.h"
#include "boruvka/lifo.h"

/** Just resets all pointers to NULL */
_bor_inline void lifoReset(bor_lifo_t *lifo);
/** Resest cursors in for last segment */
_bor_inline void lifoResetBack(bor_lifo_t *lifo);
/** Adds one segment to the end */
_bor_inline void lifoAddSegment(bor_lifo_t *lifo);

bor_lifo_t *borLifoNew(size_t el_size)
{
    return borLifoNewSize(el_size, BOR_LIFO_BUF_SIZE);
}

bor_lifo_t *borLifoNewSize(size_t el_size, size_t buf_size)
{
    bor_lifo_t *lifo;

    lifo = BOR_ALLOC(bor_lifo_t);
    borLifoInitSize(lifo, el_size, buf_size);

    return lifo;
}

void borLifoDel(bor_lifo_t *lifo)
{
    borLifoFree(lifo);
    BOR_FREE(lifo);
}

void borLifoInit(bor_lifo_t *lifo, size_t el_size)
{
    borLifoInitSize(lifo, el_size, BOR_LIFO_BUF_SIZE);
}

void borLifoInitSize(bor_lifo_t *lifo, size_t el_size, size_t buf_size)
{
    lifo->segm_size = buf_size;
    lifo->el_size   = el_size;
    lifo->data_size = lifo->segm_size - sizeof(bor_lifo_segm_t);
    lifo->data_size = (lifo->data_size / el_size) * el_size;
    lifoReset(lifo);
}

void borLifoFree(bor_lifo_t *lifo)
{
    borLifoClear(lifo);
}

void borLifoClear(bor_lifo_t *lifo)
{
    bor_lifo_segm_t *prev;

    while (lifo->back){
        prev = lifo->back->prev;
        BOR_FREE(lifo->back);
        lifo->back = prev;
    }
    lifoReset(lifo);
}

void borLifoPush(bor_lifo_t *lifo, void *el_data)
{
    if (lifo->back == NULL){
        // LIFO is empty, create a first segment
        lifoAddSegment(lifo);

    }else{
        // Shift to next element
        lifo->cur += lifo->el_size;

        if (lifo->cur + lifo->el_size > lifo->data_end){
            // LIFO is full, next segment must be created
            lifoAddSegment(lifo);
        }
    }

    // Copy data to the current position
    memcpy(lifo->cur, el_data, lifo->el_size);
}

void borLifoPop(bor_lifo_t *lifo)
{
    bor_lifo_segm_t *prev;

    if (!lifo->back)
        return;

    // Notice that we are not really removing an element we just move the
    // pointer and remove only the whole segment if necessary
    lifo->cur -= lifo->el_size;

    // Remove last segment if this is the last element popped.
    if (lifo->cur < lifo->data_begin){
        prev = lifo->back->prev;
        BOR_FREE(lifo->back);
        lifo->back = prev;

        if (lifo->back == NULL){
            // Remove the last segment, lifo is empty
            lifoReset(lifo);

        }else{
            lifoResetBack(lifo);
            lifo->cur = lifo->data_end - lifo->el_size;
        }
    }
}

_bor_inline void lifoReset(bor_lifo_t *lifo)
{
    lifo->back = NULL;
    lifo->cur = lifo->data_begin = lifo->data_end = NULL;
}

_bor_inline void lifoResetBack(bor_lifo_t *lifo)
{
    lifo->data_begin = ((char *)lifo->back) + sizeof(bor_lifo_segm_t);
    lifo->data_end   = lifo->data_begin + lifo->data_size;
    lifo->cur        = lifo->data_begin;
}

_bor_inline void lifoAddSegment(bor_lifo_t *lifo)
{
    bor_lifo_segm_t *segm;

    segm = (bor_lifo_segm_t *)BOR_MALLOC(lifo->segm_size);
    segm->prev = lifo->back;
    lifo->back = segm;
    lifoResetBack(lifo);
}
