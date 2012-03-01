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
#include <fermat/dbg.h>
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
    tree->num_nodes = 0;

    return tree;
}

void ferGPCTreeDel(fer_gpc_tree_t *tree)
{
    // TODO: del root and its subtree
    if (tree->root)
        ferGPCNodeDel(tree->root);
    FER_FREE(tree);
}

fer_gpc_tree_t *ferGPCTreeClone(fer_gpc_t *gpc, fer_gpc_tree_t *tree)
{
    fer_gpc_tree_t *ntree;

    ntree            = ferGPCTreeNew();
    ntree->fitness   = tree->fitness;
    ntree->root      = ferGPCNodeClone(gpc, tree->root);
    ntree->num_nodes = tree->num_nodes;

    return ntree;
}

static size_t fixNumNodes(fer_gpc_node_t *node,
                          unsigned int depth, unsigned int *rdepth)
{
    fer_gpc_node_t **desc;
    size_t i, num = 1;

    if (depth > *rdepth)
        *rdepth = depth;

    if (node->ndesc > 0){
        desc = FER_GPC_NODE_DESC(node);
        for (i = 0; i < node->ndesc; i++){
            num += fixNumNodes(desc[i], depth + 1, rdepth);
        }
    }

    return num;
}

void ferGPCTreeFix(fer_gpc_tree_t *tree)
{
    unsigned int depth = 0;

    if (tree->root){
        tree->num_nodes = fixNumNodes(tree->root, 0, &depth);
        tree->depth = depth;
    }
}


static int nodeById(fer_gpc_node_t *node, size_t idx, size_t cur,
                    size_t depth,
                    fer_gpc_node_t **rnode, fer_gpc_node_t ***rdesc,
                    size_t *rdepth)
{
    fer_gpc_node_t **desc;
    size_t i;
    int ret;

    if (cur == idx){
        // we reached the correct node, record the node and its depth
        if (*rnode == NULL){
            *rnode = node;
            *rdepth = depth;
        }
        return -1;
    }

    if (node->ndesc == 0)
        return cur;

    desc = FER_GPC_NODE_DESC(node);
    for (i = 0; i < node->ndesc; i++){
        ret = nodeById(desc[i], idx, cur + 1, depth + 1,
                       rnode, rdesc, rdepth);
        if (ret == -1){
            // a correct node was reached, record its storage in desc array
            if (*rdesc == NULL)
                *rdesc = &desc[i];
            return -1;
        }else{
            cur = ret;
        }
    }

    return cur;
}

fer_gpc_node_t *ferGPCTreeNodeById(fer_gpc_tree_t *tree, size_t idx,
                                   fer_gpc_node_t ***desc, size_t *depth)
{
    fer_gpc_node_t *node;

    *depth = 0;

    if (idx == 0){
        *desc = &tree->root;
        return tree->root;
    }

    node = NULL;
    *desc = NULL;
    if (nodeById(tree->root, idx, 0, 0, &node, desc, depth) != -1)
        return NULL;

    return node;
}

static void ferGPCNodePrint(const fer_gpc_node_t *node, FILE *fout, int depth)
{
    int i;
    fer_gpc_node_t **desc;

    for (i = 0; i < depth; i++)
        fprintf(fout, "  ");

    fprintf(fout, "idx: %d, ndesc: %d [%lx]",
            (int)node->idx, (int)node->ndesc, (long)node);

    desc = FER_GPC_NODE_DESC(node);
    for (i = 0; i < node->ndesc; i++){
        fprintf(fout, " %lx", (long)&desc[i]);
    }
    fprintf(fout, "\n");

    for (i = 0; i < node->ndesc; i++){
        ferGPCNodePrint(desc[i], fout, depth + 1);
    }
}

void ferGPCTreePrint(const fer_gpc_tree_t *tree, FILE *fout)
{
    fprintf(fout, "fitness: %f, num_nodes: %d [%lx]\n",
            tree->fitness, (int)tree->num_nodes, (long)tree);
    if (tree->root)
        ferGPCNodePrint(tree->root, fout, 0);
    fprintf(fout, "--------\n");
}
