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
 * True, if a node is a class (terminal) node
 */
#define FER_GPC_NODE_IS_CLASS(node) \
    (((fer_gpc_node_t *)(node))->type == 0)

/**
 * Returns pointer to (fer_gpc_node_t *[]) array of pointers to descendants
 * of a node.
 * This is always stored after a fer_gpc_node_t struct.
 */
#define FER_GPC_NODE_DESC(node) \
    ((fer_gpc_node_t **)(((char *)(node)) + sizeof(fer_gpc_node_t)))

/**
 * Returns pointer (void *) to predicate memory
 */
#define FER_GPC_NODE_MEM(node) \
    ((void *)(((char*)(node)) + sizeof(fer_gpc_node_t) + sizeof(fer_gpc_node_t *) * (node)->ndesc))


/**
 * Base struct for tree's node
 */
struct _fer_gpc_node_t {
    unsigned int idx; /*!< Index of predicate/class */
    uint8_t ndesc;    /*!< Number of descendants.
                           If 0, this is a terminal node */
} fer_packed;
typedef struct _fer_gpc_node_t fer_gpc_node_t;


/**
 * Contructs a new node
 */
fer_gpc_node_t *ferGPCNodeNew(unsigned int idx, uint8_t ndesc, size_t memsize);

/**
 * Recursively deletes a node and its subtree
 */
void ferGPCNodeDel(fer_gpc_node_t *node);

fer_gpc_node_t *ferGPCNodeClone(fer_gpc_t *gpc, fer_gpc_node_t *node);


/**
 * Struct representing a whole tree (an individual)
 */
struct _fer_gpc_tree_t {
    fer_real_t fitness;   /*!< Fitness of individual represented by a tree */
    fer_gpc_node_t *root; /*!< Root node of a tree */
};
typedef struct _fer_gpc_tree_t fer_gpc_tree_t;


fer_gpc_tree_t *ferGPCTreeNew(void);
void ferGPCTreeDel(fer_gpc_tree_t *tree);
fer_gpc_tree_t *ferGPCTreeClone(fer_gpc_t *gpc, fer_gpc_tree_t *tree);

void ferGPCTreePrint(const fer_gpc_tree_t *tree, FILE *fout);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GPC_TREE_H__ */
