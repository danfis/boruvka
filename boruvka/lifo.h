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

#ifndef __BOR_LIFO_H__
#define __BOR_LIFO_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * LIFO -- Last In First Out Queue
 * ================================
 *
 */

/**
 * Default size of the buffer used for one segment.
 */
#define BOR_LIFO_BUF_SIZE 512


/** vvvv */
struct _bor_lifo_segm_t {
    struct _bor_lifo_segm_t *prev; /*!< Previous segment */
};
typedef struct _bor_lifo_segm_t bor_lifo_segm_t;

struct _bor_lifo_t {
    size_t segm_size;      /*!< Size of a single segment */
    size_t el_size;        /*!< Element size */
    size_t data_size;      /*!< Size of data part of segment */
    bor_lifo_segm_t *back; /*!< The Last segment */
    char *cur;             /*!< Cursor pointing to the last element */
    char *data_begin;      /*!< Start of the current data segment */
    char *data_end;        /*!< End of the current data segment */
};
typedef struct _bor_lifo_t bor_lifo_t;
/** ^^^^ */


/**
 * Functions
 * ----------
 */

/**
 * Creates a new LIFO structure with default size of the segment buffer.
 */
bor_lifo_t *borLifoNew(size_t el_size);

/**
 * Creates a LIFO structe with specified buffer size.
 */
bor_lifo_t *borLifoNewSize(size_t el_size, size_t buf_size);

/**
 * Deletes a lifo.
 */
void borLifoDel(bor_lifo_t *lifo);

/**
 * In-place initialization of lifo structure
 */
void borLifoInit(bor_lifo_t *lifo, size_t el_size);

/**
 * In-place initialization with specified buffer size.
 */
void borLifoInitSize(bor_lifo_t *lifo, size_t el_size, size_t buf_size);

/**
 * Frees allocated resources.
 */
void borLifoFree(bor_lifo_t *lifo);

/**
 * Returns true if the FIFO is empty.
 */
_bor_inline int borLifoEmpty(const bor_lifo_t *lifo);

/**
 * Clears the LIFO queue. After this the LIFO will be empty.
 */
void borLifoClear(bor_lifo_t *lifo);

/**
 * Pushes a next element data at the back of the FIFO, the provided data
 * must have size as specified in *New*() function.
 */
void borLifoPush(bor_lifo_t *lifo, void *el_data);

/**
 * Removes an element from the back of LIFO.
 */
void borLifoPop(bor_lifo_t *lifo);

/**
 * Returns pointer to the last element from LIFO.
 */
_bor_inline void *borLifoBack(bor_lifo_t *lifo);


/**** INLINES ****/
_bor_inline int borLifoEmpty(const bor_lifo_t *lifo)
{
    return lifo->back == NULL;
}

_bor_inline void *borLifoBack(bor_lifo_t *lifo)
{
    return lifo->cur;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_LIFO_H__ */
