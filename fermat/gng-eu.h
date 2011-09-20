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

#ifndef __FER_GNG_EU_H__
#define __FER_GNG_EU_H__

#include <fermat/net.h>
#include <fermat/pairheap.h>
#include <fermat/vec.h>
#include <fermat/vec2.h>
#include <fermat/vec3.h>
#include <fermat/pc.h>
#include <fermat/nn.h>
#include <fermat/alloc.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * Growing Neural Gas In Euclidean Space
 * ======================================
 */


struct _fer_gng_eu_node_t {
    fer_net_node_t node;

    fer_real_t err;               /*!< Overall error */
    unsigned long err_cycle;      /*!< Last cycle in which were .err changed */
    fer_pairheap_node_t err_heap; /*!< Connection to error heap */

    fer_vec_t *w;   /*!< Weight vector */
    fer_nn_el_t nn; /*!< Struct for NN search */

    int _id; /*!< Currently useful only for ferGNGEuDumpSVT(). */
};
typedef struct _fer_gng_eu_node_t fer_gng_eu_node_t;


struct _fer_gng_eu_edge_t {
    fer_net_edge_t edge;

    int age;
};
typedef struct _fer_gng_eu_edge_t fer_gng_eu_edge_t;



/**
 * GNGEu Operations
 * -----------------
 *
 * See fer_gng_eu_ops_t.
 */

/** vvvv */

/**
 * Create new node initialized weight vector to input_signal.
 */
typedef fer_gng_eu_node_t *(*fer_gng_eu_new_node)(const fer_vec_t *input_signal, void *);

/**
 * Deletes given node.
 */
typedef void (*fer_gng_eu_del_node)(fer_gng_eu_node_t *n, void *);

/**
 * Returns random input signal.
 */
typedef const fer_vec_t *(*fer_gng_eu_input_signal)(void *);

/**
 * Returns true if algorithm should terminate.
 */
typedef int (*fer_gng_eu_terminate)(void *);

/**
 * Callback that is peridically called from GNGEu.
 *
 * It is called every .callback_period'th added node.
 */
typedef void (*fer_gng_eu_callback)(void *);

/** ^^^^ */

struct _fer_gng_eu_ops_t {
    fer_gng_eu_new_node     new_node;
    fer_gng_eu_del_node     del_node;
    fer_gng_eu_input_signal input_signal;
    fer_gng_eu_terminate    terminate;

    fer_gng_eu_callback callback;
    unsigned long callback_period;

    void *data; /*!< Data pointer that will be provided to all callbacks if
                     not specified otherwise. */
    void *new_node_data;
    void *del_node_data;
    void *input_signal_data;
    void *terminate_data;
    void *callback_data;
};
typedef struct _fer_gng_eu_ops_t fer_gng_eu_ops_t;


/**
 * Initializes ops struct to NULL values.
 */
void ferGNGEuOpsInit(fer_gng_eu_ops_t *ops);



/**
 * GNGEu Parameters
 * -----------------
 */
struct _fer_gng_eu_params_t {
    int dim;          /*!< Dimension */

    size_t lambda;    /*!< Number of steps between adding nodes */
    fer_real_t eb;    /*!< Winner node learning rate */
    fer_real_t en;    /*!< Winners' neighbors learning rate */
    fer_real_t alpha; /*!< Decrease error counter rate */
    fer_real_t beta;  /*!< Decrease error counter rate for all nodes */
    int age_max;      /*!< Maximal age of edge */

    int use_nn; /*!< Set to one of types defined in fermat/nn.h (FER_NN_*)
                     to use non-linear algorithm for NN search. You also
                     must set {.nn} params appropriately.
                     Default: FER_NN_NNCELLS */
    fer_nn_params_t nn;
};
typedef struct _fer_gng_eu_params_t fer_gng_eu_params_t;

/**
 * Initializes params struct to default values.
 */
void ferGNGEuParamsInit(fer_gng_eu_params_t *params);



/**
 * GNGEu Algorithm
 * ----------------
 *
 * See fer_gng_eu_t.
 */

struct _fer_gng_eu_t {
    fer_net_t *net;
    fer_pairheap_t *err_heap;

    fer_gng_eu_ops_t ops;
    fer_gng_eu_params_t params;

    fer_real_t *beta_n; /*!< Precomputed beta^n for n = 1, ..., lambda */
    fer_real_t *beta_lambda_n; /*!< Precomputed beta^(n*lambda) */
    size_t beta_lambda_n_len;

    size_t step;
    unsigned long cycle;

    fer_nn_t *nn;

    fer_vec_t *tmpv;
};
typedef struct _fer_gng_eu_t fer_gng_eu_t;


/**
 * Creates new instance of GNGEu algorithm.
 */
fer_gng_eu_t *ferGNGEuNew(const fer_gng_eu_ops_t *ops,
                     const fer_gng_eu_params_t *params);

/**
 * Deletes GNGEu.
 */
void ferGNGEuDel(fer_gng_eu_t *gng_eu);

/**
 * Runs GNGEu algorithm.
 *
 * This runs whole algorithm in loop until operation terminate() returns
 * true:
 * ~~~~~~
 * ferGNGEuinit()
 * do:
 *     for (step = 1 .. params.lambda):
 *         ferGNGEuLearn()
 *     ferGNGEuNewNode()
 * while not ops.terminate()
 */
void ferGNGEuRun(fer_gng_eu_t *gng_eu);


/**
 * Initialize gng_eu net.
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
void ferGNGEuInit(fer_gng_eu_t *gng_eu);

/**
 * One competitive hebbian learning step.
 */
void ferGNGEuLearn(fer_gng_eu_t *gng_eu);

/**
 * Creates new node in place with highest error counter.
 */
void ferGNGEuNewNode(fer_gng_eu_t *gng_eu);

/**
 * Performs hebbian learning between two given nodes - connection
 * between these two nodes is strengthened, i.e., edge is eigher created or
 * age of the existing edge is set to zero.
 */
void ferGNGEuHebbianLearning(fer_gng_eu_t *gng_eu,
                           fer_gng_eu_node_t *n1, fer_gng_eu_node_t *n2);

/**
 * Returns node with highest error counter.
 */
fer_gng_eu_node_t *ferGNGEuNodeWithHighestError(fer_gng_eu_t *gng_eu);

/**
 * Finds out node with highest error counter ({n1}) and its neighbor with
 * highest error counter ({n2}). Into {edge} is stored edge connecting
 * those nodes. {n1}, {n2} and {edge} are ignored if NULL is passed.
 */
void ferGNGEuNodeWithHighestError2(fer_gng_eu_t *gng_eu,
                                 fer_gng_eu_node_t **n1, fer_gng_eu_node_t **n2,
                                 fer_gng_eu_edge_t **edge);

void ferGNGEuDumpSVT(fer_gng_eu_t *gng_eu, FILE *out, const char *name);


/**
 * Net Related API
 * ----------------
 *
 * See fer_gng_eu_node_t.
 * See fer_gng_eu_edge_t.
 */

/**
 * Returns net of nodes.
 */
_fer_inline fer_net_t *ferGNGEuNet(fer_gng_eu_t *gng_eu);

/**
 * Returns list of nodes.
 */
_fer_inline fer_list_t *ferGNGEuNodes(fer_gng_eu_t *gng_eu);

/**
 * Returns number of nodes in net.
 */
_fer_inline size_t ferGNGEuNodesLen(const fer_gng_eu_t *gng_eu);

/**
 * Returns list of edges.
 */
_fer_inline fer_list_t *ferGNGEuNodes(fer_gng_eu_t *gng_eu);

/**
 * Returns number of edges in net.
 */
_fer_inline size_t ferGNGEuEdgesLen(const fer_gng_eu_t *gng_eu);

/**
 * Returns list of nodes
 */
_fer_inline fer_list_t *ferGNGEuEdges(fer_gng_eu_t *gng_eu);

/**
 * Returns GNGEu node from list pointer.
 *
 * Usage:
 * ~~~~~
 * fer_list_t *list, *item;
 * fer_gng_eu_node_t *n;
 *
 * list = ferGNGEuNodes(gng_eu);
 * FER_LIST_FOR_EACH(list, item){
 *     n = ferGNGEuNodeFromList(item);
 *     ....
 * }
 */
_fer_inline fer_gng_eu_node_t *ferGNGEuNodeFromList(fer_list_t *item);

/**
 * Similar to *ferGNGEuNodeFromList()* but works with nodes.
 */
_fer_inline fer_gng_eu_edge_t *ferGNGEuEdgeFromList(fer_list_t *item);

/**
 * Cast Net node to GNGEu node.
 */
_fer_inline fer_gng_eu_node_t *ferGNGEuNodeFromNet(fer_net_node_t *n);

/**
 * Cast Net edge to GNGEu edge.
 */
_fer_inline fer_gng_eu_edge_t *ferGNGEuEdgeFromNet(fer_net_edge_t *e);

/**
 * Cast GNGEu node to Net node.
 */
_fer_inline fer_net_node_t *ferGNGEuNodeToNet(fer_gng_eu_node_t *n);

/**
 * Cast GNGEu edge to Net edge.
 */
_fer_inline fer_net_edge_t *ferGNGEuEdgeToNet(fer_gng_eu_edge_t *e);



/**
 * Node API
 * ^^^^^^^^^
 *
 * See fer_gng_eu_node_t.
 */

/**
 * Adds node into network
 */
_fer_inline void ferGNGEuNodeAdd(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n,
                                 const fer_vec_t *w);

/**
 * Removes node from network
 */
_fer_inline void ferGNGEuNodeRemove(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n);

/**
 * Removes node from network and deletes it (ops.del_node is used).
 */
_fer_inline void ferGNGEuNodeDel(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n);

/**
 * Fixes node's error counter, i.e. applies correct beta^(n * lambda)
 */
_fer_inline void ferGNGEuNodeFixError(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n);

/**
 * Increment error counter
 */
_fer_inline void ferGNGEuNodeIncError(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n,
                                    fer_real_t inc);
/**
 * Scales error counter
 */
_fer_inline void ferGNGEuNodeScaleError(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n,
                                      fer_real_t scale);

/**
 * Disconnects node from net, i.e., deletes all incidenting edges.
 */
void ferGNGEuNodeDisconnect(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n);

/**
 * Connects new node at given position (is) and connects it with two
 * nearest nodes [ops.new_node(), ops.nearest()].
 */
fer_gng_eu_node_t *ferGNGEuNodeNewAtPos(fer_gng_eu_t *gng_eu, const void *is);




/**
 * Edge API
 * ^^^^^^^^^
 *
 * See fer_gng_eu_edge_t.
 */

/**
 * Creates and initializes new edge between {n1} and {n2}.
 */
fer_gng_eu_edge_t *ferGNGEuEdgeNew(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n1,
                                              fer_gng_eu_node_t *n2);

/**
 * Deletes edge
 */
void ferGNGEuEdgeDel(fer_gng_eu_t *gng_eu, fer_gng_eu_edge_t *edge);

/**
 * Returns age of edge.
 *
 * Always use this function instead of direct access to struct!
 */
_fer_inline int ferGNGEuEdgeAge(const fer_gng_eu_t *gng_eu, const fer_gng_eu_edge_t *edge);


/**
 * Returns edge connecting {n1} and {n2}.
 */
_fer_inline fer_gng_eu_edge_t *ferGNGEuEdgeBetween(fer_gng_eu_t *gng_eu,
                                              fer_gng_eu_node_t *n1,
                                              fer_gng_eu_node_t *n2);

/**
 * Deletes edge between {n1} and {n2}.
 */
void ferGNGEuEdgeBetweenDel(fer_gng_eu_t *gng_eu,
                          fer_gng_eu_node_t *n1, fer_gng_eu_node_t *n2);

/**
 * Returns (via {n1} and {n2}) incidenting nodes of edge
 */
_fer_inline void ferGNGEuEdgeNodes(fer_gng_eu_edge_t *e,
                                 fer_gng_eu_node_t **n1, fer_gng_eu_node_t **n2);





/**** INLINES ****/
_fer_inline fer_net_t *ferGNGEuNet(fer_gng_eu_t *gng_eu)
{
    return gng_eu->net;
}

_fer_inline fer_list_t *ferGNGEuNodes(fer_gng_eu_t *gng_eu)
{
    return ferNetNodes(gng_eu->net);
}

_fer_inline size_t ferGNGEuNodesLen(const fer_gng_eu_t *gng_eu)
{
    return ferNetNodesLen(gng_eu->net);
}

_fer_inline fer_list_t *ferGNGEuEdges(fer_gng_eu_t *gng_eu)
{
    return ferNetEdges(gng_eu->net);
}

_fer_inline size_t ferGNGEuEdgesLen(const fer_gng_eu_t *gng_eu)
{
    return ferNetEdgesLen(gng_eu->net);
}

_fer_inline fer_gng_eu_node_t *ferGNGEuNodeFromList(fer_list_t *item)
{
    fer_net_node_t *nn;
    fer_gng_eu_node_t *n;

    nn = FER_LIST_ENTRY(item, fer_net_node_t, list);
    n  = fer_container_of(nn, fer_gng_eu_node_t, node);
    return n;
}

_fer_inline fer_gng_eu_edge_t *ferGNGEuEdgeFromList(fer_list_t *item)
{
    fer_net_edge_t *nn;
    fer_gng_eu_edge_t *n;

    nn = FER_LIST_ENTRY(item, fer_net_edge_t, list);
    n  = fer_container_of(nn, fer_gng_eu_edge_t, edge);
    return n;
}

_fer_inline fer_gng_eu_node_t *ferGNGEuNodeFromNet(fer_net_node_t *n)
{
    return fer_container_of(n, fer_gng_eu_node_t, node);
}

_fer_inline fer_gng_eu_edge_t *ferGNGEuEdgeFromNet(fer_net_edge_t *e)
{
    return fer_container_of(e, fer_gng_eu_edge_t, edge);
}

_fer_inline fer_net_node_t *ferGNGEuNodeToNet(fer_gng_eu_node_t *n)
{
    return &n->node;
}

_fer_inline fer_net_edge_t *ferGNGEuEdgeToNet(fer_gng_eu_edge_t *e)
{
    return &e->edge;
}



_fer_inline void ferGNGEuNodeAdd(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n,
                                 const fer_vec_t *w)
{
    n->err       = FER_ZERO;
    n->err_cycle = gng_eu->cycle;
    ferPairHeapAdd(gng_eu->err_heap, &n->err_heap);

    ferNetAddNode(gng_eu->net, &n->node);

    if (gng_eu->params.dim == 2){
        n->w = (fer_vec_t *)ferVec2Clone((const fer_vec2_t *)w);
    }else if (gng_eu->params.dim == 3){
        n->w = (fer_vec_t *)ferVec3Clone((const fer_vec3_t *)w);
    }else{
        n->w = ferVecClone(gng_eu->params.dim, (const fer_vec_t *)w);
    }

    if (gng_eu->nn){
        ferNNElInit(gng_eu->nn, &n->nn, n->w);
        ferNNAdd(gng_eu->nn, &n->nn);
    }
}

_fer_inline void ferGNGEuNodeRemove(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n)
{
    ferPairHeapRemove(gng_eu->err_heap, &n->err_heap);

    if (ferNetNodeEdgesLen(&n->node) != 0)
        ferGNGEuNodeDisconnect(gng_eu, n);
    ferNetRemoveNode(gng_eu->net, &n->node);

    if (gng_eu->nn){
        ferNNRemove(gng_eu->nn, &n->nn);
    }

    ferVecDel(n->w);
}

_fer_inline void ferGNGEuNodeDel(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n)
{
    ferGNGEuNodeRemove(gng_eu, n);
    if (gng_eu->ops.del_node){
        gng_eu->ops.del_node(n, gng_eu->ops.del_node_data);
    }else{
        FER_FREE(n);
    }
}

_fer_inline void ferGNGEuNodeFixError(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n)
{
    unsigned long diff;

    diff = gng_eu->cycle - n->err_cycle;
    if (diff > 0 && diff <= gng_eu->beta_lambda_n_len){
        n->err *= gng_eu->beta_lambda_n[diff - 1];
    }else if (diff > 0){
        n->err *= gng_eu->beta_lambda_n[gng_eu->beta_lambda_n_len - 1];

        diff = diff - gng_eu->beta_lambda_n_len;
        n->err *= pow(gng_eu->beta_n[gng_eu->beta_lambda_n_len - 1], diff);
    }
    n->err_cycle = gng_eu->cycle;
}

_fer_inline void ferGNGEuNodeIncError(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n,
                                    fer_real_t inc)
{
    ferGNGEuNodeFixError(gng_eu, n);
    n->err += inc;
    ferPairHeapUpdate(gng_eu->err_heap, &n->err_heap);
}

_fer_inline void ferGNGEuNodeScaleError(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n,
                                      fer_real_t scale)
{
    ferGNGEuNodeFixError(gng_eu, n);
    n->err *= scale;
    ferPairHeapUpdate(gng_eu->err_heap, &n->err_heap);
}



_fer_inline int ferGNGEuEdgeAge(const fer_gng_eu_t *gng_eu, const fer_gng_eu_edge_t *edge)
{
    return edge->age;
}

_fer_inline fer_gng_eu_edge_t *ferGNGEuEdgeBetween(fer_gng_eu_t *gng_eu,
                                              fer_gng_eu_node_t *n1,
                                              fer_gng_eu_node_t *n2)
{
    fer_net_edge_t *ne;
    fer_gng_eu_edge_t *e = NULL;

    ne = ferNetNodeCommonEdge(&n1->node, &n2->node);
    if (ne)
        e  = fer_container_of(ne, fer_gng_eu_edge_t, edge);
    return e;
}

_fer_inline void ferGNGEuEdgeNodes(fer_gng_eu_edge_t *e,
                                 fer_gng_eu_node_t **n1, fer_gng_eu_node_t **n2)
{
    fer_net_node_t *n;

    n   = ferNetEdgeNode(&e->edge, 0);
    *n1 = fer_container_of(n, fer_gng_eu_node_t, node);

    n   = ferNetEdgeNode(&e->edge, 1);
    *n2 = fer_container_of(n, fer_gng_eu_node_t, node);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GNG_EU_H__ */


