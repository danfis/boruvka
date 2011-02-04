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

#ifndef __FER_GANN_NET_H__
#define __FER_GANN_NET_H__

#include <fermat/core.h>
#include <fermat/list.h>

struct _gann_net_t {
    fer_list_t nodes; /*!< List of nodes */
    size_t nodes_len; /*!< Number of nodes in list */
    fer_list_t edges; /*!< List of edges */
    size_t edges_len; /*!< Number of edges in list */
};
typedef struct _gann_net_t gann_net_t;


struct _gann_net_node_t {
    fer_list_t list;     /*!< Edgeection into list of all nodes */

    fer_list_t edges; /*!< List of all incidenting edgeections */
    size_t edges_len; /*!< Number of edgeections in list */
};
typedef struct _gann_net_node_t gann_net_node_t;


struct _gann_net_edge_t {
    gann_net_node_t *n[2]; /*!< Start and end nodes of edge */
    fer_list_m_t nlist[2]; /*!< Edgeection into list of edgeections
                                incidenting with node.
                                .nlist[0] correspond with node .n[0] and
                                .nlist[1] with .n[1] */

    fer_list_t list;       /*!< Edgeection into list of all edges */
};
typedef struct _gann_net_edge_t gann_net_edge_t;


/**
 * Node
 * -------
 * TODO
 */

/**
 * Allocates and initializes new node.
 */
gann_net_node_t *gannNetNodeNew(void);

/**
 * Deletes node.
 */
void gannNetNodeDel(gann_net_node_t *v);

/**
 * Returns number of edgeections incidenting with given node.
 */
_fer_inline size_t gannNetNodeEdgesLen(const gann_net_node_t *v);

/**
 * Returns head of list of edgeections incidenting with node.
 */
_fer_inline fer_list_t *gannNetNodeEdges(gann_net_node_t *n);

/**
 * Returns true if node incidents with given edgeection.
 */
_fer_inline int gannNetNodeHasEdge(const gann_net_node_t *n,
                                   const gann_net_edge_t *e);

/**
 * Returns edgeection (first) edgeecting given pair of nodes.
 */
gann_net_edge_t *gannNetNodeCommonEdge(const gann_net_node_t *n1,
                                       const gann_net_node_t *n2);


/**
 * Edge
 * -----
 * TODO
 */

/**
 * Allocates and initializes edgeection.
 */
gann_net_edge_t *gannNetEdgeNew(void);

/**
 * Deletes edgeection.
 */
void gannNetEdgeDel(gann_net_edge_t *e);

/**
 * Returns start or end node of edge.
 * Parameter i can be either 0 or 1 (no check is performed).
 */
_fer_inline gann_net_node_t *gannNetEdgeNode(gann_net_edge_t *e, size_t i);

/**
 * Returns true if edgeection incidents with given node.
 */
_fer_inline int gannNetEdgeHasNode(const gann_net_edge_t *e,
                                   const gann_net_node_t *n);

/**
 * Returns the other (start/end) node than provided.
 */
_fer_inline gann_net_node_t *gannNetEdgeOtherNode(gann_net_edge_t *e,
                                                  const gann_net_node_t *n);

/**
 * Returns true if given triplet of edgeections form triangle.
 */
int gannNetEdgeTriCheck(const gann_net_edge_t *e1,
                        const gann_net_edge_t *e2,
                        const gann_net_edge_t *e3);

/**
 * Return pointer of edge struct based on list item (pointer to nlist[0|1].
 */
_fer_inline gann_net_edge_t *gannNetEdgeFromNodeList(fer_list_t *l);




/**
 * Network
 * --------
 * TODO
 */

/**
 * Creates new empty network.
 */
gann_net_t *gannNetNew(void);

/**
 * Deletes network.
 * Warning: No nodes or edges are deleted because they were
 * allocated outside a net! See gannNetDel2() if you want more
 * sofisticated destructor.
 */
void gannNetDel(gann_net_t *m);

/**
 * Deletes net. This destructor can be used for deleting all nodes and
 * edges contained in net.
 *
 * Before freeing a net, destructor iterates over all nodes. Each
 * node is first disedgeected from net and then delnode is called with
 * second argument ndata. Similarly are iterated edges.
 */
void gannNetDel2(gann_net_t *m,
                 void (*delnode)(gann_net_node_t *, void *), void *ndata,
                 void (*deledge)(gann_net_edge_t *, void *), void *cdata);

/**
 * Returns number of nodes stored in net.
 */
_fer_inline size_t gannNetNodesLen(const gann_net_t *m);

/**
 * Returns number of edges stored in net.
 */
_fer_inline size_t gannNetEdgesLen(const gann_net_t *m);

/**
 * Returns list of nodes.
 */
_fer_inline fer_list_t *gannNetNodes(gann_net_t *m);

/**
 * Returns list of edgeections.
 */
_fer_inline fer_list_t *gannNetEdges(gann_net_t *m);

/**
 * Adds node into net.
 */
void gannNetAddNode(gann_net_t *m, gann_net_node_t *v);

/**
 * Removes node from net.
 * If node is edgeected with any edgeection, node can't be removed and -1
 * is returned. On success 0 is returned.
 */
int gannNetRemoveNode(gann_net_t *m, gann_net_node_t *v);

/**
 * Adds edgeection into net. Start and end points (nodes) must be provided.
 */
void gannNetAddEdge(gann_net_t *m, gann_net_edge_t *e,
                    gann_net_node_t *start, gann_net_node_t *end);

/**
 * Removes edgeection from net.
 */
void gannNetRemoveEdge(gann_net_t *m, gann_net_edge_t *e);


#if 0
/**
 * Dumps net as one object in SVT format.
 * See http://svt.danfis.cz for more info.
 */
void gannNetDumpSVT(gann_net_t *m, FILE *out, const char *name);

/**
 * Dumps net as list of triangles.
 * One triangle per line (ax ay az bx by ...)
 */
void gannNetDumpTriangles(gann_net_t *m, FILE *out);

/**
 * Dumps net in povray format.
 */
void gannNetDumpPovray(gann_net_t *m, FILE *out);
#endif



/**** INLINES ****/
_fer_inline size_t gannNetNodeEdgesLen(const gann_net_node_t *v)
{
    return v->edges_len;
}

_fer_inline fer_list_t *gannNetNodeEdges(gann_net_node_t *v)
{
    return &v->edges;
}

_fer_inline int gannNetNodeHasEdge(const gann_net_node_t *v,
                                      const gann_net_edge_t *e)
{
    fer_list_t *item;
    gann_net_edge_t *edge;

    ferListForEach(&v->edges, item){
        edge = gannNetEdgeFromNodeList(item);
        if (edge == e)
            return 1;
    }

    return 0;
}





_fer_inline gann_net_node_t *gannNetEdgeNode(gann_net_edge_t *e, size_t i)
{
    return e->n[i];
}

_fer_inline int gannNetEdgeHasNode(const gann_net_edge_t *e,
                                   const gann_net_node_t *n)
{
    return e->n[0] == n || e->n[1] == n;
}

_fer_inline gann_net_node_t *gannNetEdgeOtherNode(gann_net_edge_t *e,
                                                  const gann_net_node_t *v)
{
    if (e->n[0] == v)
        return e->n[1];
    return e->n[0];
}

_fer_inline gann_net_edge_t *gannNetEdgeFromNodeList(fer_list_t *l)
{
    fer_list_m_t *m;
    gann_net_edge_t *e;

    m = ferListMFromList(l);
    e = ferListEntry(l, gann_net_edge_t, nlist[m->mark]);

    return e;
}


_fer_inline size_t gannNetNodesLen(const gann_net_t *n)
{
    return n->nodes_len;
}

_fer_inline size_t gannNetEdgesLen(const gann_net_t *n)
{
    return n->edges_len;
}

_fer_inline fer_list_t *gannNetNodes(gann_net_t *n)
{
    return &n->nodes;
}

_fer_inline fer_list_t *gannNetEdges(gann_net_t *n)
{
    return &n->edges;
}

#endif /* __FER_GANN_NET_H__ */


