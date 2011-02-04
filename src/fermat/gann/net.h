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
    fer_list_t conns; /*!< List of conns */
    size_t conns_len; /*!< Number of conns in list */
};
typedef struct _gann_net_t gann_net_t;


struct _gann_net_node_t {
    fer_list_t list;     /*!< Connection into list of all nodes */

    fer_list_t conns; /*!< List of all incidenting connections */
    size_t conns_len; /*!< Number of connections in list */
};
typedef struct _gann_net_node_t gann_net_node_t;


struct _gann_net_conn_t {
    gann_net_node_t *n[2]; /*!< Start and end nodes of conn */
    fer_list_m_t nlist[2]; /*!< Connection into list of connections
                                incidenting with node.
                                .nlist[0] correspond with node .n[0] and
                                .nlist[1] with .n[1] */

    fer_list_t list;       /*!< Connection into list of all conns */
};
typedef struct _gann_net_conn_t gann_net_conn_t;


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
 * Returns number of connections incidenting with given node.
 */
_fer_inline size_t gannNetNodeConnsLen(const gann_net_node_t *v);

/**
 * Returns head of list of connections incidenting with node.
 */
_fer_inline fer_list_t *gannNetNodeConns(gann_net_node_t *n);

/**
 * Returns true if node incidents with given connection.
 */
_fer_inline int gannNetNodeHasConn(const gann_net_node_t *n,
                                   const gann_net_conn_t *e);

/**
 * Returns connection (first) connecting given pair of nodes.
 */
gann_net_conn_t *gannNetNodeCommonConn(const gann_net_node_t *n1,
                                       const gann_net_node_t *n2);


/**
 * Conn
 * -----
 * TODO
 */

/**
 * Allocates and initializes connection.
 */
gann_net_conn_t *gannNetConnNew(void);

/**
 * Deletes connection.
 */
void gannNetConnDel(gann_net_conn_t *e);

/**
 * Returns start or end node of conn.
 * Parameter i can be either 0 or 1 (no check is performed).
 */
_fer_inline gann_net_node_t *gannNetConnNode(gann_net_conn_t *e, size_t i);

/**
 * Returns true if connection incidents with given node.
 */
_fer_inline int gannNetConnHasNode(const gann_net_conn_t *e,
                                   const gann_net_node_t *n);

/**
 * Returns the other (start/end) node than provided.
 */
_fer_inline gann_net_node_t *gannNetConnOtherNode(gann_net_conn_t *e,
                                                  const gann_net_node_t *n);

/**
 * Returns true if given triplet of connections form triangle.
 */
int gannNetConnTriCheck(const gann_net_conn_t *e1,
                        const gann_net_conn_t *e2,
                        const gann_net_conn_t *e3);

/**
 * Return pointer of conn struct based on list item (pointer to nlist[0|1].
 */
_fer_inline gann_net_conn_t *gannNetConnFromNodeList(fer_list_t *l);




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
 * Warning: No nodes or conns are deleted because they were
 * allocated outside a net! See gannNetDel2() if you want more
 * sofisticated destructor.
 */
void gannNetDel(gann_net_t *m);

/**
 * Deletes net. This destructor can be used for deleting all nodes and
 * conns contained in net.
 *
 * Before freeing a net, destructor iterates over all nodes. Each
 * node is first disconnected from net and then delnode is called with
 * second argument ndata. Similarly are iterated conns.
 */
void gannNetDel2(gann_net_t *m,
                 void (*delnode)(gann_net_node_t *, void *), void *ndata,
                 void (*delconn)(gann_net_conn_t *, void *), void *cdata);

/**
 * Returns number of nodes stored in net.
 */
_fer_inline size_t gannNetNodesLen(const gann_net_t *m);

/**
 * Returns number of conns stored in net.
 */
_fer_inline size_t gannNetConnsLen(const gann_net_t *m);

/**
 * Returns list of nodes.
 */
_fer_inline fer_list_t *gannNetNodes(gann_net_t *m);

/**
 * Returns list of connections.
 */
_fer_inline fer_list_t *gannNetConns(gann_net_t *m);

/**
 * Adds node into net.
 */
void gannNetAddNode(gann_net_t *m, gann_net_node_t *v);

/**
 * Removes node from net.
 * If node is connected with any connection, node can't be removed and -1
 * is returned. On success 0 is returned.
 */
int gannNetRemoveNode(gann_net_t *m, gann_net_node_t *v);

/**
 * Adds connection into net. Start and end points (nodes) must be provided.
 */
void gannNetAddConn(gann_net_t *m, gann_net_conn_t *e,
                    gann_net_node_t *start, gann_net_node_t *end);

/**
 * Removes connection from net.
 */
void gannNetRemoveConn(gann_net_t *m, gann_net_conn_t *e);


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
_fer_inline size_t gannNetNodeConnsLen(const gann_net_node_t *v)
{
    return v->conns_len;
}

_fer_inline fer_list_t *gannNetNodeConns(gann_net_node_t *v)
{
    return &v->conns;
}

_fer_inline int gannNetNodeHasConn(const gann_net_node_t *v,
                                      const gann_net_conn_t *e)
{
    fer_list_t *item;
    gann_net_conn_t *conn;

    ferListForEach(&v->conns, item){
        conn = gannNetConnFromNodeList(item);
        if (conn == e)
            return 1;
    }

    return 0;
}





_fer_inline gann_net_node_t *gannNetConnNode(gann_net_conn_t *e, size_t i)
{
    return e->n[i];
}

_fer_inline int gannNetConnHasNode(const gann_net_conn_t *e,
                                   const gann_net_node_t *n)
{
    return e->n[0] == n || e->n[1] == n;
}

_fer_inline gann_net_node_t *gannNetConnOtherNode(gann_net_conn_t *e,
                                                  const gann_net_node_t *v)
{
    if (e->n[0] == v)
        return e->n[1];
    return e->n[0];
}

_fer_inline gann_net_conn_t *gannNetConnFromNodeList(fer_list_t *l)
{
    fer_list_m_t *m;
    gann_net_conn_t *e;

    m = ferListMFromList(l);
    e = ferListEntry(l, gann_net_conn_t, nlist[m->mark]);

    return e;
}


_fer_inline size_t gannNetNodesLen(const gann_net_t *n)
{
    return n->nodes_len;
}

_fer_inline size_t gannNetConnsLen(const gann_net_t *n)
{
    return n->conns_len;
}

_fer_inline fer_list_t *gannNetNodes(gann_net_t *n)
{
    return &n->nodes;
}

_fer_inline fer_list_t *gannNetConns(gann_net_t *n)
{
    return &n->conns;
}

#endif /* __FER_GANN_NET_H__ */


