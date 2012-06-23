/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_NET_H__
#define __BOR_NET_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Net - Neural Network
 * =====================
 * TODO
 */

struct _bor_net_t {
    bor_list_t nodes; /*!< List of nodes */
    size_t nodes_len; /*!< Number of nodes in list */
    bor_list_t edges; /*!< List of edges */
    size_t edges_len; /*!< Number of edges in list */
};
typedef struct _bor_net_t bor_net_t;


struct _bor_net_node_t {
    bor_list_t list;     /*!< Edgeection into list of all nodes */

    bor_list_t edges; /*!< List of all incidenting edgeections */
    size_t edges_len; /*!< Number of edgeections in list */
};
typedef struct _bor_net_node_t bor_net_node_t;


struct _bor_net_edge_t {
    bor_net_node_t *n[2]; /*!< Start and end nodes of edge */
    bor_list_m_t nlist[2]; /*!< Edgeection into list of edgeections
                                incidenting with node.
                                .nlist[0] correspond with node .n[0] and
                                .nlist[1] with .n[1] */

    bor_list_t list;       /*!< Edgeection into list of all edges */
};
typedef struct _bor_net_edge_t bor_net_edge_t;


/**
 * Node
 * -------
 * TODO
 *
 * See bor_net_node_t.
 */

/**
 * Allocates and initializes new node.
 */
bor_net_node_t *borNetNodeNew(void);

/**
 * Deletes node.
 */
void borNetNodeDel(bor_net_node_t *v);

/**
 * Returns number of edgeections incidenting with given node.
 */
_bor_inline size_t borNetNodeEdgesLen(const bor_net_node_t *v);

/**
 * Returns head of list of edgeections incidenting with node.
 */
_bor_inline bor_list_t *borNetNodeEdges(bor_net_node_t *n);

/**
 * Returns true if node incidents with given edgeection.
 */
_bor_inline int borNetNodeHasEdge(const bor_net_node_t *n,
                                   const bor_net_edge_t *e);

/**
 * Returns edgeection (first) edgeecting given pair of nodes.
 */
bor_net_edge_t *borNetNodeCommonEdge(const bor_net_node_t *n1,
                                       const bor_net_node_t *n2);


/**
 * Edge
 * -----
 * TODO
 *
 * See bor_net_edge_t.
 */

/**
 * Allocates and initializes edgeection.
 */
bor_net_edge_t *borNetEdgeNew(void);

/**
 * Deletes edgeection.
 */
void borNetEdgeDel(bor_net_edge_t *e);

/**
 * Returns start or end node of edge.
 * Parameter i can be either 0 or 1 (no check is performed).
 */
_bor_inline bor_net_node_t *borNetEdgeNode(bor_net_edge_t *e, size_t i);

/**
 * Returns true if edgeection incidents with given node.
 */
_bor_inline int borNetEdgeHasNode(const bor_net_edge_t *e,
                                   const bor_net_node_t *n);

/**
 * Returns the other (start/end) node than provided.
 */
_bor_inline bor_net_node_t *borNetEdgeOtherNode(bor_net_edge_t *e,
                                                  const bor_net_node_t *n);

/**
 * Returns true if given triplet of edgeections form triangle.
 */
int borNetEdgeTriCheck(const bor_net_edge_t *e1,
                        const bor_net_edge_t *e2,
                        const bor_net_edge_t *e3);

/**
 * Return pointer of edge struct based on list item (pointer to nlist[0|1].
 */
_bor_inline bor_net_edge_t *borNetEdgeFromNodeList(bor_list_t *l);




/**
 * Net
 * ----
 * TODO
 *
 * See bor_net_t.
 */

/**
 * Creates new empty network.
 */
bor_net_t *borNetNew(void);

/**
 * Deletes network.
 * Warning: No nodes or edges are deleted because they were
 * allocated outside a net! See borNetDel2() if you want more
 * sofisticated destructor.
 */
void borNetDel(bor_net_t *m);

/**
 * Deletes net. This destructor can be used for deleting all nodes and
 * edges contained in net.
 *
 * Before freeing a net, destructor iterates over all nodes. Each
 * node is first disedgeected from net and then delnode is called with
 * second argument ndata. Similarly are iterated edges.
 */
void borNetDel2(bor_net_t *m,
                 void (*delnode)(bor_net_node_t *, void *), void *ndata,
                 void (*deledge)(bor_net_edge_t *, void *), void *cdata);

/**
 * Returns number of nodes stored in net.
 */
_bor_inline size_t borNetNodesLen(const bor_net_t *m);

/**
 * Returns number of edges stored in net.
 */
_bor_inline size_t borNetEdgesLen(const bor_net_t *m);

/**
 * Returns list of nodes.
 */
_bor_inline bor_list_t *borNetNodes(bor_net_t *m);

/**
 * Returns list of edgeections.
 */
_bor_inline bor_list_t *borNetEdges(bor_net_t *m);

/**
 * Adds node into net.
 */
void borNetAddNode(bor_net_t *m, bor_net_node_t *v);

/**
 * Removes node from net.
 * If node is edgeected with any edgeection, node can't be removed and -1
 * is returned. On success 0 is returned.
 */
int borNetRemoveNode(bor_net_t *m, bor_net_node_t *v);

/**
 * Adds edgeection into net. Start and end points (nodes) must be provided.
 */
void borNetAddEdge(bor_net_t *m, bor_net_edge_t *e,
                    bor_net_node_t *start, bor_net_node_t *end);

/**
 * Removes edgeection from net.
 */
void borNetRemoveEdge(bor_net_t *m, bor_net_edge_t *e);


/**
 * Traverses all nodes in net.
 * The callback {cb} is called for each node in net.
 */
void borNetTraverseNode(bor_net_t *net,
                        void (*cb)(bor_net_t *net, bor_net_node_t *n, void *data),
                        void *data);

/**
 * Traverses all edges in net.
 * The callback {cb} is called for each edge in net.
 */
void borNetTraverseEdge(bor_net_t *net,
                        void (*cb)(bor_net_t *net, bor_net_edge_t *e, void *data),
                        void *data);

#if 0
/**
 * Dumps net as one object in SVT format.
 * See http://svt.danfis.cz for more info.
 */
void borNetDumpSVT(bor_net_t *m, FILE *out, const char *name);

/**
 * Dumps net as list of triangles.
 * One triangle per line (ax ay az bx by ...)
 */
void borNetDumpTriangles(bor_net_t *m, FILE *out);

/**
 * Dumps net in povray format.
 */
void borNetDumpPovray(bor_net_t *m, FILE *out);
#endif



/**** INLINES ****/
_bor_inline size_t borNetNodeEdgesLen(const bor_net_node_t *v)
{
    return v->edges_len;
}

_bor_inline bor_list_t *borNetNodeEdges(bor_net_node_t *v)
{
    return &v->edges;
}

_bor_inline int borNetNodeHasEdge(const bor_net_node_t *v,
                                      const bor_net_edge_t *e)
{
    bor_list_t *item;
    bor_net_edge_t *edge;

    BOR_LIST_FOR_EACH(&v->edges, item){
        edge = borNetEdgeFromNodeList(item);
        if (edge == e)
            return 1;
    }

    return 0;
}





_bor_inline bor_net_node_t *borNetEdgeNode(bor_net_edge_t *e, size_t i)
{
    return e->n[i];
}

_bor_inline int borNetEdgeHasNode(const bor_net_edge_t *e,
                                   const bor_net_node_t *n)
{
    return e->n[0] == n || e->n[1] == n;
}

_bor_inline bor_net_node_t *borNetEdgeOtherNode(bor_net_edge_t *e,
                                                  const bor_net_node_t *v)
{
    if (e->n[0] == v)
        return e->n[1];
    return e->n[0];
}

_bor_inline bor_net_edge_t *borNetEdgeFromNodeList(bor_list_t *l)
{
    bor_list_m_t *m;
    bor_net_edge_t *e;

    m = borListMFromList(l);
    e = BOR_LIST_M_ENTRY(l, bor_net_edge_t, nlist, m->mark);

    return e;
}


_bor_inline size_t borNetNodesLen(const bor_net_t *n)
{
    return n->nodes_len;
}

_bor_inline size_t borNetEdgesLen(const bor_net_t *n)
{
    return n->edges_len;
}

_bor_inline bor_list_t *borNetNodes(bor_net_t *n)
{
    return &n->nodes;
}

_bor_inline bor_list_t *borNetEdges(bor_net_t *n)
{
    return &n->edges;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_NET_H__ */


