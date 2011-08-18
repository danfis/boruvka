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

#ifndef __FER_VPTREE_H__
#define __FER_VPTREE_H__

#include <fermat/core.h>
#include <fermat/vec.h>
#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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
 * See fer_vptree_t.
 */


/**
 * Returns distance between two {d}-dimensional vectors
 */
typedef fer_real_t (*fer_vptree_dist)(int d,
                                      const fer_vec_t *v1,
                                      const fer_vec_t *v2, void *data);

struct _fer_vptree_params_t {
    int dim;              /*!< Dimension of space. Default: 2 */
    fer_vptree_dist dist; /*!< Callback for distance measurement.
                               Default: L2 norm distance */
    void *dist_data;      /*!< User-defined data for .dist. Default: NULL */
    int minsize;          /*!< Minimal number of elements in leaf node. Default: 1 */
    int maxsize;          /*!< Maximal number of elements in leaf node. Default: 2 */
    int samplesize;       /*!< Size of the samples used in *Build()
                               function. Default: 5 */
};
typedef struct _fer_vptree_params_t fer_vptree_params_t;

/**
 * Initializes params struct
 */
void ferVPTreeParamsInit(fer_vptree_params_t *params);



struct __fer_vptree_node_t {
    fer_vec_t *vp;     /*!< Vantage point */
    fer_real_t mean;
    struct __fer_vptree_node_t *left, *right; /*!< Left and right subtree */
    fer_list_t els;    /*!< List of elements */
    size_t size;       /*!< Number of elements */
};
typedef struct __fer_vptree_node_t _fer_vptree_node_t;

struct _fer_vptree_t {
    fer_vptree_params_t params;
    _fer_vptree_node_t *root;
};
typedef struct _fer_vptree_t fer_vptree_t;



/**
 * User structure
 * ---------------
 *
 * TODO: Example
 */
struct _fer_vptree_el_t {
    fer_list_t list;    /*!< Connection into node's list of elements */
    const fer_vec_t *p; /*!< Pointer to user-defined point */
};
typedef struct _fer_vptree_el_t fer_vptree_el_t;

/**
 * Initialize element struct.
 * This must be called before added to vp-tree
 */
void ferVPTreeElInit(fer_vptree_el_t *el, const fer_vec_t *p);



/**
 * Functions
 * ----------
 */

/**
 * Creates new empty vp-tree
 */
fer_vptree_t *ferVPTreeNew(const fer_vptree_params_t *params);

/**
 * Builds vp-tree from array of elements
 * TODO: Example
 */
fer_vptree_t *ferVPTreeBuild(const fer_vptree_params_t *params,
                             fer_vptree_el_t *els, size_t els_len, size_t stride);

/**
 * Deletes vp-tree
 */
void ferVPTreeDel(fer_vptree_t *vp);

/**
 * Adds element to the vp-tree
 */
void ferVPTreeAdd(fer_vptree_t *vp, fer_vptree_el_t *el);

/**
 * Removes element from vp-tree
 */
void ferVPTreeRemove(fer_vptree_t *vp, fer_vptree_el_t *el);

/**
 * Updates position of element in vp-tree.
 */
void ferVPTreeUpdate(fer_vptree_t *vp, fer_vptree_el_t *el);

/**
 * Finds {num} nearest elements to given point {p}.
 *
 * Array of pointers els must be allocated and must have at least {num}
 * elements. This array is filled with pointers to elements that are
 * nearest to point {p}. Number of found elements is returned.
 */
size_t ferVPTreeNearest(const fer_vptree_t *vp, const fer_vec_t *p, size_t num,
                        fer_vptree_el_t **els);


void ferVPTreeDump(fer_vptree_t *vp, FILE *out);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_VPTREE_H__ */



