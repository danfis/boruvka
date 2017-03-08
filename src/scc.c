/***
 * Boruvka
 * --------
 * Copyright (c)2015 Daniel Fiser <danfis@danfis.cz>
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

#include "boruvka/scc.h"
#include "boruvka/alloc.h"
#include "boruvka/sort.h"

struct _scc_dfs_t {
    int cur_index;
    int *index;
    int *lowlink;
    int *in_stack;
    int *stack;
    int stack_size;
};
typedef struct _scc_dfs_t scc_dfs_t;

static void sccTarjanStrongconnect(bor_scc_t *scc, scc_dfs_t *dfs, int node)
{
    int i, w;
    long it;
    bor_scc_comp_t *comp;

    dfs->index[node] = dfs->lowlink[node] = dfs->cur_index++;
    dfs->stack[dfs->stack_size++] = node;
    dfs->in_stack[node] = 1;

    it = scc->it(node, scc->userdata);
    while ((w = scc->next(node, &it, scc->userdata)) >= 0){
        if (dfs->index[w] == -1){
            sccTarjanStrongconnect(scc, dfs, w);
            dfs->lowlink[node] = BOR_MIN(dfs->lowlink[node], dfs->lowlink[w]);
        }else if (dfs->in_stack[w]){
            dfs->lowlink[node] = BOR_MIN(dfs->lowlink[node], dfs->lowlink[w]);
        }
    }

    if (dfs->index[node] == dfs->lowlink[node]){
        // Find how deep unroll stack
        for (i = dfs->stack_size - 1; dfs->stack[i] != node; --i)
            dfs->in_stack[dfs->stack[i]] = 0;
        dfs->in_stack[dfs->stack[i]] = 0;

        // Create new component
        ++scc->comp_size;
        scc->comp = BOR_REALLOC_ARR(scc->comp, bor_scc_comp_t, scc->comp_size);
        comp = scc->comp + scc->comp_size - 1;
        comp->node_size = dfs->stack_size - i;
        comp->node = BOR_ALLOC_ARR(int, comp->node_size);

        // Copy node IDs from stack to the component
        memcpy(comp->node, dfs->stack + i, sizeof(int) * comp->node_size);
        if (comp->node_size > 1)
            borSortByIntKey(comp->node, comp->node_size, sizeof(int), 0);

        // Shrink stack
        dfs->stack_size = i;
    }
}

static void sccTarjan(bor_scc_t *scc, int start_node)
{
    scc_dfs_t dfs;
    int i, node;

    // Initialize structure for Tarjan's algorithm
    dfs.cur_index = 0;
    dfs.index    = BOR_ALLOC_ARR(int, 4 * scc->node_size);
    dfs.lowlink  = dfs.index + scc->node_size;
    dfs.in_stack = dfs.lowlink + scc->node_size;
    dfs.stack    = dfs.in_stack + scc->node_size;
    dfs.stack_size = 0;
    for (i = 0; i < scc->node_size; ++i){
        dfs.index[i] = dfs.lowlink[i] = -1;
        dfs.in_stack[i] = 0;
    }

    if (start_node >= 0){
        sccTarjanStrongconnect(scc, &dfs, start_node);
    }else{
        for (node = 0; node < scc->node_size; ++node){
            if (dfs.index[node] == -1)
                sccTarjanStrongconnect(scc, &dfs, node);
        }
    }

    BOR_FREE(dfs.index);
}


void borSCCInit(bor_scc_t *scc, int node_size,
                bor_scc_neighbor_it_fn it_fn,
                bor_scc_neighbor_next_fn next_fn,
                void *userdata)
{
    bzero(scc, sizeof(*scc));
    scc->node_size = node_size;
    scc->it = it_fn;
    scc->next = next_fn;
    scc->userdata = userdata;
}

void borSCCFree(bor_scc_t *scc)
{
    int i;

    for (i = 0; i < scc->comp_size; ++i){
        if (scc->comp[i].node != NULL)
            BOR_FREE(scc->comp[i].node);
    }
    if (scc->comp)
        BOR_FREE(scc->comp);
}

void borSCC(bor_scc_t *scc)
{
    sccTarjan(scc, -1);
}

void borSCC1(bor_scc_t *scc, int node)
{
    sccTarjan(scc, node);
}
