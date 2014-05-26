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

#ifndef __BOR_SEGM_ARR_H__
#define __BOR_SEGM_ARR_H__

#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Segmented Array
 * ================
 *
 * Segmented array is an array that is allocated as a list of fixed size
 * segments. The goal of this concept is to have extensible array that does
 * not need to copy back and forth data when reallocating memory.
 *
 * See bor_segmarr_t.
 */


struct _bor_segmarr_t {
    size_t el_size;       /*!< Size of a single element in bytes. */
    size_t segm_size;     /*!< Size of one segment in bytes */
    size_t els_per_segm;  /*!< Size of one segment as number of elements it
                               can hold. */
    char **segm;          /*!< Array of segments. */
    size_t num_segm;      /*!< Number of segments. */
    size_t alloc_segm;    /*!< Number of actually allocated segment slots */
};
typedef struct _bor_segmarr_t bor_segmarr_t;

/**
 * Creates and initializes a new segmented array.
 * The parameter {el_size} is a size of a single emelent that will be stored
 * in the array, the {segment_size} parameter is size of one segment.
 */
bor_segmarr_t *borSegmArrNew(size_t el_size, size_t segment_size);

/**
 * Frees all allocated memory of segmented array.
 */
void borSegmArrDel(bor_segmarr_t *arr);

/**
 * Returns a pointer to the i'th element from the array.
 * If the requested element is too big to satisfy with the current array,
 * the array is expanded.
 */
_bor_inline void *borSegmArrGet(bor_segmarr_t *arr, size_t i);

/**
 * Allocates additional segments to have num_segs overall.
 */
void borSegmArrExpandSegments(bor_segmarr_t *arr, size_t num_segs);

/**** INLINES ****/
_bor_inline void *borSegmArrGet(bor_segmarr_t *arr, size_t i)
{
    size_t segm_id = i / arr->els_per_segm;
    size_t offset  = (i % arr->els_per_segm) * arr->el_size;

    if (segm_id >= arr->num_segm)
        borSegmArrExpandSegments(arr, segm_id + 1);

    return &arr->segm[segm_id][offset];
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_SEGM_ARR_H__ */

