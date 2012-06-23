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

#ifndef __BOR_NN_H__
#define __BOR_NN_H__

#include <boruvka/gug.h>
#include <boruvka/vptree.h>
#include <boruvka/nn-linear.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Generalized API for Nearest Neighbor Search
 * ============================================
 *
 * This is a generalized API for several nearest neighbor search
 * algorithms. If you plan to try exchanging algorithms with each other,
 * this is the easiest way to do that.
 *
 * Currently incorporated algorithms are:
 *     1. :doc:`/bor-gug.h`
 *     2. :doc:`/bor-vptree.h`
 *     3. :doc:`/bor-nn-linear.h`
 *
 * See bor_nn_t.
 * See bor_nn_params_t.
 */

struct _bor_nn_t {
    uint8_t type; /*!< Type of NN search algorithm */
};
typedef struct _bor_nn_t bor_nn_t;

struct _bor_nn_params_t {
    uint8_t type; /*!< Type of NN search algorithm. See 'Types of
                       Algorithms'. Default: BOR_NN_LINEAR */
    bor_gug_params_t gug;
    bor_vptree_params_t vptree;
    bor_nn_linear_params_t linear;
};
typedef struct _bor_nn_params_t bor_nn_params_t;

/**
 * Initialize parameters
 */
_bor_inline void borNNParamsInit(bor_nn_params_t *params);

/**
 * Set dimensionality for all algorithms.
 */
_bor_inline void borNNParamsSetDim(bor_nn_params_t *params, int dim);


/**
 * Types of Algorithms
 * --------------------
 */

/** vvvv */

#define BOR_NN_GUG    1 /*!< Growing Uniform Grid */
#define BOR_NN_VPTREE 2 /*!< VP-Tree */
#define BOR_NN_LINEAR 3 /*!< Linear searching */

/** ^^^^ */


/**
 * User structure
 * ---------------
 *
 * TODO: Example
 */
struct _bor_nn_el_t {
    const bor_vec_t *p; /*!< Pointer to user-defined point vector */
    bor_list_t list;    /*!< Connection into list of elements */
    uint64_t __;        /*!< Eight bytes available for specific NN search
                             algorithm */
};
typedef struct _bor_nn_el_t bor_nn_el_t;

/**
 * Initialize an element struct.
 * This must be called before using the element.
 */
_bor_inline void borNNElInit(bor_nn_t *nn, bor_nn_el_t *el, const bor_vec_t *p);



/**
 * Functions
 * ----------
 */

/**
 * Creates new instance of NN search struct.
 *
 * A type of the NN search algorithm must be provided and appropriate part
 * of the bor_nn_params_t struct must be set.
 */
_bor_inline bor_nn_t *borNNNew(const bor_nn_params_t *params);

/**
 * Free all allocated memory.
 */
_bor_inline void borNNDel(bor_nn_t *nn);

/**
 * Adds an element to the search struct.
 */
_bor_inline void borNNAdd(bor_nn_t *nn, bor_nn_el_t *el);

/**
 * Removes an element from the search struct.
 */
_bor_inline void borNNRemove(bor_nn_t *nn, bor_nn_el_t *el);

/**
 * Updates an element in the search struct.
 * This function should be called whenever a position of the element is
 * changed.
 */
_bor_inline void borNNUpdate(bor_nn_t *nn, bor_nn_el_t *el);


/**
 * Finds {num} elements nearest to the given point {p}.
 *
 * Array of pointers els must be allocated and must have at least {num}
 * elements. This array is filled with pointers to elements that are
 * nearest to point {p}. Number of found elements is returned.
 */
_bor_inline size_t borNNNearest(const bor_nn_t *nn, const bor_vec_t *p,
                                size_t num, bor_nn_el_t **els);


/**** INLINES ****/
_bor_inline void borNNParamsInit(bor_nn_params_t *params)
{
    params->type = BOR_NN_LINEAR;
    borGUGParamsInit(&params->gug);
    borVPTreeParamsInit(&params->vptree);
    borNNLinearParamsInit(&params->linear);
}

_bor_inline void borNNParamsSetDim(bor_nn_params_t *params, int dim)
{
    params->gug.dim = dim;
    params->vptree.dim = dim;
    params->linear.dim = dim;
}

_bor_inline void borNNElInit(bor_nn_t *nn, bor_nn_el_t *el, const bor_vec_t *p)
{
    if (nn->type == BOR_NN_GUG){
        borGUGElInit((bor_gug_el_t *)el, p);
    }else if (nn->type == BOR_NN_VPTREE){
        borVPTreeElInit((bor_vptree_el_t *)el, p);
    }else if (nn->type == BOR_NN_LINEAR){
        borNNLinearElInit((bor_nn_linear_el_t *)el, p);
    }
}

_bor_inline bor_nn_t *borNNNew(const bor_nn_params_t *params)
{
    bor_nn_t *nn = NULL;

    if (params->type == BOR_NN_GUG){
        nn = (bor_nn_t *)borGUGNew(&params->gug);
    }else if (params->type == BOR_NN_VPTREE){
        nn = (bor_nn_t *)borVPTreeNew(&params->vptree);
    }else if (params->type == BOR_NN_LINEAR){
        nn = (bor_nn_t *)borNNLinearNew(&params->linear);
    }

    return nn;
}

_bor_inline void borNNDel(bor_nn_t *nn)
{
    if (nn->type == BOR_NN_GUG){
        borGUGDel((bor_gug_t *)nn);
    }else if (nn->type == BOR_NN_VPTREE){
        borVPTreeDel((bor_vptree_t *)nn);
    }else if (nn->type == BOR_NN_LINEAR){
        borNNLinearDel((bor_nn_linear_t *)nn);
    }
}

_bor_inline void borNNAdd(bor_nn_t *nn, bor_nn_el_t *el)
{
    if (nn->type == BOR_NN_GUG){
        borGUGAdd((bor_gug_t *)nn, (bor_gug_el_t *)el);
    }else if (nn->type == BOR_NN_VPTREE){
        borVPTreeAdd((bor_vptree_t *)nn, (bor_vptree_el_t *)el);
    }else if (nn->type == BOR_NN_LINEAR){
        borNNLinearAdd((bor_nn_linear_t *)nn, (bor_nn_linear_el_t *)el);
    }
}

_bor_inline void borNNRemove(bor_nn_t *nn, bor_nn_el_t *el)
{
    if (nn->type == BOR_NN_GUG){
        borGUGRemove((bor_gug_t *)nn, (bor_gug_el_t *)el);
    }else if (nn->type == BOR_NN_VPTREE){
        borVPTreeRemove((bor_vptree_t *)nn, (bor_vptree_el_t *)el);
    }else if (nn->type == BOR_NN_LINEAR){
        borNNLinearRemove((bor_nn_linear_t *)nn, (bor_nn_linear_el_t *)el);
    }
}

_bor_inline void borNNUpdate(bor_nn_t *nn, bor_nn_el_t *el)
{
    if (nn->type == BOR_NN_GUG){
        borGUGUpdate((bor_gug_t *)nn, (bor_gug_el_t *)el);
    }else if (nn->type == BOR_NN_VPTREE){
        borVPTreeUpdate((bor_vptree_t *)nn, (bor_vptree_el_t *)el);
    }else if (nn->type == BOR_NN_LINEAR){
        borNNLinearUpdate((bor_nn_linear_t *)nn, (bor_nn_linear_el_t *)el);
    }
}

_bor_inline size_t borNNNearest(const bor_nn_t *nn, const bor_vec_t *p,
                                size_t num, bor_nn_el_t **els)
{
    if (nn->type == BOR_NN_GUG){
        return borGUGNearest((const bor_gug_t *)nn, p, num,
                                 (bor_gug_el_t **)els);
    }else if (nn->type == BOR_NN_VPTREE){
        return borVPTreeNearest((const bor_vptree_t *)nn, p, num,
                                (bor_vptree_el_t **)els);
    }else if (nn->type == BOR_NN_LINEAR){
        return borNNLinearNearest((const bor_nn_linear_t *)nn, p, num,
                                  (bor_nn_linear_el_t **)els);
    }

    return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_NN_H__ */
