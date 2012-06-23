/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_NN_LINEAR_H__
#define __BOR_NN_LINEAR_H__

#include <boruvka/list.h>
#include <boruvka/vec.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Linear Nearest Neighbor Search
 * ===============================
 *
 * See bor_nn_linear_t.
 */

/**
 * Parameters
 * -----------
 *
 * See bor_nn_linear_params_t.
 */

/** vvvv */

/**
 * Returns distance between two {d}-dimensional vectors
 */
typedef bor_real_t (*bor_nn_linear_dist)(int d,
                                         const bor_vec_t *v1,
                                         const bor_vec_t *v2, void *data);

/** ^^^^ */

struct _bor_nn_linear_params_t {
    int dim;                 /*!< Dimension of space. Default: 2 */
    bor_nn_linear_dist dist; /*!< Callback for distance measurement.
                                  Default: L2 norm distance */
    void *dist_data;         /*!< User-defined data for .dist. Default: NULL */
};
typedef struct _bor_nn_linear_params_t bor_nn_linear_params_t;

/**
 * Initialize parameters to default values.
 */
void borNNLinearParamsInit(bor_nn_linear_params_t *p);


struct _bor_nn_linear_t {
    uint8_t type;
    bor_list_t list; /*!< List of all elements */
    bor_nn_linear_params_t params;
};
typedef struct _bor_nn_linear_t bor_nn_linear_t;


/**
 * User structure
 * ---------------
 *
 * TODO: Example
 */
struct _bor_nn_linear_el_t {
    const bor_vec_t *p; /*!< Pointer to user-defined point vector */
    bor_list_t list;    /*!< Connection into node's list of elements */
    bor_real_t dist;
};
typedef struct _bor_nn_linear_el_t bor_nn_linear_el_t;

/**
 * Initialize element struct.
 * This must be called before added.
 */
_bor_inline void borNNLinearElInit(bor_nn_linear_el_t *el, const bor_vec_t *p);



/**
 * Functions
 * ----------
 */

/**
 * Creates new empty search structure.
 */
bor_nn_linear_t *borNNLinearNew(const bor_nn_linear_params_t *params);

/**
 * Destructor.
 */
void borNNLinearDel(bor_nn_linear_t *nn);

/**
 * Adds element.
 */
_bor_inline void borNNLinearAdd(bor_nn_linear_t *nn, bor_nn_linear_el_t *el);

/**
 * Removes element.
 */
_bor_inline void borNNLinearRemove(bor_nn_linear_t *nn, bor_nn_linear_el_t *el);

/**
 * Updates position of the element.
 */
_bor_inline void borNNLinearUpdate(bor_nn_linear_t *nn, bor_nn_linear_el_t *el);

/**
 * Finds {num} nearest elements to given point {p}.
 *
 * Array of pointers els must be allocated and must have at least {num}
 * elements. This array is filled with pointers to elements that are
 * nearest to point {p}. Number of found elements is returned.
 */
size_t borNNLinearNearest(const bor_nn_linear_t *nn, const bor_vec_t *p, size_t num,
                          bor_nn_linear_el_t **els);

/**** INLINES ****/
_bor_inline void borNNLinearElInit(bor_nn_linear_el_t *el, const bor_vec_t *p)
{
    el->p = p;
    borListInit(&el->list);
}

_bor_inline void borNNLinearAdd(bor_nn_linear_t *nn, bor_nn_linear_el_t *el)
{
    borListAppend(&nn->list, &el->list);
}

_bor_inline void borNNLinearRemove(bor_nn_linear_t *nn, bor_nn_linear_el_t *el)
{
    borListDel(&el->list);
}

_bor_inline void borNNLinearUpdate(bor_nn_linear_t *nn, bor_nn_linear_el_t *el)
{
    // nop
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_NN_LINEAR_H__ */
