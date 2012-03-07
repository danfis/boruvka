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


fer_gpc_node_t *ferGPCNodeNew(int idx, int ndesc, size_t memsize)
{
    fer_gpc_node_t *node;

    node = FER_ALLOC(fer_gpc_node_t);
    node->idx   = idx;
    node->ndesc = ndesc;

    node->desc = NULL;
    if (ndesc > 0){
        node->desc = FER_ALLOC_ARR(fer_gpc_node_t *, ndesc);
    }

    node->mem = NULL;
    if (memsize > 0)
        node->mem = ferRealloc(NULL, memsize);

    return node;
}

void ferGPCNodeDel(fer_gpc_node_t *node)
{
    int i;

    for (i = 0; i < node->ndesc; i++){
        if (node->desc[i] != NULL)
            ferGPCNodeDel(node->desc[i]);
    }

    if (node->desc != NULL)
        FER_FREE(node->desc);
    if (node->mem != NULL)
        FER_FREE(node->mem);
    FER_FREE(node);
}

fer_gpc_node_t *ferGPCNodeClone(fer_gpc_t *gpc, fer_gpc_node_t *node)
{
    fer_gpc_node_t *n;
    size_t memsize;
    int i;

    memsize = 0;
    if (node->ndesc > 0)
        memsize = __ferGPCPredMemsize(gpc, node->idx);

    n = ferGPCNodeNew(node->idx, node->ndesc, memsize);

    memcpy(n->mem, node->mem, memsize);

    for (i = 0; i < n->ndesc; i++){
        n->desc[i] = ferGPCNodeClone(gpc, node->desc[i]);
    }

    return n;
}



fer_gpc_tree_t *ferGPCTreeNew(void)
{
    fer_gpc_tree_t *tree;

    tree = FER_ALLOC(fer_gpc_tree_t);
    tree->fitness = FER_ZERO;
    tree->root    = NULL;
    tree->num_nodes = 0;
    tree->depth     = 0;

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

static int fixNumNodes(fer_gpc_node_t *node, int depth, int *rdepth)
{
    int i, num = 1;

    if (depth > *rdepth)
        *rdepth = depth;

    if (node->ndesc > 0){
        for (i = 0; i < node->ndesc; i++){
            num += fixNumNodes(node->desc[i], depth + 1, rdepth);
        }
    }

    return num;
}

void ferGPCTreeFix(fer_gpc_tree_t *tree)
{
    int depth = 0;

    if (tree->root){
        tree->num_nodes = fixNumNodes(tree->root, 0, &depth);
        tree->depth = depth;
    }
}


static int nodeById(fer_gpc_node_t *node, int idx, int cur, int depth,
                    fer_gpc_node_t **rnode, fer_gpc_node_t ***rdesc,
                    int *rdepth)
{
    int i, ret;

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

    for (i = 0; i < node->ndesc; i++){
        ret = nodeById(node->desc[i], idx, cur + 1, depth + 1,
                       rnode, rdesc, rdepth);
        if (ret == -1){
            // a correct node was reached, record its storage in desc array
            if (*rdesc == NULL)
                *rdesc = &node->desc[i];
            return -1;
        }else{
            cur = ret;
        }
    }

    return cur;
}

fer_gpc_node_t *ferGPCTreeNodeById(fer_gpc_tree_t *tree, int idx,
                                   fer_gpc_node_t ***desc, int *depth)
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

    for (i = 0; i < depth; i++)
        fprintf(fout, "  ");

    fprintf(fout, "idx: %d, ndesc: %d [%lx]",
            (int)node->idx, (int)node->ndesc, (long)node);

    for (i = 0; i < node->ndesc; i++){
        fprintf(fout, " %lx", (long)&node->desc[i]);
    }
    fprintf(fout, "\n");

    for (i = 0; i < node->ndesc; i++){
        ferGPCNodePrint(node->desc[i], fout, depth + 1);
    }
}

void __ferGPCTreePrint(const fer_gpc_tree_t *tree, FILE *fout)
{
    fprintf(fout, "fitness: %f, num_nodes: %d [%lx]\n",
            tree->fitness, (int)tree->num_nodes, (long)tree);
    if (tree->root)
        ferGPCNodePrint(tree->root, fout, 0);
    fprintf(fout, "--------\n");
}
