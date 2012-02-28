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

#include <string.h>
#include <fermat/alloc.h>
#include <fermat/gpc-tree.h>


fer_gpc_node_t *ferGPCNodeNew(unsigned int idx, uint8_t ndesc, size_t memsize)
{
    fer_gpc_node_t *node;
    fer_gpc_node_t **desc;
    void *mem;
    size_t size;
    uint8_t i;

    size  = sizeof(fer_gpc_node_t);
    size += ndesc * sizeof(fer_gpc_node_t *);
    size += memsize;

    node = ferRealloc(NULL, size);
    node->idx   = idx;
    node->ndesc = ndesc;

    if (ndesc > 0){
        desc = FER_GPC_NODE_DESC(node);
        for (i = 0; i < ndesc; i++){
            desc[i] = NULL;
        }
    }

    if (memsize > 0){
        mem = FER_GPC_NODE_MEM(node);
        memset(mem, 0, memsize);
    }

    return node;
}

void ferGPCNodeDel(fer_gpc_node_t *node)
{
    fer_gpc_node_t **desc;
    uint8_t i;

    if (node->ndesc > 0){
        desc = FER_GPC_NODE_DESC(node);
        for (i = 0; i < node->ndesc; i++){
            if (desc[i])
                ferGPCNodeDel(desc[i]);
        }
    }

    FER_FREE(node);
}

fer_gpc_node_t *ferGPCNodeClone(fer_gpc_t *gpc, fer_gpc_node_t *node)
{
    fer_gpc_node_t *nnode;
    fer_gpc_node_t **ndesc, **desc;
    size_t size, i;

    size  = sizeof(fer_gpc_node_t);
    if (node->ndesc > 0){
        size += node->ndesc * sizeof(fer_gpc_node_t *);
        size += __ferGPCPredMemsize(gpc, node->idx);
    }

    // copy all data
    nnode = ferRealloc(NULL, size);
    memcpy(nnode, node, size);

    // clone also subtree
    desc  = FER_GPC_NODE_DESC(node);
    ndesc = FER_GPC_NODE_DESC(nnode);
    for (i = 0; i < node->ndesc; i++){
        ndesc[i] = ferGPCNodeClone(gpc, desc[i]);
    }

    return nnode;
}



fer_gpc_tree_t *ferGPCTreeNew(void)
{
    fer_gpc_tree_t *tree;

    tree = FER_ALLOC(fer_gpc_tree_t);
    tree->fitness = FER_ZERO;
    tree->root    = NULL;

    return tree;
}

void ferGPCTreeDel(fer_gpc_tree_t *tree)
{
    // TODO: del root and its subtree
    if (tree->root)
        ferGPCNodeDel(tree->root);
    FER_FREE(tree);
}

static void ferGPCNodePrint(const fer_gpc_node_t *node, FILE *fout, int depth)
{
    int i;
    fer_gpc_node_t **desc;

    for (i = 0; i < depth; i++)
        fprintf(fout, "  ");

    fprintf(fout, "idx: %d, ndesc: %d\n", (int)node->idx, (int)node->ndesc);

    desc = FER_GPC_NODE_DESC(node);
    for (i = 0; i < node->ndesc; i++){
        ferGPCNodePrint(desc[i], fout, depth + 1);
    }
}

fer_gpc_tree_t *ferGPCTreeClone(fer_gpc_t *gpc, fer_gpc_tree_t *tree)
{
    fer_gpc_tree_t *ntree;

    ntree = ferGPCTreeNew();
    ntree->fitness = tree->fitness;
    ntree->root = ferGPCNodeClone(gpc, tree->root);

    return ntree;
}

void ferGPCTreePrint(const fer_gpc_tree_t *tree, FILE *fout)
{
    fprintf(fout, "fitness: %f\n", tree->fitness);
    if (tree->root)
        ferGPCNodePrint(tree->root, fout, 0);
    fprintf(fout, "--------\n");
}
