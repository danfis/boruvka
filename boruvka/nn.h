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

#ifndef __FER_NN_H__
#define __FER_NN_H__

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
 *     1. :doc:`/fer-gug.h`
 *     2. :doc:`/fer-vptree.h`
 *     3. :doc:`/fer-nn-linear.h`
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
                       Algorithms'. Default: FER_NN_LINEAR */
    bor_gug_params_t gug;
    bor_vptree_params_t vptree;
    bor_nn_linear_params_t linear;
};
typedef struct _bor_nn_params_t bor_nn_params_t;

/**
 * Initialize parameters
 */
_fer_inline void ferNNParamsInit(bor_nn_params_t *params);

/**
 * Set dimensionality for all algorithms.
 */
_fer_inline void ferNNParamsSetDim(bor_nn_params_t *params, int dim);


/**
 * Types of Algorithms
 * --------------------
 */

/** vvvv */

#define FER_NN_GUG    1 /*!< Growing Uniform Grid */
#define FER_NN_VPTREE 2 /*!< VP-Tree */
#define FER_NN_LINEAR 3 /*!< Linear searching */

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
_fer_inline void ferNNElInit(bor_nn_t *nn, bor_nn_el_t *el, const bor_vec_t *p);



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
_fer_inline bor_nn_t *ferNNNew(const bor_nn_params_t *params);

/**
 * Free all allocated memory.
 */
_fer_inline void ferNNDel(bor_nn_t *nn);

/**
 * Adds an element to the search struct.
 */
_fer_inline void ferNNAdd(bor_nn_t *nn, bor_nn_el_t *el);

/**
 * Removes an element from the search struct.
 */
_fer_inline void ferNNRemove(bor_nn_t *nn, bor_nn_el_t *el);

/**
 * Updates an element in the search struct.
 * This function should be called whenever a position of the element is
 * changed.
 */
_fer_inline void ferNNUpdate(bor_nn_t *nn, bor_nn_el_t *el);


/**
 * Finds {num} elements nearest to the given point {p}.
 *
 * Array of pointers els must be allocated and must have at least {num}
 * elements. This array is filled with pointers to elements that are
 * nearest to point {p}. Number of found elements is returned.
 */
_fer_inline size_t ferNNNearest(const bor_nn_t *nn, const bor_vec_t *p,
                                size_t num, bor_nn_el_t **els);


/**** INLINES ****/
_fer_inline void ferNNParamsInit(bor_nn_params_t *params)
{
    params->type = FER_NN_LINEAR;
    ferGUGParamsInit(&params->gug);
    ferVPTreeParamsInit(&params->vptree);
    ferNNLinearParamsInit(&params->linear);
}

_fer_inline void ferNNParamsSetDim(bor_nn_params_t *params, int dim)
{
    params->gug.dim = dim;
    params->vptree.dim = dim;
    params->linear.dim = dim;
}

_fer_inline void ferNNElInit(bor_nn_t *nn, bor_nn_el_t *el, const bor_vec_t *p)
{
    if (nn->type == FER_NN_GUG){
        ferGUGElInit((bor_gug_el_t *)el, p);
    }else if (nn->type == FER_NN_VPTREE){
        ferVPTreeElInit((bor_vptree_el_t *)el, p);
    }else if (nn->type == FER_NN_LINEAR){
        ferNNLinearElInit((bor_nn_linear_el_t *)el, p);
    }
}

_fer_inline bor_nn_t *ferNNNew(const bor_nn_params_t *params)
{
    bor_nn_t *nn = NULL;

    if (params->type == FER_NN_GUG){
        nn = (bor_nn_t *)ferGUGNew(&params->gug);
    }else if (params->type == FER_NN_VPTREE){
        nn = (bor_nn_t *)ferVPTreeNew(&params->vptree);
    }else if (params->type == FER_NN_LINEAR){
        nn = (bor_nn_t *)ferNNLinearNew(&params->linear);
    }

    return nn;
}

_fer_inline void ferNNDel(bor_nn_t *nn)
{
    if (nn->type == FER_NN_GUG){
        ferGUGDel((bor_gug_t *)nn);
    }else if (nn->type == FER_NN_VPTREE){
        ferVPTreeDel((bor_vptree_t *)nn);
    }else if (nn->type == FER_NN_LINEAR){
        ferNNLinearDel((bor_nn_linear_t *)nn);
    }
}

_fer_inline void ferNNAdd(bor_nn_t *nn, bor_nn_el_t *el)
{
    if (nn->type == FER_NN_GUG){
        ferGUGAdd((bor_gug_t *)nn, (bor_gug_el_t *)el);
    }else if (nn->type == FER_NN_VPTREE){
        ferVPTreeAdd((bor_vptree_t *)nn, (bor_vptree_el_t *)el);
    }else if (nn->type == FER_NN_LINEAR){
        ferNNLinearAdd((bor_nn_linear_t *)nn, (bor_nn_linear_el_t *)el);
    }
}

_fer_inline void ferNNRemove(bor_nn_t *nn, bor_nn_el_t *el)
{
    if (nn->type == FER_NN_GUG){
        ferGUGRemove((bor_gug_t *)nn, (bor_gug_el_t *)el);
    }else if (nn->type == FER_NN_VPTREE){
        ferVPTreeRemove((bor_vptree_t *)nn, (bor_vptree_el_t *)el);
    }else if (nn->type == FER_NN_LINEAR){
        ferNNLinearRemove((bor_nn_linear_t *)nn, (bor_nn_linear_el_t *)el);
    }
}

_fer_inline void ferNNUpdate(bor_nn_t *nn, bor_nn_el_t *el)
{
    if (nn->type == FER_NN_GUG){
        ferGUGUpdate((bor_gug_t *)nn, (bor_gug_el_t *)el);
    }else if (nn->type == FER_NN_VPTREE){
        ferVPTreeUpdate((bor_vptree_t *)nn, (bor_vptree_el_t *)el);
    }else if (nn->type == FER_NN_LINEAR){
        ferNNLinearUpdate((bor_nn_linear_t *)nn, (bor_nn_linear_el_t *)el);
    }
}

_fer_inline size_t ferNNNearest(const bor_nn_t *nn, const bor_vec_t *p,
                                size_t num, bor_nn_el_t **els)
{
    if (nn->type == FER_NN_GUG){
        return ferGUGNearest((const bor_gug_t *)nn, p, num,
                                 (bor_gug_el_t **)els);
    }else if (nn->type == FER_NN_VPTREE){
        return ferVPTreeNearest((const bor_vptree_t *)nn, p, num,
                                (bor_vptree_el_t **)els);
    }else if (nn->type == FER_NN_LINEAR){
        return ferNNLinearNearest((const bor_nn_linear_t *)nn, p, num,
                                  (bor_nn_linear_el_t **)els);
    }

    return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_NN_H__ */
