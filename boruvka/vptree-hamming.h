/***
 * Boruvka
 * --------
 * Copyright (c)2013 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_VPTREE_HAMMING_H__
#define __BOR_VPTREE_HAMMING_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _bor_vptree_hamming_el_t;

/**
 * Vantage Point Tree for Hamming Distance
 * ========================================
 *
 * See bor_vptree_hamming_t.
 */


/**
 * Parameters
 * -----------
 *
 * See bor_vptree_hamming_params_t.
 */

/** ^^^^ */

struct _bor_vptree_hamming_params_t {
    size_t size;          /*!< Byte-length of keys (not bit-length!) */
    int minsize;          /*!< Minimal number of elements in leaf node. Default: 1 */
    int maxsize;          /*!< Maximal number of elements in leaf node. Default: 2 */
    int samplesize;       /*!< Size of the samples used in *Build()
                               function. Default: 5 */
};
typedef struct _bor_vptree_hamming_params_t bor_vptree_hamming_params_t;

/**
 * Initializes params struct
 */
void borVPTreeHammingParamsInit(bor_vptree_hamming_params_t *params);



struct __bor_vptree_hamming_node_t {
    char *vp; /*!< Vantage point */
    unsigned int radius;
    struct __bor_vptree_hamming_node_t *left, *right; /*!< Left and right subtree */
    struct __bor_vptree_hamming_node_t *parent;
    bor_list_t els;    /*!< List of elements */
    size_t size;       /*!< Number of elements */
};
typedef struct __bor_vptree_hamming_node_t _bor_vptree_hamming_node_t;

struct _bor_vptree_hamming_t {
    // TODO
    uint8_t type; /*!< Type of NN search algorithm. See boruvka/nn.h */

    bor_vptree_hamming_params_t params;
    _bor_vptree_hamming_node_t *root;

    struct _bor_vptree_hamming_el_t **els; /*!< Tmp array for elements */
    size_t els_size;                       /*!< Size of .els array */
};
typedef struct _bor_vptree_hamming_t bor_vptree_hamming_t;



/**
 * User structure
 * ---------------
 *
 * TODO: Example
 */
struct _bor_vptree_hamming_el_t {
    char *data;
    bor_list_t list;    /*!< Connection into node's list of elements */
    _bor_vptree_hamming_node_t *node; /*!< Back reference to owner of this element */
};
typedef struct _bor_vptree_hamming_el_t bor_vptree_hamming_el_t;

/**
 * Initialize element struct.
 * This must be called before added to vp-tree
 */
void borVPTreeHammingElInit(bor_vptree_hamming_el_t *el, const char *data);



/**
 * Functions
 * ----------
 */

/**
 * Creates new empty vp-tree
 */
bor_vptree_hamming_t *borVPTreeHammingNew(const bor_vptree_hamming_params_t *params);

#if 0
/**
 * Builds vp-tree from array of elements
 * TODO: Example
 */
bor_vptree_t *borVPTreeBuild(const bor_vptree_params_t *params,
                             bor_vptree_el_t *els, size_t els_len, size_t stride);
#endif

/**
 * Deletes vp-tree
 */
void borVPTreeHammingDel(bor_vptree_hamming_t *vp);

/**
 * Adds element to the vp-tree
 */
void borVPTreeHammingAdd(bor_vptree_hamming_t *vp, bor_vptree_hamming_el_t *el);

#if 0
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
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_VPTREE_HAMMING_H__ */
