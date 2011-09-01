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

#ifndef __FER_GNG_T_H__
#define __FER_GNG_T_H__

#include <fermat/net.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Growing Neural Gas with Targeting (GNG-T)
 * ==========================================
 *
 * [1] H. Frezza-Buet, Following non-stationary distributions by
 *     controlling the vector quantization accuracy of a growing neural gas
 *     network, Neurocomputing 71 (7-9) (2008) 1191-1202.
 * 
 */


struct _fer_gngt_node_t {
    fer_net_node_t node;

    fer_real_t err; /*!< Overall error */
    int won;        /*!< True if node has won in last epoch */
};
typedef struct _fer_gngt_node_t fer_gngt_node_t;


struct _fer_gngt_edge_t {
    fer_net_edge_t edge;

    int age;
};
typedef struct _fer_gngt_edge_t fer_gngt_edge_t;



/**
 * GNG-T Operations
 * ---------------
 *
 * See fer_gngt_ops_t.
 */

/** vvvv */

/**
 * Return two nodes that will be used for network initialization.
 * If not specified (NULL) two random input signals are used for
 * initialization.
 */
typedef void (*fer_gngt_init)(fer_gngt_node_t **n1,
                              fer_gngt_node_t **n2,
                              void *);

/**
 * Create new node initialized with weight vector to input_signal.
 */
typedef fer_gngt_node_t *(*fer_gngt_new_node)(const void *input_signal, void *);

/**
 * Create a new node in between n1 and n2.
 */
typedef fer_gngt_node_t *(*fer_gngt_new_node_between)(const fer_gngt_node_t *n1,
                                                      const fer_gngt_node_t *n2,
                                                      void *);

/**
 * Deletes given node.
 */
typedef void (*fer_gngt_del_node)(fer_gngt_node_t *n, void *);

/**
 * Returns random input signal.
 */
typedef const void *(*fer_gngt_input_signal)(void *);

/**
 * Returns (via n1 and n2) first and second nearest node to input signal.
 */
typedef void (*fer_gngt_nearest)(const void *input_signal,
                                 fer_gngt_node_t **n1,
                                 fer_gngt_node_t **n2,
                                 void *);

/**
 * Returns squared distance between input_signal and node.
 */
typedef fer_real_t (*fer_gngt_dist2)(const void *input_signal,
                                     const fer_gngt_node_t *node, void *);

/**
 * Move given node towards input_signal by given fraction.
 *
 * If position on node is w and position of input_signal is v, then:
 * w = w + ((v - w) * fraction)
 */
typedef void (*fer_gngt_move_towards)(fer_gngt_node_t *node,
                                      const void *input_signal,
                                      fer_real_t fraction,
                                      void *);

/**
 * Returns true if algorithm should terminate.
 * This is called at the end of each epoch.
 */
typedef int (*fer_gngt_terminate)(void *);

/**
 * Callback that is peridically called from GNG-T.
 *
 * It is called every .callback_period'th added node.
 */
typedef void (*fer_gngt_callback)(void *);

/** ^^^^ */

struct _fer_gngt_ops_t {
    fer_gngt_init             init;
    fer_gngt_new_node         new_node;
    fer_gngt_new_node_between new_node_between;
    fer_gngt_del_node         del_node;
    fer_gngt_input_signal     input_signal;
    fer_gngt_nearest          nearest;
    fer_gngt_dist2            dist2;
    fer_gngt_move_towards     move_towards;
    fer_gngt_terminate        terminate;

    fer_gngt_callback callback;
    unsigned long callback_period;

    void *data; /*!< Data pointer that will be provided to all callbacks if
                     not specified otherwise. */

    void *init_data;
    void *new_node_data;
    void *new_node_between_data;
    void *del_node_data;
    void *input_signal_data;
    void *nearest_data;
    void *dist2_data;
    void *move_towards_data;
    void *terminate_data;
    void *callback_data;
};
typedef struct _fer_gngt_ops_t fer_gngt_ops_t;


/**
 * Initializes ops struct to NULL values.
 */
void ferGNGTOpsInit(fer_gngt_ops_t *ops);



/**
 * GNG-T Parameters
 * -----------------
 */
struct _fer_gngt_params_t {
    size_t lambda;     /*!< Number of adaptation steps. Default: 200 */
    fer_real_t eb;     /*!< Winner node learning rate. Default: 0.05 */
    fer_real_t en;     /*!< Winners' neighbors learning rate. Default: 0.0006 */
    int age_max;       /*!< Maximal age of edge. Default: 200 */
    fer_real_t target; /*!< Target average error. Default: 100 */
};
typedef struct _fer_gngt_params_t fer_gngt_params_t;

/**
 * Initializes params struct to default values.
 */
void ferGNGTParamsInit(fer_gngt_params_t *params);



/**
 * GNG-T Algorithm
 * ----------------
 *
 * See fer_gngt_t.
 */

struct _fer_gngt_t {
    fer_net_t *net;
    fer_gngt_ops_t ops;
    fer_gngt_params_t params;

    fer_real_t avg_err; /*!< Last computed average error */
};
typedef struct _fer_gngt_t fer_gngt_t;


/**
 * Creates new instance of GNG algorithm.
 */
fer_gngt_t *ferGNGTNew(const fer_gngt_ops_t *ops,
                       const fer_gngt_params_t *params);

/**
 * Deletes GNG.
 */
void ferGNGTDel(fer_gngt_t *gng);

/**
 * Runs GNG algorithm.
 *
 * This runs whole algorithm in loop until operation terminate() returns
 * true:
 * ~~~~~~
 * ferGNGTInit()
 * do:
 *     ferGNGTReset()
 *     for (step = 1 .. params.lambda):
 *         ferGNGTAdapt()
 *     ferGNGTGrowShrink()
 * while not ops.terminate()
 */
void ferGNGTRun(fer_gngt_t *gng);


/**
 * Initialize gng net.
 *
 * ~~~~~
 * if ops.init != NULL:
 *     ops.init()
 * else:
 *     is = ops.input_signal()
 *     n1 = ops.new_node(is)
 *
 *     is = ops.input_signal()
 *     n2 = ops.new_node(is)
 * create edge between n1 and n2
 */
void ferGNGTInit(fer_gngt_t *gng);

/**
 * Resets errors of all nodes to zero.
 */
void ferGNGTReset(fer_gngt_t *gng);

/**
 * One competitive hebbian learning step.
 *
 * ~~~~
 * is = ops.input_signal()
 * n, m = ops.nearest()
 * refresh edge between n and m
 * increment age of all edges emanating from n by one
 * remove all edges with age > age_max
 * update n's error: e = e + ops.dist2(n, is)
 * update weights of nodes:
 *      wn = wn + eb * (is - wn)
 *      wi = wi + en * (is - wi) for neighbors of n
 */
void ferGNGTAdapt(fer_gngt_t *gng);

/**
 * Compares target error with average error and creates or deletes a node
 * according to it.
 */
void ferGNGTGrowShrink(fer_gngt_t *gng);

/**
 * Returns last computed average error
 */
_fer_inline fer_real_t ferGNGTAvgErr(const fer_gngt_t *gng);


/**
 * Net Related API
 * ----------------
 *
 * See fer_gngt_node_t.
 * See fer_gngt_edge_t.
 */

/**
 * Returns net of nodes.
 */
_fer_inline fer_net_t *ferGNGTNet(fer_gngt_t *gng);

/**
 * Returns list of nodes.
 */
_fer_inline fer_list_t *ferGNGTNodes(fer_gngt_t *gng);

/**
 * Returns number of nodes in net.
 */
_fer_inline size_t ferGNGTNodesLen(const fer_gngt_t *gng);

/**
 * Returns list of edges.
 */
_fer_inline fer_list_t *ferGNGTNodes(fer_gngt_t *gng);

/**
 * Returns number of edges in net.
 */
_fer_inline size_t ferGNGTEdgesLen(const fer_gngt_t *gng);

/**
 * Returns list of nodes
 */
_fer_inline fer_list_t *ferGNGTEdges(fer_gngt_t *gng);

/**
 * Returns GNG node from list pointer.
 *
 * Usage:
 * ~~~~~
 * fer_list_t *list, *item;
 * fer_gngt_node_t *n;
 *
 * list = ferGNGTNodes(gng);
 * FER_LIST_FOR_EACH(list, item){
 *     n = ferGNGTNodeFromList(item);
 *     ....
 * }
 */
_fer_inline fer_gngt_node_t *ferGNGTNodeFromList(fer_list_t *item);

/**
 * Similar to *ferGNGTNodeFromList()* but works with nodes.
 */
_fer_inline fer_gngt_edge_t *ferGNGTEdgeFromList(fer_list_t *item);

/**
 * Cast Net node to GNG node.
 */
_fer_inline fer_gngt_node_t *ferGNGTNodeFromNet(fer_net_node_t *n);

/**
 * Cast Net edge to GNG edge.
 */
_fer_inline fer_gngt_edge_t *ferGNGTEdgeFromNet(fer_net_edge_t *e);

/**
 * Cast GNG node to Net node.
 */
_fer_inline fer_net_node_t *ferGNGTNodeToNet(fer_gngt_node_t *n);

/**
 * Cast GNG edge to Net edge.
 */
_fer_inline fer_net_edge_t *ferGNGTEdgeToNet(fer_gngt_edge_t *e);



/**
 * Node API
 * ^^^^^^^^^
 *
 * See fer_gngt_node_t.
 */

/**
 * Adds node into network
 */
_fer_inline void ferGNGTNodeAdd(fer_gngt_t *gng, fer_gngt_node_t *n);

/**
 * Removes node from network
 */
_fer_inline void ferGNGTNodeRemove(fer_gngt_t *gng, fer_gngt_node_t *n);

/**
 * Removes node from network and deletes it (ops.del_node is used).
 */
_fer_inline void ferGNGTNodeDel(fer_gngt_t *gng, fer_gngt_node_t *n);

/**
 * Disconnects node from net, i.e., deletes all incidenting edges.
 */
void ferGNGTNodeDisconnect(fer_gngt_t *gng, fer_gngt_node_t *n);

/**
 * Connects new node at given position (is) and connects it with two
 * nearest nodes [ops.new_node(), ops.nearest()].
 */
fer_gngt_node_t *ferGNGTNodeNewAtPos(fer_gngt_t *gng, const void *is);




/**
 * Edge API
 * ^^^^^^^^^
 *
 * See fer_gngt_edge_t.
 */

/**
 * Creates and initializes new edge between {n1} and {n2}.
 */
fer_gngt_edge_t *ferGNGTEdgeNew(fer_gngt_t *gng, fer_gngt_node_t *n1,
                                                 fer_gngt_node_t *n2);

/**
 * Deletes edge
 */
void ferGNGTEdgeDel(fer_gngt_t *gng, fer_gngt_edge_t *edge);

/**
 * Returns age of edge.
 *
 * Always use this function instead of direct access to struct!
 */
_fer_inline int ferGNGTEdgeAge(const fer_gngt_t *gng, const fer_gngt_edge_t *edge);


/**
 * Returns edge connecting {n1} and {n2}.
 */
_fer_inline fer_gngt_edge_t *ferGNGTEdgeBetween(fer_gngt_t *gng,
                                                fer_gngt_node_t *n1,
                                                fer_gngt_node_t *n2);

/**
 * Deletes edge between {n1} and {n2}.
 */
void ferGNGTEdgeBetweenDel(fer_gngt_t *gng,
                           fer_gngt_node_t *n1, fer_gngt_node_t *n2);

/**
 * Returns (via {n1} and {n2}) incidenting nodes of edge
 */
_fer_inline void ferGNGTEdgeNodes(fer_gngt_edge_t *e,
                                  fer_gngt_node_t **n1, fer_gngt_node_t **n2);





/**** INLINES ****/
_fer_inline fer_real_t ferGNGTAvgErr(const fer_gngt_t *gng)
{
    return gng->avg_err;
}

_fer_inline fer_net_t *ferGNGTNet(fer_gngt_t *gng)
{
    return gng->net;
}

_fer_inline fer_list_t *ferGNGTNodes(fer_gngt_t *gng)
{
    return ferNetNodes(gng->net);
}

_fer_inline size_t ferGNGTNodesLen(const fer_gngt_t *gng)
{
    return ferNetNodesLen(gng->net);
}

_fer_inline fer_list_t *ferGNGTEdges(fer_gngt_t *gng)
{
    return ferNetEdges(gng->net);
}

_fer_inline size_t ferGNGTEdgesLen(const fer_gngt_t *gng)
{
    return ferNetEdgesLen(gng->net);
}

_fer_inline fer_gngt_node_t *ferGNGTNodeFromList(fer_list_t *item)
{
    fer_net_node_t *nn;
    fer_gngt_node_t *n;

    nn = FER_LIST_ENTRY(item, fer_net_node_t, list);
    n  = fer_container_of(nn, fer_gngt_node_t, node);
    return n;
}

_fer_inline fer_gngt_edge_t *ferGNGTEdgeFromList(fer_list_t *item)
{
    fer_net_edge_t *nn;
    fer_gngt_edge_t *n;

    nn = FER_LIST_ENTRY(item, fer_net_edge_t, list);
    n  = fer_container_of(nn, fer_gngt_edge_t, edge);
    return n;
}

_fer_inline fer_gngt_node_t *ferGNGTNodeFromNet(fer_net_node_t *n)
{
    return fer_container_of(n, fer_gngt_node_t, node);
}

_fer_inline fer_gngt_edge_t *ferGNGTEdgeFromNet(fer_net_edge_t *e)
{
    return fer_container_of(e, fer_gngt_edge_t, edge);
}

_fer_inline fer_net_node_t *ferGNGTNodeToNet(fer_gngt_node_t *n)
{
    return &n->node;
}

_fer_inline fer_net_edge_t *ferGNGTEdgeToNet(fer_gngt_edge_t *e)
{
    return &e->edge;
}



_fer_inline void ferGNGTNodeAdd(fer_gngt_t *gng, fer_gngt_node_t *n)
{
    n->err = FER_ZERO;
    n->won = 0;
    ferNetAddNode(gng->net, &n->node);
}

_fer_inline void ferGNGTNodeRemove(fer_gngt_t *gng, fer_gngt_node_t *n)
{
    if (ferNetNodeEdgesLen(&n->node) != 0)
        ferGNGTNodeDisconnect(gng, n);
    ferNetRemoveNode(gng->net, &n->node);
}

_fer_inline void ferGNGTNodeDel(fer_gngt_t *gng, fer_gngt_node_t *n)
{
    ferGNGTNodeRemove(gng, n);
    gng->ops.del_node(n, gng->ops.del_node_data);
}


_fer_inline int ferGNGTEdgeAge(const fer_gngt_t *gng, const fer_gngt_edge_t *edge)
{
    return edge->age;
}

_fer_inline fer_gngt_edge_t *ferGNGTEdgeBetween(fer_gngt_t *gng,
                                              fer_gngt_node_t *n1,
                                              fer_gngt_node_t *n2)
{
    fer_net_edge_t *ne;
    fer_gngt_edge_t *e = NULL;

    ne = ferNetNodeCommonEdge(&n1->node, &n2->node);
    if (ne)
        e  = fer_container_of(ne, fer_gngt_edge_t, edge);
    return e;
}

_fer_inline void ferGNGTEdgeNodes(fer_gngt_edge_t *e,
                                 fer_gngt_node_t **n1, fer_gngt_node_t **n2)
{
    fer_net_node_t *n;

    n   = ferNetEdgeNode(&e->edge, 0);
    *n1 = fer_container_of(n, fer_gngt_node_t, node);

    n   = ferNetEdgeNode(&e->edge, 1);
    *n2 = fer_container_of(n, fer_gngt_node_t, node);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GNG_T_H__ */


