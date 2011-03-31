/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __FER_NET_H__
#define __FER_NET_H__

#include <fermat/core.h>
#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Net - Neural Network
 * =====================
 * TODO
 */

struct _fer_net_t {
    fer_list_t nodes; /*!< List of nodes */
    size_t nodes_len; /*!< Number of nodes in list */
    fer_list_t edges; /*!< List of edges */
    size_t edges_len; /*!< Number of edges in list */
};
typedef struct _fer_net_t fer_net_t;


struct _fer_net_node_t {
    fer_list_t list;     /*!< Edgeection into list of all nodes */

    fer_list_t edges; /*!< List of all incidenting edgeections */
    size_t edges_len; /*!< Number of edgeections in list */
};
typedef struct _fer_net_node_t fer_net_node_t;


struct _fer_net_edge_t {
    fer_net_node_t *n[2]; /*!< Start and end nodes of edge */
    fer_list_m_t nlist[2]; /*!< Edgeection into list of edgeections
                                incidenting with node.
                                .nlist[0] correspond with node .n[0] and
                                .nlist[1] with .n[1] */

    fer_list_t list;       /*!< Edgeection into list of all edges */
};
typedef struct _fer_net_edge_t fer_net_edge_t;


/**
 * Node
 * -------
 * TODO
 *
 * See fer_net_node_t.
 */

/**
 * Allocates and initializes new node.
 */
fer_net_node_t *ferNetNodeNew(void);

/**
 * Deletes node.
 */
void ferNetNodeDel(fer_net_node_t *v);

/**
 * Returns number of edgeections incidenting with given node.
 */
_fer_inline size_t ferNetNodeEdgesLen(const fer_net_node_t *v);

/**
 * Returns head of list of edgeections incidenting with node.
 */
_fer_inline fer_list_t *ferNetNodeEdges(fer_net_node_t *n);

/**
 * Returns true if node incidents with given edgeection.
 */
_fer_inline int ferNetNodeHasEdge(const fer_net_node_t *n,
                                   const fer_net_edge_t *e);

/**
 * Returns edgeection (first) edgeecting given pair of nodes.
 */
fer_net_edge_t *ferNetNodeCommonEdge(const fer_net_node_t *n1,
                                       const fer_net_node_t *n2);


/**
 * Edge
 * -----
 * TODO
 *
 * See fer_net_edge_t.
 */

/**
 * Allocates and initializes edgeection.
 */
fer_net_edge_t *ferNetEdgeNew(void);

/**
 * Deletes edgeection.
 */
void ferNetEdgeDel(fer_net_edge_t *e);

/**
 * Returns start or end node of edge.
 * Parameter i can be either 0 or 1 (no check is performed).
 */
_fer_inline fer_net_node_t *ferNetEdgeNode(fer_net_edge_t *e, size_t i);

/**
 * Returns true if edgeection incidents with given node.
 */
_fer_inline int ferNetEdgeHasNode(const fer_net_edge_t *e,
                                   const fer_net_node_t *n);

/**
 * Returns the other (start/end) node than provided.
 */
_fer_inline fer_net_node_t *ferNetEdgeOtherNode(fer_net_edge_t *e,
                                                  const fer_net_node_t *n);

/**
 * Returns true if given triplet of edgeections form triangle.
 */
int ferNetEdgeTriCheck(const fer_net_edge_t *e1,
                        const fer_net_edge_t *e2,
                        const fer_net_edge_t *e3);

/**
 * Return pointer of edge struct based on list item (pointer to nlist[0|1].
 */
_fer_inline fer_net_edge_t *ferNetEdgeFromNodeList(fer_list_t *l);




/**
 * Net
 * ----
 * TODO
 *
 * See fer_net_t.
 */

/**
 * Creates new empty network.
 */
fer_net_t *ferNetNew(void);

/**
 * Deletes network.
 * Warning: No nodes or edges are deleted because they were
 * allocated outside a net! See ferNetDel2() if you want more
 * sofisticated destructor.
 */
void ferNetDel(fer_net_t *m);

/**
 * Deletes net. This destructor can be used for deleting all nodes and
 * edges contained in net.
 *
 * Before freeing a net, destructor iterates over all nodes. Each
 * node is first disedgeected from net and then delnode is called with
 * second argument ndata. Similarly are iterated edges.
 */
void ferNetDel2(fer_net_t *m,
                 void (*delnode)(fer_net_node_t *, void *), void *ndata,
                 void (*deledge)(fer_net_edge_t *, void *), void *cdata);

/**
 * Returns number of nodes stored in net.
 */
_fer_inline size_t ferNetNodesLen(const fer_net_t *m);

/**
 * Returns number of edges stored in net.
 */
_fer_inline size_t ferNetEdgesLen(const fer_net_t *m);

/**
 * Returns list of nodes.
 */
_fer_inline fer_list_t *ferNetNodes(fer_net_t *m);

/**
 * Returns list of edgeections.
 */
_fer_inline fer_list_t *ferNetEdges(fer_net_t *m);

/**
 * Adds node into net.
 */
void ferNetAddNode(fer_net_t *m, fer_net_node_t *v);

/**
 * Removes node from net.
 * If node is edgeected with any edgeection, node can't be removed and -1
 * is returned. On success 0 is returned.
 */
int ferNetRemoveNode(fer_net_t *m, fer_net_node_t *v);

/**
 * Adds edgeection into net. Start and end points (nodes) must be provided.
 */
void ferNetAddEdge(fer_net_t *m, fer_net_edge_t *e,
                    fer_net_node_t *start, fer_net_node_t *end);

/**
 * Removes edgeection from net.
 */
void ferNetRemoveEdge(fer_net_t *m, fer_net_edge_t *e);


#if 0
/**
 * Dumps net as one object in SVT format.
 * See http://svt.danfis.cz for more info.
 */
void ferNetDumpSVT(fer_net_t *m, FILE *out, const char *name);

/**
 * Dumps net as list of triangles.
 * One triangle per line (ax ay az bx by ...)
 */
void ferNetDumpTriangles(fer_net_t *m, FILE *out);

/**
 * Dumps net in povray format.
 */
void ferNetDumpPovray(fer_net_t *m, FILE *out);
#endif



/**** INLINES ****/
_fer_inline size_t ferNetNodeEdgesLen(const fer_net_node_t *v)
{
    return v->edges_len;
}

_fer_inline fer_list_t *ferNetNodeEdges(fer_net_node_t *v)
{
    return &v->edges;
}

_fer_inline int ferNetNodeHasEdge(const fer_net_node_t *v,
                                      const fer_net_edge_t *e)
{
    fer_list_t *item;
    fer_net_edge_t *edge;

    FER_LIST_FOR_EACH(&v->edges, item){
        edge = ferNetEdgeFromNodeList(item);
        if (edge == e)
            return 1;
    }

    return 0;
}





_fer_inline fer_net_node_t *ferNetEdgeNode(fer_net_edge_t *e, size_t i)
{
    return e->n[i];
}

_fer_inline int ferNetEdgeHasNode(const fer_net_edge_t *e,
                                   const fer_net_node_t *n)
{
    return e->n[0] == n || e->n[1] == n;
}

_fer_inline fer_net_node_t *ferNetEdgeOtherNode(fer_net_edge_t *e,
                                                  const fer_net_node_t *v)
{
    if (e->n[0] == v)
        return e->n[1];
    return e->n[0];
}

_fer_inline fer_net_edge_t *ferNetEdgeFromNodeList(fer_list_t *l)
{
    fer_list_m_t *m;
    fer_net_edge_t *e;

    m = ferListMFromList(l);
    e = FER_LIST_ENTRY(l, fer_net_edge_t, nlist[m->mark]);

    return e;
}


_fer_inline size_t ferNetNodesLen(const fer_net_t *n)
{
    return n->nodes_len;
}

_fer_inline size_t ferNetEdgesLen(const fer_net_t *n)
{
    return n->edges_len;
}

_fer_inline fer_list_t *ferNetNodes(fer_net_t *n)
{
    return &n->nodes;
}

_fer_inline fer_list_t *ferNetEdges(fer_net_t *n)
{
    return &n->edges;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_NET_H__ */


