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

#ifndef __PDDL_DIGRAPH_H__
#define __PDDL_DIGRAPH_H__

#include <stdio.h>
#include <boruvka/iset.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct bor_digraph_edge {
    int from;
    int to;
    bor_iset_t label;
    int color;
};
typedef struct bor_digraph_edge bor_digraph_edge_t;

struct bor_digraph_node {
    bor_iset_t label;
    bor_iset_t out;
    bor_iset_t in;
    int color;
};
typedef struct bor_digraph_node bor_digraph_node_t;

struct bor_digraph {
    bor_digraph_node_t *node;
    int node_size;
    int node_alloc;

    bor_digraph_edge_t *edge;
    int edge_size;
    int edge_alloc;
};
typedef struct bor_digraph bor_digraph_t;

#define PDDL_DIGRAPH_FOR_EACH_NODE(G, N) \
    for (int __i = 0; __i < (G)->node_size && ((N) = (G)->node + __i); ++__i)
#define PDDL_DIGRAPH_FOR_EACH_EDGE(G, N, E) \
    for (int __i = 0; __i < borISetSize(&(N)->edge) \
                && ((E) = (G)->edge + borISetGet(&(N)->edge, __i)); ++__i)

/**
 * Initializes transition graph.
 */
void borDigraphInit(bor_digraph_t *g);

/**
 * Frees allocated memory.
 */
void borDigraphFree(bor_digraph_t *g);

/**
 * Copy the graph.
 */
void borDigraphCopy(bor_digraph_t *dst, const bor_digraph_t *src);

/**
 * Adds a new transition graph node and returns its assigned ID.
 */
int borDigraphAddNode(bor_digraph_t *g);

/**
 * Adds a new edge between two nodes.
 */
int borDigraphAddEdge(bor_digraph_t *g, int from_node, int to_node);

/**
 * Returns ID of the edge connection the given two nodes.
 */
int borDigraphGetEdge(const bor_digraph_t *g, int from_node, int to_node);

/**
 * Adds fact to the node label.
 */
_bor_inline void borDigraphNodeAddLabel(bor_digraph_t *g,
                                        int node_id, int label)
{
    borISetAdd(&g->node[node_id].label, label);
}

/**
 * Adds operator's ID to the edge label.
 */
_bor_inline void borDigraphEdgeAddLabel(bor_digraph_t *g,
                                        int edge_id, int label)
{
    borISetAdd(&g->edge[edge_id].label, label);
}

/**
 * Returns an edge between from_node and to_node. The edges is created if
 * no such edge exists.
 */
_bor_inline int borDigraphGetOrAddEdge(bor_digraph_t *g,
                                       int from_node, int to_node)
{
    int edge_id = borDigraphGetEdge(g, from_node, to_node);
    if (edge_id < 0)
        edge_id = borDigraphAddEdge(g, from_node, to_node);
    return edge_id;
}

/**
 * Adds op_id onto edge. The edge is created if not already there.
 * Returns ID of the edge.
 */
_bor_inline int borDigraphAddOrUpdateEdge(bor_digraph_t *g, int from_node,
                                          int to_node, int edge_label)
{
    int edge_id = borDigraphGetOrAddEdge(g, from_node, to_node);
    borDigraphEdgeAddLabel(g, edge_id, edge_label);
    return edge_id;
}

/**
 * Store the union of node labels in label.
 */
_bor_inline void borDigraphUnionNodeLabels(const bor_digraph_t *g,
                                           bor_iset_t *label)
{
    for (int i = 0; i < g->node_size; ++i)
        borISetUnion(label, &g->node[i].label);
}

/**
 * Store the union of edge labels in label.
 */
_bor_inline void borDigraphUnionEdgeLabels(const bor_digraph_t *g,
                                           bor_iset_t *label)
{
    for (int i = 0; i < g->edge_size; ++i)
        borISetUnion(label, &g->edge[i].label);
}

/**
 * Create the graph induced by the given nodes.\
 */
void borDigraphInduce(bor_digraph_t *dst,
                      const bor_digraph_t *g,
                      const bor_iset_t *nodes,
                      const bor_iset_t *edges);

/**
 * Computes strongly connected components of g and stores them in a form
 * of a graph in scc so that each node corresponds to one component and the
 * edges correspond to edges between components.
 * The labels of nodes are node IDs of the input graph (not labels of the
 * of the input graph), but labels of the edges are merged labels of the
 * input graph.
 */
void borDigraphSCC(bor_digraph_t *scc, const bor_digraph_t *g);

/**
 * Runs DFS from from_nodes while ignoring nodes in ingore (nodes in ingore
 * set are not considered as a part of the graph).
 * The output array visited is set to true on IDs of nodes that were
 * visited, i.e., visited must have size of (sizeof(int) * g->node_size).
 * It is assumed that from_nodes and ignore are disjunct.
 */
void borDigraphDFS(const bor_digraph_t *g, const bor_iset_t *from_nodes,
                   const bor_iset_t *ignore, int *visited);

void borDigraphPrintDebug(const bor_digraph_t *g, FILE *fout);
void borDigraphPrintDot(const bor_digraph_t *g, FILE *fout,
                   void (*node_label)(const bor_digraph_t *g, FILE *fout,
                                      int node_id, void *ud),
                   void (*node_color)(const bor_digraph_t *g, FILE *fout,
                                      int node_id, void *ud),
                   void (*edge_label)(const bor_digraph_t *g, FILE *fout,
                                      int edge_id, void *ud),
                   void (*edge_color)(const bor_digraph_t *g, FILE *fout,
                                      int edge_id, void *ud),
                   void *ud);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __PDDL_DIGRAPH_H__ */
