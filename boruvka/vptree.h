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

#ifndef __BOR_VPTREE_H__
#define __BOR_VPTREE_H__

#include <boruvka/core.h>
#include <boruvka/vec.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _bor_vptree_el_t;

/**
 * Vantage Point Tree
 * ===================
 *
 * [1] P. N. Yianilos, Data Structures and Algorithms for Nearest
 *     Neighbor Search in General Metric Spaces, in: Proceedings of
 *     the Fifth Annual ACM-SIAM Symposium on Discrete Algorithms (SODA),
 *     1993.
 * [2] A. W.-c. Fu, P. M.-s. Chan, Y.-L. Cheung, Y. S. Moon, Dynamic
 *     vp-tree indexing for n-nearest neighbor search given pairwise
 *     distances, The VLDB Journal 9 (2000) 154-173.
 * 
 * See bor_vptree_t.
 */


/**
 * Parameters
 * -----------
 *
 * See bor_vptree_params_t.
 */

/** vvvv */

/**
 * Returns distance between two {d}-dimensional vectors
 */
typedef bor_real_t (*bor_vptree_dist)(int d,
                                      const bor_vec_t *v1,
                                      const bor_vec_t *v2, void *data);

/** ^^^^ */

struct _bor_vptree_params_t {
    int dim;              /*!< Dimension of space. Default: 2 */
    bor_vptree_dist dist; /*!< Callback for distance measurement.
                               Default: L2 norm distance */
    void *dist_data;      /*!< User-defined data for .dist. Default: NULL */
    int minsize;          /*!< Minimal number of elements in leaf node. Default: 1 */
    int maxsize;          /*!< Maximal number of elements in leaf node. Default: 2 */
    int samplesize;       /*!< Size of the samples used in *Build()
                               function. Default: 5 */
};
typedef struct _bor_vptree_params_t bor_vptree_params_t;

/**
 * Initializes params struct
 */
void borVPTreeParamsInit(bor_vptree_params_t *params);



struct __bor_vptree_node_t {
    bor_vec_t *vp;     /*!< Vantage point */
    bor_real_t radius;
    struct __bor_vptree_node_t *left, *right; /*!< Left and right subtree */
    struct __bor_vptree_node_t *parent;
    bor_list_t els;    /*!< List of elements */
    size_t size;       /*!< Number of elements */
};
typedef struct __bor_vptree_node_t _bor_vptree_node_t;

struct _bor_vptree_t {
    uint8_t type; /*!< Type of NN search algorithm. See boruvka/nn.h */

    bor_vptree_params_t params;
    _bor_vptree_node_t *root;

    struct _bor_vptree_el_t **els; /*!< Tmp array for elements */
    size_t els_size;               /*!< Size of .els array */
};
typedef struct _bor_vptree_t bor_vptree_t;



/**
 * User structure
 * ---------------
 *
 * TODO: Example
 */
struct _bor_vptree_el_t {
    const bor_vec_t *p; /*!< Pointer to user-defined point vector */
    bor_list_t list;    /*!< Connection into node's list of elements */
    _bor_vptree_node_t *node; /*!< Back reference to owner of this element */
};
typedef struct _bor_vptree_el_t bor_vptree_el_t;

/**
 * Initialize element struct.
 * This must be called before added to vp-tree
 */
void borVPTreeElInit(bor_vptree_el_t *el, const bor_vec_t *p);



/**
 * Functions
 * ----------
 */

/**
 * Creates new empty vp-tree
 */
bor_vptree_t *borVPTreeNew(const bor_vptree_params_t *params);

/**
 * Builds vp-tree from array of elements
 * TODO: Example
 */
bor_vptree_t *borVPTreeBuild(const bor_vptree_params_t *params,
                             bor_vptree_el_t *els, size_t els_len, size_t stride);

/**
 * Deletes vp-tree
 */
void borVPTreeDel(bor_vptree_t *vp);

/**
 * Adds element to the vp-tree
 */
void borVPTreeAdd(bor_vptree_t *vp, bor_vptree_el_t *el);

/**
 * Removes element from vp-tree
 */
void borVPTreeRemove(bor_vptree_t *vp, bor_vptree_el_t *el);

/**
 * Updates position of element in vp-tree.
 */
void borVPTreeUpdate(bor_vptree_t *vp, bor_vptree_el_t *el);

/**
 * Finds {num} nearest elements to given point {p}.
 *
 * Array of pointers els must be allocated and must have at least {num}
 * elements. This array is filled with pointers to elements that are
 * nearest to point {p}. Number of found elements is returned.
 */
size_t borVPTreeNearest(const bor_vptree_t *vp, const bor_vec_t *p, size_t num,
                        bor_vptree_el_t **els);


void borVPTreeDump(bor_vptree_t *vp, FILE *out);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_VPTREE_H__ */



