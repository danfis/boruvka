/***
 * fermat
 * -------
 * Copyright (c)2012 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __FER_GPC_TREE_H__
#define __FER_GPC_TREE_H__

#include <stdio.h>
#include <fermat/gpc.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Base struct for tree's node
 */
struct _fer_gpc_node_t {
    int idx;   /*!< Index of predicate/class */
    int ndesc; /*!< Number of descendants.
                    If 0, this is a terminal (class) node */
    struct _fer_gpc_node_t **desc; /*!< Array of descendants */
    void *mem; /*!< User defined memory */
} fer_packed;
typedef struct _fer_gpc_node_t fer_gpc_node_t;


/**
 * Contructs a new node
 */
fer_gpc_node_t *ferGPCNodeNew(int idx, int ndesc, size_t memsize);

/**
 * Recursively deletes a node and its subtree
 */
void ferGPCNodeDel(fer_gpc_node_t *node);

/**
 * Recursively clones a node tree
 */
fer_gpc_node_t *ferGPCNodeClone(fer_gpc_t *gpc, fer_gpc_node_t *node);




/**
 * Struct representing a whole tree (an individual)
 */
struct _fer_gpc_tree_t {
    fer_real_t fitness;   /*!< Fitness of individual represented by the tree */
    fer_gpc_node_t *root; /*!< Root node of the tree */
    int num_nodes;        /*!< Number of nodes in the tree */
    int depth;            /*!< Depth of the tree */
};
typedef struct _fer_gpc_tree_t fer_gpc_tree_t;


/**
 * Creates a new tree structure
 */
fer_gpc_tree_t *ferGPCTreeNew(void);

/**
 * Deletes a tree
 */
void ferGPCTreeDel(fer_gpc_tree_t *tree);

/**
 * Clones a whole tree.
 */
fer_gpc_tree_t *ferGPCTreeClone(fer_gpc_t *gpc, fer_gpc_tree_t *tree);

/**
 * Fixes info about a tree.
 * If the tree was changed, this function should be called to fix an info
 * about it stored in the structre.
 */
void ferGPCTreeFix(fer_gpc_tree_t *tree);

/**
 * Returns a idx'th node from a tree along with of its storage in desc
 * array and its depth.
 */
fer_gpc_node_t *ferGPCTreeNodeById(fer_gpc_tree_t *tree, int idx,
                                   fer_gpc_node_t ***desc, int *depth);

/**
 * Pretty print of a tree. For debug purposes.
 */
void __ferGPCTreePrint(const fer_gpc_tree_t *tree, FILE *fout);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GPC_TREE_H__ */
