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

#ifndef __FER_GNG_H__
#define __FER_GNG_H__

#include <fermat/net.h>
#include <fermat/pairheap.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Growing Neural Gas
 * ===================
 * Generalized implementation of Growing Neural Gas algorithm as was
 * described in:
 *
 *      B. Fritzke, "A growing neural gas network learns topologies,"
 *      Neural Inf. Process. Syst., vol. 7, pp. 625­632, 1995.
 *
 *
 * This implementation is not limited to 2-D, 3-D or any other dimensions.
 * In fact, nodes doesn't have assigned any weight vector - it is user's
 * responsibility to provide that.
 *
 * User must define several callbacks and fill *fer_gng_ops_t* structure,
 * ferGNG*() functions take care of the core of algorithm.
 *
 *
 * | **Algorithm works as follows:**
 * |
 * | 1. Initializes network by two random nodes. [*ferGNGInit()*]
 * | 2. Check termination condition
 * | 3. Learn topology. ['ferGNGLearn()*]
 * |     1. Get random input signal
 * |     2. Find two nearest nodes to input signal - {n1}, {n2}
 * |     3. Create connection between {n1} and {n2} if doesn't exist and set age
 * |        to zero
 * |     4. Increase error counter of winner node.
 * |     5. Adapt nodes to input signal using fractions {eb} and {en}
 * |     6. Increment age of all edges that incident with winner node by one
 * |     7. Remove all edges with age higher than {age_max}
 * | 4. If the number of input signals presented so far to the network is an
 * |    integer multiple of the parameter {lambda}, create new node. [*ferGNGNewNode()*]
 * |     1. Get node with highest error counter -> {q}
 * |     2. Get {q}'s neighbor node with highest error counter -> {f}
 * |     3. Create new node between {q} and {f} -> {r}
 * |     4. Create {q-r} and {f-r} edges and delete {q-f} edge.
 * |     5. Decrease error counter of {q} and {f} ({alpha} parameter).
 * |     6. Set error counter of {r} as average error counter of {q} and {f}.
 * | 5. Decrease error counters of all nodes [*ferGNGDecreaseErrCounters()*]
 * | 6. Go to 2.
 */


struct _fer_gng_node_t {
    fer_net_node_t node;

    fer_real_t err;               /*!< Overall error */
    unsigned long err_cycle;      /*!< Last cycle in which were .err changed */
    fer_pairheap_node_t err_heap; /*!< Connection to error heap */
};
typedef struct _fer_gng_node_t fer_gng_node_t;


struct _fer_gng_edge_t {
    fer_net_edge_t edge;

    int age;
};
typedef struct _fer_gng_edge_t fer_gng_edge_t;



/**
 * GNG Operations
 * ---------------
 *
 * See fer_gng_ops_t.
 */

/** vvvv */

/**
 * Return two nodes that will be used for network initialization.
 * If not specified (NULL) two random input ignals are use for
 * initialization.
 */
typedef void (*fer_gng_init)(fer_gng_node_t **n1,
                             fer_gng_node_t **n2,
                             void *);

/**
 * Create new node initialized weight vector to input_signal.
 */
typedef fer_gng_node_t *(*fer_gng_new_node)(const void *input_signal, void *);

/**
 * Create new node in between n1 and n2.
 */
typedef fer_gng_node_t *(*fer_gng_new_node_between)(const fer_gng_node_t *n1,
                                                    const fer_gng_node_t *n2,
                                                    void *);

/**
 * Deletes given node.
 */
typedef void (*fer_gng_del_node)(fer_gng_node_t *n, void *);

/**
 * Returns random input signal.
 */
typedef const void *(*fer_gng_input_signal)(void *);

/**
 * Returns (via n1 and n2) first and second nearest node to input signal.
 */
typedef void (*fer_gng_nearest)(const void *input_signal,
                                fer_gng_node_t **n1,
                                fer_gng_node_t **n2,
                                void *);

/**
 * Returns squared distance between input_signal and node.
 */
typedef fer_real_t (*fer_gng_dist2)(const void *input_signal,
                                    const fer_gng_node_t *node, void *);

/**
 * Move given node towards input_signal by given fraction.
 *
 * If position on node is w and position of input_signal is v, then:
 * w = w + ((v - w) * fraction)
 */
typedef void (*fer_gng_move_towards)(fer_gng_node_t *node,
                                     const void *input_signal,
                                     fer_real_t fraction,
                                     void *);

/**
 * Returns true if algorithm should terminate.
 */
typedef int (*fer_gng_terminate)(void *);

/**
 * Callback that is peridically called from GNG.
 *
 * It is called every .callback_period'th added node.
 */
typedef void (*fer_gng_callback)(void *);

/** ^^^^ */

struct _fer_gng_ops_t {
    fer_gng_init             init;
    fer_gng_new_node         new_node;
    fer_gng_new_node_between new_node_between;
    fer_gng_del_node         del_node;
    fer_gng_input_signal     input_signal;
    fer_gng_nearest          nearest;
    fer_gng_dist2            dist2;
    fer_gng_move_towards     move_towards;
    fer_gng_terminate        terminate;

    fer_gng_callback callback;
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
typedef struct _fer_gng_ops_t fer_gng_ops_t;


/**
 * Initializes ops struct to NULL values.
 */
void ferGNGOpsInit(fer_gng_ops_t *ops);



/**
 * GNG Parameters
 * ---------------
 */
struct _fer_gng_params_t {
    size_t lambda;    /*!< Number of steps between adding nodes */
    fer_real_t eb;    /*!< Winner node learning rate */
    fer_real_t en;    /*!< Winners' neighbors learning rate */
    fer_real_t alpha; /*!< Decrease error counter rate */
    fer_real_t beta;  /*!< Decrease error counter rate for all nodes */
    int age_max;      /*!< Maximal age of edge */
};
typedef struct _fer_gng_params_t fer_gng_params_t;

/**
 * Initializes params struct to default values.
 */
void ferGNGParamsInit(fer_gng_params_t *params);



/**
 * GNG Algorithm
 * --------------
 *
 * See fer_gng_t.
 */

struct _fer_gng_t {
    fer_net_t *net;
    fer_pairheap_t *err_heap;

    fer_gng_ops_t ops;
    fer_gng_params_t params;

    fer_real_t *beta_n; /*!< Precomputed beta^n for n = 1, ..., lambda */
    fer_real_t *beta_lambda_n; /*!< Precomputed beta^(n*lambda) */
    size_t beta_lambda_n_len;

    size_t step;
    unsigned long cycle;
};
typedef struct _fer_gng_t fer_gng_t;


/**
 * Creates new instance of GNG algorithm.
 */
fer_gng_t *ferGNGNew(const fer_gng_ops_t *ops,
                     const fer_gng_params_t *params);

/**
 * Deletes GNG.
 */
void ferGNGDel(fer_gng_t *gng);

/**
 * Runs GNG algorithm.
 *
 * This runs whole algorithm in loop until operation terminate() returns
 * true.
 */
void ferGNGRun(fer_gng_t *gng);



/**
 * Net Related API
 * ----------------
 *
 * See fer_gng_node_t.
 * See fer_gng_edge_t.
 */

/**
 * Connects new node at given position (is).
 */
fer_gng_node_t *ferGNGConnectNewNode(fer_gng_t *gng, const void *is);

/**
 * Returns age of edge.
 *
 * Always use this function instead of direct access to struct!
 */
int ferGNGEdgeAge(const fer_gng_t *gng, const fer_gng_edge_t *edge);

/**
 * Returns net of nodes.
 */
_fer_inline fer_net_t *ferGNGNet(fer_gng_t *gng);

/**
 * Returns list of nodes.
 */
_fer_inline fer_list_t *ferGNGNodes(fer_gng_t *gng);

/**
 * Returns number of nodes in net.
 */
_fer_inline size_t ferGNGNodesLen(const fer_gng_t *gng);

/**
 * Returns list of edges.
 */
_fer_inline fer_list_t *ferGNGNodes(fer_gng_t *gng);

/**
 * Returns number of edges in net.
 */
_fer_inline size_t ferGNGEdgesLen(const fer_gng_t *gng);

/**
 * Returns GNG node from list pointer.
 *
 * Usage:
 * ~~~~~
 * fer_list_t *list, *item;
 * fer_gng_node_t *n;
 *
 * list = ferGNGNodes(gng);
 * ferListForEach(list, item){
 *     n = ferGNGNodeFromList(item);
 *     ....
 * }
 */
_fer_inline fer_gng_node_t *ferGNGNodeFromList(fer_list_t *item);

/**
 * Similar to *ferGNGNodeFromList()* but works with nodes.
 */
_fer_inline fer_gng_edge_t *ferGNGEdgeFromList(fer_list_t *item);

/**
 * Cast Net node to GNG node.
 */
_fer_inline fer_gng_node_t *ferGNGNodeFromNet(fer_net_node_t *n);

/**
 * Cast Net edge to GNG edge.
 */
_fer_inline fer_gng_edge_t *ferGNGEdgeFromNet(fer_net_edge_t *e);

/**
 * Cast GNG node to Net node.
 */
_fer_inline fer_net_node_t *ferGNGNodeToNet(fer_gng_node_t *n);

/**
 * Cast GNG edge to Net edge.
 */
_fer_inline fer_net_edge_t *ferGNGEdgeToNet(fer_gng_edge_t *e);



/**** INLINES ****/
_fer_inline fer_net_t *ferGNGNet(fer_gng_t *gng)
{
    return gng->net;
}

_fer_inline fer_list_t *ferGNGNodes(fer_gng_t *gng)
{
    return ferNetNodes(gng->net);
}

_fer_inline size_t ferGNGNodesLen(const fer_gng_t *gng)
{
    return ferNetNodesLen(gng->net);
}

_fer_inline fer_list_t *ferGNGEdges(fer_gng_t *gng)
{
    return ferNetEdges(gng->net);
}

_fer_inline size_t ferGNGEdgesLen(const fer_gng_t *gng)
{
    return ferNetEdgesLen(gng->net);
}

_fer_inline fer_gng_node_t *ferGNGNodeFromList(fer_list_t *item)
{
    fer_net_node_t *nn;
    fer_gng_node_t *n;

    nn = ferListEntry(item, fer_net_node_t, list);
    n  = fer_container_of(nn, fer_gng_node_t, node);
    return n;
}

_fer_inline fer_gng_edge_t *ferGNGEdgeFromList(fer_list_t *item)
{
    fer_net_edge_t *nn;
    fer_gng_edge_t *n;

    nn = ferListEntry(item, fer_net_edge_t, list);
    n  = fer_container_of(nn, fer_gng_edge_t, edge);
    return n;
}

_fer_inline fer_gng_node_t *ferGNGNodeFromNet(fer_net_node_t *n)
{
    return fer_container_of(n, fer_gng_node_t, node);
}

_fer_inline fer_gng_edge_t *ferGNGEdgeFromNet(fer_net_edge_t *e)
{
    return fer_container_of(e, fer_gng_edge_t, edge);
}

_fer_inline fer_net_node_t *ferGNGNodeToNet(fer_gng_node_t *n)
{
    return &n->node;
}

_fer_inline fer_net_edge_t *ferGNGEdgeToNet(fer_gng_edge_t *e)
{
    return &e->edge;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GNG_H__ */

