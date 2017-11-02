/***
 * Boruvka
 * --------
 * Copyright (c)2017 Daniel Fiser <danfis@danfis.cz>
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

#include <boruvka/alloc.h>
#include <boruvka/digraph.h>

void borDigraphInit(bor_digraph_t *g)
{
    bzero(g, sizeof(*g));
}

void borDigraphFree(bor_digraph_t *g)
{
    for (int i = 0; i < g->node_size; ++i){
        borISetFree(&g->node[i].label);
        borISetFree(&g->node[i].out);
        borISetFree(&g->node[i].in);
    }
    for (int i = 0; i < g->edge_size; ++i)
        borISetFree(&g->edge[i].label);
    if (g->node != NULL)
        BOR_FREE(g->node);
    if (g->edge != NULL)
        BOR_FREE(g->edge);
}

int borDigraphAddNode(bor_digraph_t *g)
{
    bor_digraph_node_t *node;

    if (g->node_size >= g->node_alloc){
        if (g->node_alloc == 0)
            g->node_alloc = 1;
        g->node_alloc *= 2;
        g->node = BOR_REALLOC_ARR(g->node, bor_digraph_node_t, g->node_alloc);
    }

    node = g->node + g->node_size++;
    bzero(node, sizeof(*node));

    return g->node_size - 1;
}

int borDigraphAddEdge(bor_digraph_t *g, int from_node, int to_node)
{
    bor_digraph_edge_t *edge;

    if (g->edge_size >= g->edge_alloc){
        if (g->edge_alloc == 0)
            g->edge_alloc = 1;
        g->edge_alloc *= 2;
        g->edge = BOR_REALLOC_ARR(g->edge, bor_digraph_edge_t, g->edge_alloc);
    }

    edge = g->edge + g->edge_size++;
    bzero(edge, sizeof(*edge));
    edge->from = from_node;
    edge->to = to_node;
    borISetInit(&edge->label);

    borISetAdd(&g->node[from_node].out, g->edge_size - 1);
    borISetAdd(&g->node[to_node].in, g->edge_size - 1);

    return g->edge_size - 1;
}

int borDigraphGetEdge(const bor_digraph_t *g, int from_node, int to_node)
{
    const bor_digraph_node_t *node = g->node + from_node;
    int edge_id;

    BOR_ISET_FOR_EACH(&node->out, edge_id){
        if (g->edge[edge_id].to == to_node)
            return edge_id;
    }
    return -1;
}


static int borDigraphInduceNode(const bor_digraph_t *g, int node,
                                bor_digraph_t *dst)
{
    int dnode = borDigraphAddNode(dst);
    borISetUnion(&dst->node[dnode].label, &g->node[node].label);
    dst->node[dnode].color = g->node[node].color;
    return dnode;
}

void borDigraphInduce(bor_digraph_t *dst,
                      const bor_digraph_t *g,
                      const bor_iset_t *nodes,
                      const bor_iset_t *edges)
{
    int *node_map; // mapping from g nodes to dst nodes
    int node, edge, dedge;

    borDigraphInit(dst);

    node_map = BOR_ALLOC_ARR(int, g->node_size);
    for (int i = 0; i < g->node_size; ++i)
        node_map[i] = -1;

    // Create nodes
    if (nodes != NULL){
        BOR_ISET_FOR_EACH(nodes, node)
            node_map[node] = borDigraphInduceNode(g, node, dst);
    }else{
        BOR_ISET_FOR_EACH(edges, edge){
            const bor_digraph_edge_t *e = g->edge + edge;
            if (node_map[e->from] == -1)
                node_map[e->from] = borDigraphInduceNode(g, e->from, dst);
            if (node_map[e->to] == -1)
                node_map[e->to] = borDigraphInduceNode(g, e->to, dst);
        }
    }

    // Create edges
    for (int edge = 0; edge < g->edge_size; ++edge){
        if (edges != NULL && !borISetIn(edge, edges))
            continue;

        const bor_digraph_edge_t *e = g->edge + edge;
        if (node_map[e->from] >= 0 && node_map[e->to] >= 0){
            dedge = borDigraphAddEdge(dst, node_map[e->from], node_map[e->to]);
            borISetUnion(&dst->edge[dedge].label, &e->label);
            dst->edge[dedge].color = e->color;
        }
    }

    BOR_FREE(node_map);
}


struct scc_dfs {
    int cur_index;
    int *index;
    int *lowlink;
    int *in_stack;
    int *stack;
    int stack_size;
};
typedef struct scc_dfs scc_dfs_t;

static void sccTarjanExtract(bor_digraph_t *scc, int *scc_node_map,
                             scc_dfs_t *dfs, int nid)
{
    int i, depth, dst_node;

    // Find how deep unroll stack
    for (i = dfs->stack_size - 1; dfs->stack[i] != nid; --i)
        dfs->in_stack[dfs->stack[i]] = 0;
    dfs->in_stack[dfs->stack[i]] = 0;
    depth = dfs->stack_size - i;

    // Create new component
    dst_node = borDigraphAddNode(scc);

    // Copy node IDs from stack to the component
    for (int j = 0; j < depth; ++j){
        borDigraphNodeAddLabel(scc, dst_node, dfs->stack[i + j]);
        scc_node_map[dfs->stack[i + j]] = dst_node;
    }

    // Shrink stack
    dfs->stack_size = i;
}

static void sccTarjanStrongconnect(bor_digraph_t *scc, int *scc_node_map,
                                   const bor_digraph_t *g,
                                   scc_dfs_t *dfs, int nid)
{
    int w, edge_id;
    const bor_digraph_node_t *node = g->node + nid;

    dfs->index[nid] = dfs->lowlink[nid] = dfs->cur_index++;
    dfs->stack[dfs->stack_size++] = nid;
    dfs->in_stack[nid] = 1;

    BOR_ISET_FOR_EACH(&node->out, edge_id){
        w = g->edge[edge_id].to;
        if (dfs->index[w] == -1){
            sccTarjanStrongconnect(scc, scc_node_map, g, dfs, w);
            dfs->lowlink[nid] = BOR_MIN(dfs->lowlink[nid], dfs->lowlink[w]);
        }else if (dfs->in_stack[w]){
            dfs->lowlink[nid] = BOR_MIN(dfs->lowlink[nid], dfs->lowlink[w]);
        }
    }

    if (dfs->index[nid] == dfs->lowlink[nid])
        sccTarjanExtract(scc, scc_node_map, dfs, nid);
}

static void sccAddEdges(bor_digraph_t *scc, const int *scc_node_map,
                        const bor_digraph_t *g)
{
    for (int i = 0; i < g->edge_size; ++i){
        const bor_digraph_edge_t *edge = g->edge + i;
        int f = scc_node_map[edge->from];
        int t = scc_node_map[edge->to];
        if (f != t){
            int e = borDigraphGetOrAddEdge(scc, f, t);
            bor_digraph_edge_t *dst = scc->edge + e;
            borISetUnion(&dst->label, &edge->label);
        }
    }
}

void borDigraphSCC(bor_digraph_t *scc, const bor_digraph_t *g)
{
    scc_dfs_t dfs;
    int *scc_node_map;

    borDigraphInit(scc);

    // Initialize structure for Tarjan's algorithm
    dfs.cur_index = 0;
    dfs.index    = BOR_ALLOC_ARR(int, 4 * g->node_size);
    dfs.lowlink  = dfs.index + g->node_size;
    dfs.in_stack = dfs.lowlink + g->node_size;
    dfs.stack    = dfs.in_stack + g->node_size;
    dfs.stack_size = 0;
    for (int i = 0; i < g->node_size; ++i){
        dfs.index[i] = dfs.lowlink[i] = -1;
        dfs.in_stack[i] = 0;
    }

    // Allocate map between g nodes and ID of components
    scc_node_map = BOR_ALLOC_ARR(int, g->node_size);
    for (int i = 0; i < g->node_size; ++i)
        scc_node_map[i] = -1;

    // Find out SCCs
    for (int node = 0; node < g->node_size; ++node){
        if (dfs.index[node] == -1)
            sccTarjanStrongconnect(scc, scc_node_map, g, &dfs, node);
    }

    // Add edges to the SCC graph
    if (scc->node_size > 1)
        sccAddEdges(scc, scc_node_map, g);

    BOR_FREE(scc_node_map);
    BOR_FREE(dfs.index);
}

static void dfs(const bor_digraph_t *g, int from, const bor_iset_t *ignore,
                int *visited)
{
    int edge_id;
    const bor_digraph_node_t *node = g->node + from;

    visited[from] = 1;
    BOR_ISET_FOR_EACH(&node->out, edge_id){
        const bor_digraph_edge_t *edge = g->edge + edge_id;
        if (ignore != NULL && borISetIn(edge->to, ignore))
            continue;
        if (!visited[edge->to])
            dfs(g, edge->to, ignore, visited);
    }
}

void borDigraphDFS(const bor_digraph_t *g, const bor_iset_t *from_nodes,
                   const bor_iset_t *ignore, int *visited)
{
    int node_id;

    bzero(visited, sizeof(int) * g->node_size);
    BOR_ISET_FOR_EACH(from_nodes, node_id){
        if (!visited[node_id])
            dfs(g, node_id, ignore, visited);
    }
}

void borDigraphPrintDebug(const bor_digraph_t *g, FILE *fout)
{
    int id, edge;

    for (int i = 0; i < g->node_size; ++i){
        fprintf(fout, "N%d", i);
        if (g->node[i].color != 0)
            fprintf(fout, " C%d", g->node[i].color);
        BOR_ISET_FOR_EACH(&g->node[i].label, id)
            fprintf(fout, " %d", id);
        fprintf(fout, ":-");
        BOR_ISET_FOR_EACH(&g->node[i].out, edge){
            fprintf(fout, " N%d", g->edge[edge].to);
            if (g->edge[edge].color != 0)
                fprintf(fout, "+T%d", g->edge[edge].color);
            BOR_ISET_FOR_EACH(&g->edge[edge].label, id)
                fprintf(fout, "+%d", id);
        }
        fprintf(fout, "\n");
    }
}

void borDigraphPrintDot(const bor_digraph_t *g, FILE *fout,
                   void (*node_label)(const bor_digraph_t *g, FILE *fout,
                                      int node_id, void *ud),
                   void (*node_color)(const bor_digraph_t *g, FILE *fout,
                                      int node_id, void *ud),
                   void (*edge_label)(const bor_digraph_t *g, FILE *fout,
                                      int edge_id, void *ud),
                   void (*edge_color)(const bor_digraph_t *g, FILE *fout,
                                      int edge_id, void *ud),
                   void *ud)
{
    fprintf(fout, "digraph {\n");
    for (int i = 0; i < g->node_size; ++i){
        fprintf(fout, "\tN%d [label=\"", i);
        node_label(g, fout, i, ud);
        fprintf(fout, "\",color=\"");
        node_color(g, fout, i, ud);
        fprintf(fout, "\"];\n");
    }

    for (int i = 0; i < g->edge_size; ++i){
        const bor_digraph_edge_t *e = g->edge + i;
        fprintf(fout, "\tN%d -> N%d [label=\"", e->from, e->to);
        edge_label(g, fout, i, ud);
        fprintf(fout, "\",color=\"");
        edge_color(g, fout, i, ud);
        fprintf(fout, "\"];\n");
    }
    fprintf(fout, "}\n");
}
