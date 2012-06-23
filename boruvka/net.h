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

#ifndef __FER_NET_H__
#define __FER_NET_H__

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
bor_net_node_t *ferNetNodeNew(void);

/**
 * Deletes node.
 */
void ferNetNodeDel(bor_net_node_t *v);

/**
 * Returns number of edgeections incidenting with given node.
 */
_fer_inline size_t ferNetNodeEdgesLen(const bor_net_node_t *v);

/**
 * Returns head of list of edgeections incidenting with node.
 */
_fer_inline bor_list_t *ferNetNodeEdges(bor_net_node_t *n);

/**
 * Returns true if node incidents with given edgeection.
 */
_fer_inline int ferNetNodeHasEdge(const bor_net_node_t *n,
                                   const bor_net_edge_t *e);

/**
 * Returns edgeection (first) edgeecting given pair of nodes.
 */
bor_net_edge_t *ferNetNodeCommonEdge(const bor_net_node_t *n1,
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
bor_net_edge_t *ferNetEdgeNew(void);

/**
 * Deletes edgeection.
 */
void ferNetEdgeDel(bor_net_edge_t *e);

/**
 * Returns start or end node of edge.
 * Parameter i can be either 0 or 1 (no check is performed).
 */
_fer_inline bor_net_node_t *ferNetEdgeNode(bor_net_edge_t *e, size_t i);

/**
 * Returns true if edgeection incidents with given node.
 */
_fer_inline int ferNetEdgeHasNode(const bor_net_edge_t *e,
                                   const bor_net_node_t *n);

/**
 * Returns the other (start/end) node than provided.
 */
_fer_inline bor_net_node_t *ferNetEdgeOtherNode(bor_net_edge_t *e,
                                                  const bor_net_node_t *n);

/**
 * Returns true if given triplet of edgeections form triangle.
 */
int ferNetEdgeTriCheck(const bor_net_edge_t *e1,
                        const bor_net_edge_t *e2,
                        const bor_net_edge_t *e3);

/**
 * Return pointer of edge struct based on list item (pointer to nlist[0|1].
 */
_fer_inline bor_net_edge_t *ferNetEdgeFromNodeList(bor_list_t *l);




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
bor_net_t *ferNetNew(void);

/**
 * Deletes network.
 * Warning: No nodes or edges are deleted because they were
 * allocated outside a net! See ferNetDel2() if you want more
 * sofisticated destructor.
 */
void ferNetDel(bor_net_t *m);

/**
 * Deletes net. This destructor can be used for deleting all nodes and
 * edges contained in net.
 *
 * Before freeing a net, destructor iterates over all nodes. Each
 * node is first disedgeected from net and then delnode is called with
 * second argument ndata. Similarly are iterated edges.
 */
void ferNetDel2(bor_net_t *m,
                 void (*delnode)(bor_net_node_t *, void *), void *ndata,
                 void (*deledge)(bor_net_edge_t *, void *), void *cdata);

/**
 * Returns number of nodes stored in net.
 */
_fer_inline size_t ferNetNodesLen(const bor_net_t *m);

/**
 * Returns number of edges stored in net.
 */
_fer_inline size_t ferNetEdgesLen(const bor_net_t *m);

/**
 * Returns list of nodes.
 */
_fer_inline bor_list_t *ferNetNodes(bor_net_t *m);

/**
 * Returns list of edgeections.
 */
_fer_inline bor_list_t *ferNetEdges(bor_net_t *m);

/**
 * Adds node into net.
 */
void ferNetAddNode(bor_net_t *m, bor_net_node_t *v);

/**
 * Removes node from net.
 * If node is edgeected with any edgeection, node can't be removed and -1
 * is returned. On success 0 is returned.
 */
int ferNetRemoveNode(bor_net_t *m, bor_net_node_t *v);

/**
 * Adds edgeection into net. Start and end points (nodes) must be provided.
 */
void ferNetAddEdge(bor_net_t *m, bor_net_edge_t *e,
                    bor_net_node_t *start, bor_net_node_t *end);

/**
 * Removes edgeection from net.
 */
void ferNetRemoveEdge(bor_net_t *m, bor_net_edge_t *e);


/**
 * Traverses all nodes in net.
 * The callback {cb} is called for each node in net.
 */
void ferNetTraverseNode(bor_net_t *net,
                        void (*cb)(bor_net_t *net, bor_net_node_t *n, void *data),
                        void *data);

/**
 * Traverses all edges in net.
 * The callback {cb} is called for each edge in net.
 */
void ferNetTraverseEdge(bor_net_t *net,
                        void (*cb)(bor_net_t *net, bor_net_edge_t *e, void *data),
                        void *data);

#if 0
/**
 * Dumps net as one object in SVT format.
 * See http://svt.danfis.cz for more info.
 */
void ferNetDumpSVT(bor_net_t *m, FILE *out, const char *name);

/**
 * Dumps net as list of triangles.
 * One triangle per line (ax ay az bx by ...)
 */
void ferNetDumpTriangles(bor_net_t *m, FILE *out);

/**
 * Dumps net in povray format.
 */
void ferNetDumpPovray(bor_net_t *m, FILE *out);
#endif



/**** INLINES ****/
_fer_inline size_t ferNetNodeEdgesLen(const bor_net_node_t *v)
{
    return v->edges_len;
}

_fer_inline bor_list_t *ferNetNodeEdges(bor_net_node_t *v)
{
    return &v->edges;
}

_fer_inline int ferNetNodeHasEdge(const bor_net_node_t *v,
                                      const bor_net_edge_t *e)
{
    bor_list_t *item;
    bor_net_edge_t *edge;

    FER_LIST_FOR_EACH(&v->edges, item){
        edge = ferNetEdgeFromNodeList(item);
        if (edge == e)
            return 1;
    }

    return 0;
}





_fer_inline bor_net_node_t *ferNetEdgeNode(bor_net_edge_t *e, size_t i)
{
    return e->n[i];
}

_fer_inline int ferNetEdgeHasNode(const bor_net_edge_t *e,
                                   const bor_net_node_t *n)
{
    return e->n[0] == n || e->n[1] == n;
}

_fer_inline bor_net_node_t *ferNetEdgeOtherNode(bor_net_edge_t *e,
                                                  const bor_net_node_t *v)
{
    if (e->n[0] == v)
        return e->n[1];
    return e->n[0];
}

_fer_inline bor_net_edge_t *ferNetEdgeFromNodeList(bor_list_t *l)
{
    bor_list_m_t *m;
    bor_net_edge_t *e;

    m = ferListMFromList(l);
    e = FER_LIST_M_ENTRY(l, bor_net_edge_t, nlist, m->mark);

    return e;
}


_fer_inline size_t ferNetNodesLen(const bor_net_t *n)
{
    return n->nodes_len;
}

_fer_inline size_t ferNetEdgesLen(const bor_net_t *n)
{
    return n->edges_len;
}

_fer_inline bor_list_t *ferNetNodes(bor_net_t *n)
{
    return &n->nodes;
}

_fer_inline bor_list_t *ferNetEdges(bor_net_t *n)
{
    return &n->edges;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_NET_H__ */


