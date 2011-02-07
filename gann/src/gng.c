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

#include <gann/gng.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Node functions */
/** Initialize node */
_fer_inline void nodeInit(gann_gng_t *gng, gann_gng_node_t *n);
/** Adds node into network */
_fer_inline void nodeAdd(gann_gng_t *gng, gann_gng_node_t *n);
/** Set error counter of node */
_fer_inline void nodeSetErrCounter(gann_gng_t *gng, gann_gng_node_t *n,
                                   fer_real_t err);
/** Increases error counter of node */
_fer_inline void nodeIncErrCounter(gann_gng_t *gng, gann_gng_node_t *n,
                                   fer_real_t inc);
/** Scale error counter of node */
_fer_inline void nodeScaleErrCounter(gann_gng_t *gng, gann_gng_node_t *n,
                                    fer_real_t s);
/** Final destructor for nodes */
static void nodeFinalDel(gann_net_node_t *node, void *data);

/** Edge functions */
/** Creates and initializes new edge between n1 and n2 */
_fer_inline gann_gng_edge_t *edgeNew(gann_gng_t *gng, gann_gng_node_t *n1,
                                                      gann_gng_node_t *n2);
/** Deletes edge */
_fer_inline void edgeDel(gann_gng_t *gng, gann_gng_edge_t *edge);
/** Delete callback */
typedef void (*delnode_t)(gann_net_node_t *, void *);
static void delEdge(gann_net_edge_t *edge, void *data);

/** Returns node with highest error counter */
static gann_gng_node_t *nodeWithHighestErr(gann_gng_t *gng);
/** Returns q's neighbor with highest error counter and edge that connects
 *  that node with q. */
static gann_gng_node_t *nodeWithHighestErr2(gann_gng_t *gng, gann_gng_node_t *q,
                                            gann_gng_edge_t **edge);



void gannGNGOpsInit(gann_gng_ops_t *ops)
{
    bzero(ops, sizeof(gann_gng_ops_t));
}

void gannGNGParamsInit(gann_gng_params_t *params)
{
    params->lambda  = 200;
    params->eb      = 0.05;
    params->en      = 0.0006;
    params->alpha   = 0.95;
    params->beta    = 0.9995;
    params->age_max = 200;
}


gann_gng_t *gannGNGNew(const gann_gng_ops_t *ops,
                       const gann_gng_params_t *params,
                       void *data)
{
    gann_gng_t *gng;

    gng = FER_ALLOC(gann_gng_t);

    gng->net = gannNetNew();

    gng->ops    = *ops;
    gng->params = *params;
    gng->data   = data;

    return gng;
}

void gannGNGDel(gann_gng_t *gng)
{
    if (gng->net){
        gannNetDel2(gng->net, nodeFinalDel, gng,
                              delEdge, gng);
    }

    free(gng);
}

void gannGNGRun(gann_gng_t *gng)
{
    size_t step;
    gannGNGInit(gng);

    step = 1;
    while (!gng->ops.terminate(gng->data)){
        gannGNGLearn(gng);

        if (step % gng->params.lambda == 0){
            gannGNGNewNode(gng);
        }
        gannGNGDecreaseErrCounters(gng);

        step++;
    }
}

void gannGNGInit(gann_gng_t *gng)
{
    const void *is;
    gann_gng_node_t *n;

    is = gng->ops.input_signal(gng->data);
    n  = gng->ops.new_node(is, gng->data);
    nodeInit(gng, n);
    nodeAdd(gng, n);

    is = gng->ops.input_signal(gng->data);
    n  = gng->ops.new_node(is, gng->data);
    nodeInit(gng, n);
    nodeAdd(gng, n);
}

void gannGNGLearn(gann_gng_t *gng)
{
    const void *input_signal;
    gann_net_node_t *nn;
    gann_gng_node_t *n1, *n2, *n;
    gann_net_edge_t *nedge;
    gann_gng_edge_t *edge;
    fer_real_t dist2;
    fer_list_t *list, *item, *item_tmp;

    // 1. Get input signal
    input_signal = gng->ops.input_signal(gng->data);

    // 2. Find two nearest node to input signal
    gng->ops.nearest(input_signal, &n1, &n2, gng->data);

    // 3. Create connection between n1 and n2 if doesn't exist and set age
    //    to zero
    nedge = gannNetNodeCommonEdge(&n1->node, &n2->node);
    if (!nedge){
        edge = edgeNew(gng, n1, n2);
    }else{
        edge = fer_container_of(nedge, gann_gng_edge_t, edge);
    }
    edge->age = 0;

    // 4. Increase error counter of winner node
    dist2 = gng->ops.dist2(input_signal, n1, gng->data);
    nodeIncErrCounter(gng, n1, dist2);

    // 5. Adapt nodes to input signal using fractions eb and en
    // + 6. Increment age of all edges by one
    // + 7. Remove edges with age higher than age_max
    gng->ops.move_towards(n1, input_signal, gng->params.eb, gng->data);
    // adapt also direct topological neighbors of winner node
    list = gannNetNodeEdges(&n1->node);
    ferListForEachSafe(list, item, item_tmp){
        nedge = gannNetEdgeFromNodeList(item);
        edge  = fer_container_of(nedge, gann_gng_edge_t, edge);
        nn   = gannNetEdgeOtherNode(&edge->edge, &n1->node);
        n    = fer_container_of(nn, gann_gng_node_t, node);

        // move node (5.)
        gng->ops.move_towards(n, input_signal, gng->params.en, gng->data);

        // increase age (6.)
        edge->age += 1;

        // remove edge if it has age higher than age_max (7.)
        if (edge->age > gng->params.age_max){
            edgeDel(gng, edge);
        }
    }
}

void gannGNGNewNode(gann_gng_t *gng)
{
    gann_gng_node_t *q, *f, *r;
    gann_gng_edge_t *eqf;
    fer_real_t err;

    // 1. Get node with highest error counter
    q = nodeWithHighestErr(gng);

    // 2. Get q's neighbor with highest error counter
    f = nodeWithHighestErr2(gng, q, &eqf);
    if (!f){
        return;
    }

    // 3. Create new node between q and f
    r = gng->ops.new_node_between(q, f, gng->data);
    nodeAdd(gng, r);

    // 4. Create q-r and f-r edges and remove q-f edge (which is eqf)
    edgeDel(gng, eqf);
    edgeNew(gng, q, r);
    edgeNew(gng, f, r);

    // 5. Decrease error counters of q and f
    nodeScaleErrCounter(gng, q, gng->params.alpha);
    nodeScaleErrCounter(gng, f, gng->params.alpha);

    // 6. Set error counter of new node (r)
    err  = gannGNGNodeErrCounter(gng, q);
    err += gannGNGNodeErrCounter(gng, q);
    err /= FER_REAL(2.);
    nodeSetErrCounter(gng, r, err);
}

void gannGNGDecreaseErrCounters(gann_gng_t *gng)
{
    fer_list_t *list, *item;
    gann_net_node_t *nn;
    gann_gng_node_t *n;

    list = gannNetNodes(gng->net);
    ferListForEach(list, item){
        nn = ferListEntry(item, gann_net_node_t, list);
        n  = fer_container_of(nn, gann_gng_node_t, node);
        nodeScaleErrCounter(gng, n, gng->params.beta);
    }
}


/*** Node functions ***/
fer_real_t gannGNGNodeErrCounter(const gann_gng_t *gng, const gann_gng_node_t *n)
{
    return n->err_counter;
}

_fer_inline void nodeInit(gann_gng_t *gng, gann_gng_node_t *n)
{
    n->err_counter = FER_ZERO;
}

_fer_inline void nodeAdd(gann_gng_t *gng, gann_gng_node_t *n)
{
    gannNetAddNode(gng->net, &n->node);
}

_fer_inline void nodeSetErrCounter(gann_gng_t *gng, gann_gng_node_t *n,
                                   fer_real_t err)
{
    n->err_counter = err;
}

_fer_inline void nodeIncErrCounter(gann_gng_t *gng, gann_gng_node_t *n,
                                   fer_real_t inc)
{
    n->err_counter += inc;
}

_fer_inline void nodeScaleErrCounter(gann_gng_t *gng, gann_gng_node_t *n,
                                     fer_real_t s)
{
    n->err_counter *= s;
}

static void nodeFinalDel(gann_net_node_t *node, void *data)
{
    gann_gng_t *gng = (gann_gng_t *)data;
    gann_gng_node_t *n;

    n = fer_container_of(node, gann_gng_node_t, node);
    gng->ops.del_node(n, gng->data);
}



/*** Edge functions ***/
_fer_inline gann_gng_edge_t *edgeNew(gann_gng_t *gng, gann_gng_node_t *n1,
                                                      gann_gng_node_t *n2)
{
    gann_gng_edge_t *e;

    e = FER_ALLOC(gann_gng_edge_t);
    e->age = 0;

    gannNetAddEdge(gng->net, &e->edge, &n1->node, &n2->node);

    return e;
}

_fer_inline void edgeDel(gann_gng_t *gng, gann_gng_edge_t *e)
{
    gannNetRemoveEdge(gng->net, &e->edge);
    free(e);
}

static void delEdge(gann_net_edge_t *edge, void *data)
{
    free(edge);
}



static gann_gng_node_t *nodeWithHighestErr(gann_gng_t *gng)
{
    fer_list_t *list, *item;
    gann_net_node_t *nn;
    gann_gng_node_t *n, *n_highest;
    fer_real_t err, err_highest;

    err_highest = FER_REAL_MIN;
    n_highest   = NULL;

    list = gannNetNodes(gng->net);
    ferListForEach(list, item){
        nn = ferListEntry(item, gann_net_node_t, list);
        n  = fer_container_of(nn, gann_gng_node_t, node);
        err = gannGNGNodeErrCounter(gng, n);

        if (err > err_highest){
            err_highest = err;
            n_highest   = n;
        }
    }

    return n_highest;
}

static gann_gng_node_t *nodeWithHighestErr2(gann_gng_t *gng, gann_gng_node_t *q,
                                            gann_gng_edge_t **edge)
{
    fer_list_t *list, *item;
    gann_net_edge_t *ne;
    gann_gng_edge_t *e_highest;
    gann_net_node_t *nn;
    gann_gng_node_t *n, *n_highest;
    fer_real_t err, err_highest;

    err_highest = FER_REAL_MIN;
    n_highest = NULL;
    e_highest = NULL;

    list = gannNetNodeEdges(&q->node);
    ferListForEach(list, item){
        ne = gannNetEdgeFromNodeList(item);
        nn = gannNetEdgeOtherNode(ne, &q->node);
        n  = fer_container_of(nn, gann_gng_node_t, node);
        err = gannGNGNodeErrCounter(gng, n);

        if (err > err_highest){
            err_highest = err;
            n_highest   = n;
            e_highest   = fer_container_of(ne, gann_gng_edge_t, edge);
        }
    }

    *edge = e_highest;
    return n_highest;
}
