/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
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

#include <fermat/nncells.h>
#include <fermat/vptree.h>

/**
 * Generalized API for Nearest Neighbor Search
 * ============================================
 *
 * This is a generalized API for several nearest neighbor search
 * algorithms. If you plan to try exchanging algorithms with each other,
 * this is the easiest way to do that.
 *
 * Currently incorporated algorithms are:
 *     1. :doc:`/fer-nncells.h`
 *     2. :doc:`/fer-vptree.h`
 *
 * See fer_nn_t.
 * See fer_nn_params_t.
 */

struct _fer_nn_t {
    uint8_t type; /*!< Type of NN search algorithm */
};
typedef struct _fer_nn_t fer_nn_t;

struct _fer_nn_params_t {
    fer_nncells_params_t nncells;
    fer_vptree_params_t vptree;
};
typedef struct _fer_nn_params_t fer_nn_params_t;

/**
 * Types of Algorithms
 * --------------------
 */
/** vvvv **/
#define FER_NN_NNCELLS 1
#define FER_NN_VPTREE  2
/** ^^^^ **/


/**
 * User structure
 * ---------------
 *
 * TODO: Example
 */
struct _fer_nn_el_t {
    const fer_vec_t *p; /*!< Pointer to user-defined point vector */
    fer_list_t list;    /*!< Connection into list of elements */
    uint64_t __;        /*!< Eight bytes available for specific NN search
                             algorithm */
};
typedef struct _fer_nn_el_t fer_nn_el_t;

/**
 * Initialize an element struct.
 * This must be called before using the element.
 */
_fer_inline void ferNNElInit(fer_nn_t *nn, fer_nn_el_t *el, const fer_vec_t *p);



/**
 * Functions
 * ----------
 */

/**
 * Creates new instance of NN search struct.
 *
 * A type of the NN search algorithm must be provided and appropriate part
 * of the fer_nn_params_t struct must be set.
 */
_fer_inline fer_nn_t *ferNNNew(uint8_t type, const fer_nn_params_t *params);

/**
 * Free all allocated memory.
 */
_fer_inline void ferNNDel(fer_nn_t *nn);

/**
 * Adds an element to the search struct.
 */
_fer_inline void ferNNAdd(fer_nn_t *nn, fer_nn_el_t *el);

/**
 * Removes an element from the search struct.
 */
_fer_inline void ferNNRemove(fer_nn_t *nn, fer_nn_el_t *el);

/**
 * Updates an element in the search struct.
 * This function should be called whenever a position of the element is
 * changed.
 */
_fer_inline void ferNNUpdate(fer_nn_t *nn, fer_nn_el_t *el);


/**
 * Finds {num} elements nearest to the given point {p}.
 *
 * Array of pointers els must be allocated and must have at least {num}
 * elements. This array is filled with pointers to elements that are
 * nearest to point {p}. Number of found elements is returned.
 */
_fer_inline size_t ferNNNearest(const fer_nn_t *nn, const fer_vec_t *p,
                                size_t num, fer_nn_el_t **els);


/**** INLINES ****/
_fer_inline void ferNNElInit(fer_nn_t *nn, fer_nn_el_t *el, const fer_vec_t *p)
{
    if (nn->type == FER_NN_NNCELLS){
        ferNNCellsElInit((fer_nncells_el_t *)el, p);
    }else if (nn->type == FER_NN_VPTREE){
        ferVPTreeElInit((fer_vptree_el_t *)el, p);
    }
}

_fer_inline fer_nn_t *ferNNNew(uint8_t type, const fer_nn_params_t *params)
{
    fer_nn_t *nn = NULL;

    if (type == FER_NN_NNCELLS){
        nn = (fer_nn_t *)ferNNCellsNew(&params->nncells);
    }else if (type == FER_NN_VPTREE){
        nn = (fer_nn_t *)ferVPTreeNew(&params->vptree);
    }

    return nn;
}

_fer_inline void ferNNDel(fer_nn_t *nn)
{
    if (nn->type == FER_NN_NNCELLS){
        ferNNCellsDel((fer_nncells_t *)nn);
    }else if (nn->type == FER_NN_VPTREE){
        ferVPTreeDel((fer_vptree_t *)nn);
    }
}

_fer_inline void ferNNAdd(fer_nn_t *nn, fer_nn_el_t *el)
{
    if (nn->type == FER_NN_NNCELLS){
        ferNNCellsAdd((fer_nncells_t *)nn, (fer_nncells_el_t *)el);
    }else if (nn->type == FER_NN_VPTREE){
        ferVPTreeAdd((fer_vptree_t *)nn, (fer_vptree_el_t *)el);
    }
}

_fer_inline void ferNNRemove(fer_nn_t *nn, fer_nn_el_t *el)
{
    if (nn->type == FER_NN_NNCELLS){
        ferNNCellsRemove((fer_nncells_t *)nn, (fer_nncells_el_t *)el);
    }else if (nn->type == FER_NN_VPTREE){
        ferVPTreeRemove((fer_vptree_t *)nn, (fer_vptree_el_t *)el);
    }
}

_fer_inline void ferNNUpdate(fer_nn_t *nn, fer_nn_el_t *el)
{
    if (nn->type == FER_NN_NNCELLS){
        ferNNCellsUpdate((fer_nncells_t *)nn, (fer_nncells_el_t *)el);
    }else if (nn->type == FER_NN_VPTREE){
        ferVPTreeUpdate((fer_vptree_t *)nn, (fer_vptree_el_t *)el);
    }
}

_fer_inline size_t ferNNNearest(const fer_nn_t *nn, const fer_vec_t *p,
                                size_t num, fer_nn_el_t **els)
{
    if (nn->type == FER_NN_NNCELLS){
        return ferNNCellsNearest((const fer_nncells_t *)nn, p, num,
                                 (fer_nncells_el_t **)els);
    }else if (nn->type == FER_NN_VPTREE){
        return ferVPTreeNearest((const fer_vptree_t *)nn, p, num,
                                (fer_vptree_el_t **)els);
    }

    return 0;
}

#endif /* __FER_NN_H__ */
