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

#include <fermat/gng.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>


static void ferGNGInit(fer_gng_t *gng);
static void ferGNGLearn(fer_gng_t *gng, size_t step);
static void ferGNGNewNode(fer_gng_t *gng);

/** Node functions */
/** Adds node into network */
_fer_inline void nodeAdd(fer_gng_t *gng, fer_gng_node_t *n);
/** Removes node from network */
_fer_inline void nodeRemove(fer_gng_t *gng, fer_gng_node_t *n);
/** Removes node from network and deletes it */
_fer_inline void nodeDel(fer_gng_t *gng, fer_gng_node_t *n);
/** Fixes node's error counter, i.e. applies correct beta^(n * lambda) */
_fer_inline void nodeFixError(fer_gng_t *gng, fer_gng_node_t *n);
/** Increment error counter */
_fer_inline void nodeIncError(fer_gng_t *gng, fer_gng_node_t *n,
                              fer_real_t inc);
/** Scales error counter */
_fer_inline void nodeScaleError(fer_gng_t *gng, fer_gng_node_t *n,
                                fer_real_t scale);


/** Edge functions */
/** Creates and initializes new edge between n1 and n2 */
_fer_inline fer_gng_edge_t *edgeNew(fer_gng_t *gng, fer_gng_node_t *n1,
                                                    fer_gng_node_t *n2);
/** Deletes edge */
_fer_inline void edgeDel(fer_gng_t *gng, fer_gng_edge_t *edge);

/** Returns node with highest error counter */
static fer_gng_node_t *nodeWithHighestErr(fer_gng_t *gng);
/** Returns q's neighbor with highest error counter and edge that connects
 *  that node with q. */
static fer_gng_node_t *nodeWithHighestErr2(fer_gng_t *gng, fer_gng_node_t *q,
                                           fer_gng_edge_t **edge);


/** Should return true if n1 > n2 - this is used for err-heap */
static int errHeapLT(const fer_pairheap_node_t *n1,
                     const fer_pairheap_node_t *n2, void *);


/** Delete callbacks */
static void nodeFinalDel(fer_net_node_t *node, void *data);
static void delEdge(fer_net_edge_t *edge, void *data);

void ferGNGOpsInit(fer_gng_ops_t *ops)
{
    bzero(ops, sizeof(fer_gng_ops_t));
}

void ferGNGParamsInit(fer_gng_params_t *params)
{
    params->lambda  = 200;
    params->eb      = 0.05;
    params->en      = 0.0006;
    params->alpha   = 0.95;
    params->beta    = 0.9995;
    params->age_max = 200;
}


fer_gng_t *ferGNGNew(const fer_gng_ops_t *ops,
                     const fer_gng_params_t *params)
{
    fer_gng_t *gng;

    gng = FER_ALLOC(fer_gng_t);

    gng->net = ferNetNew();
    gng->err_heap = NULL;

    gng->ops    = *ops;
    gng->params = *params;

    // set up ops data pointers
    if (!gng->ops.init_data)
        gng->ops.init_data = gng->ops.data;
    if (!gng->ops.new_node_data)
        gng->ops.new_node_data = gng->ops.data;
    if (!gng->ops.new_node_between_data)
        gng->ops.new_node_between_data = gng->ops.data;
    if (!gng->ops.del_node_data)
        gng->ops.del_node_data = gng->ops.data;
    if (!gng->ops.input_signal_data)
        gng->ops.input_signal_data = gng->ops.data;
    if (!gng->ops.nearest_data)
        gng->ops.nearest_data = gng->ops.data;
    if (!gng->ops.dist2_data)
        gng->ops.dist2_data = gng->ops.data;
    if (!gng->ops.move_towards_data)
        gng->ops.move_towards_data = gng->ops.data;
    if (!gng->ops.terminate_data)
        gng->ops.terminate_data = gng->ops.data;
    if (!gng->ops.callback_data)
        gng->ops.callback_data = gng->ops.data;

    gng->beta_n = NULL;
    gng->beta_lambda_n = NULL;
    gng->beta_lambda_n_len = 0;

    return gng;
}

void ferGNGDel(fer_gng_t *gng)
{
    if (gng->beta_n)
        free(gng->beta_n);
    if (gng->beta_lambda_n)
        free(gng->beta_lambda_n);

    if (gng->net){
        ferNetDel2(gng->net, nodeFinalDel, gng,
                              delEdge, gng);
    }

    if (gng->err_heap)
        ferPairHeapDel(gng->err_heap);

    free(gng);
}


void ferGNGRun(fer_gng_t *gng)
{
    unsigned long cycle;

    cycle = 0;
    ferGNGInit(gng);

    do {
        for (gng->step = 1; gng->step <= gng->params.lambda; gng->step++){
            ferGNGLearn(gng, gng->step);
        }
        ferGNGNewNode(gng);

        cycle++;
        if (gng->ops.callback && gng->ops.callback_period == cycle){
            gng->ops.callback(gng->ops.callback_data);
            cycle = 0L;
        }

        gng->cycle++;
    } while (!gng->ops.terminate(gng->ops.terminate_data));
}

fer_gng_node_t *ferGNGConnectNewNode(fer_gng_t *gng, const void *is)
{
    fer_gng_node_t *r, *n1, *n2;
    fer_gng_edge_t *edge;

    gng->ops.nearest(is, &n1, &n2, gng->ops.nearest_data);

    r = gng->ops.new_node(is, gng->ops.new_node_data);
    nodeAdd(gng, r);

    edge = edgeNew(gng, r, n1);
    edge->age = 0;
    edge = edgeNew(gng, r, n2);
    edge->age = 0;

    return r;
}

void ferGNGRemoveNode(fer_gng_t *gng, fer_gng_node_t *node)
{
    fer_list_t *edges, *item, *itemtmp;
    fer_net_edge_t *ne;
    fer_gng_edge_t *edge;

    // remove incidenting edges
    edges = ferNetNodeEdges(&node->node);
    FER_LIST_FOR_EACH_SAFE(edges, item, itemtmp){
        ne = ferNetEdgeFromNodeList(item);
        edge = ferGNGEdgeFromNet(ne);
        edgeDel(gng, edge);
    }

    // remove node from net but don't delete it
    nodeRemove(gng, node);
}

void ferGNGDelEdgeBetween(fer_gng_t *gng,
                          fer_gng_node_t *n1, fer_gng_node_t *n2)
{
    fer_net_edge_t *ne;
    fer_gng_edge_t *e;

    ne = ferNetNodeCommonEdge(&n1->node, &n2->node);
    e  = fer_container_of(ne, fer_gng_edge_t, edge);

    edgeDel(gng, e);
}

void ferGNGEdgeNodes(fer_gng_edge_t *e,
                     fer_gng_node_t **n1, fer_gng_node_t **n2)
{
    fer_net_node_t *n;

    n   = ferNetEdgeNode(&e->edge, 0);
    *n1 = fer_container_of(n, fer_gng_node_t, node);

    n   = ferNetEdgeNode(&e->edge, 1);
    *n2 = fer_container_of(n, fer_gng_node_t, node);
}

static void ferGNGInit(fer_gng_t *gng)
{
    const void *is;
    fer_gng_node_t *n1 = NULL, *n2 = NULL;
    size_t i;
    fer_real_t maxbeta;

    gng->cycle = 1L;

    // initialize error heap
    if (gng->err_heap)
        ferPairHeapDel(gng->err_heap);
    gng->err_heap = ferPairHeapNew(errHeapLT, (void *)gng);

    // precompute beta^n
    if (gng->beta_n)
        free(gng->beta_n);
    gng->beta_n = FER_ALLOC_ARR(fer_real_t, gng->params.lambda);
    gng->beta_n[0] = gng->params.beta;
    for (i = 1; i < gng->params.lambda; i++){
        gng->beta_n[i] = gng->beta_n[i - 1] * gng->params.beta;
    }

    // precompute beta^(n * lambda)
    if (gng->beta_lambda_n)
        free(gng->beta_lambda_n);

    maxbeta = gng->beta_n[gng->params.lambda - 1];

    gng->beta_lambda_n_len = 1000;
    gng->beta_lambda_n = FER_ALLOC_ARR(fer_real_t, gng->beta_lambda_n_len);
    gng->beta_lambda_n[0] = maxbeta;
    for (i = 1; i < gng->beta_lambda_n_len; i++){
        gng->beta_lambda_n[i] = gng->beta_lambda_n[i - 1] * maxbeta;
    }


    if (gng->ops.init){
        gng->ops.init(&n1, &n2, gng->ops.init_data);
    }else{
        is = gng->ops.input_signal(gng->ops.input_signal_data);
        n1 = gng->ops.new_node(is, gng->ops.new_node_data);

        is = gng->ops.input_signal(gng->ops.input_signal_data);
        n2 = gng->ops.new_node(is, gng->ops.new_node_data);
    }

    nodeAdd(gng, n1);
    nodeAdd(gng, n2);
    edgeNew(gng, n1, n2);
}

static void ferGNGLearn(fer_gng_t *gng, size_t step)
{
    const void *input_signal;
    fer_net_node_t *nn;
    fer_gng_node_t *n1, *n2, *n;
    fer_net_edge_t *nedge;
    fer_gng_edge_t *edge;
    fer_real_t dist2;
    fer_list_t *list, *item, *item_tmp;

    // 1. Get input signal
    input_signal = gng->ops.input_signal(gng->ops.input_signal_data);

    // 2. Find two nearest nodes to input signal
    gng->ops.nearest(input_signal, &n1, &n2, gng->ops.nearest_data);

    // 3. Create connection between n1 and n2 if doesn't exist and set age
    //    to zero
    nedge = ferNetNodeCommonEdge(&n1->node, &n2->node);
    if (!nedge){
        edge = edgeNew(gng, n1, n2);
    }else{
        edge = fer_container_of(nedge, fer_gng_edge_t, edge);
    }
    edge->age = 0;

    // 4. Increase error counter of winner node
    dist2 = gng->ops.dist2(input_signal, n1, gng->ops.dist2_data);
    nodeIncError(gng, n1, dist2 * gng->beta_n[gng->params.lambda - step]);

    // 5. Adapt nodes to input signal using fractions eb and en
    // + 6. Increment age of all edges by one
    // + 7. Remove edges with age higher than age_max
    gng->ops.move_towards(n1, input_signal, gng->params.eb,
                           gng->ops.move_towards_data);
    // adapt also direct topological neighbors of winner node
    list = ferNetNodeEdges(&n1->node);
    FER_LIST_FOR_EACH_SAFE(list, item, item_tmp){
        nedge = ferNetEdgeFromNodeList(item);
        edge  = fer_container_of(nedge, fer_gng_edge_t, edge);
        nn   = ferNetEdgeOtherNode(&edge->edge, &n1->node);
        n    = fer_container_of(nn, fer_gng_node_t, node);

        // increase age (6.)
        edge->age += 1;

        // remove edge if it has age higher than age_max (7.)
        if (edge->age > gng->params.age_max){
            edgeDel(gng, edge);

            if (ferNetNodeEdgesLen(nn) == 0){
                // remove node if not connected into net anymore
                nodeDel(gng, n);
                n = NULL;
            }
        }

        // move node (5.)
        if (n){
            gng->ops.move_towards(n, input_signal, gng->params.en,
                gng->ops.move_towards_data);
        }
    }

    // remove winning node if not connected into net
    if (ferNetNodeEdgesLen(&n1->node) == 0){
        // remove node if not connected into net anymore
        nodeDel(gng, n1);
    }
}

static void ferGNGNewNode(fer_gng_t *gng)
{
    fer_gng_node_t *q, *f, *r;
    fer_gng_edge_t *eqf;

    do {
        // 1. Get node with highest error counter
        q = nodeWithHighestErr(gng);

        // 2. Get q's neighbor with highest error counter
        f = nodeWithHighestErr2(gng, q, &eqf);

        // Node with highest error counter doesn't have any neighbors!
        // Generally, this shouldn't happen but if it does, it means that
        // user had to delete some node from outside. In this case delete
        // the {q} node and try to find next node with highest error
        // counter.
        if (!f){
            nodeDel(gng, q);
        }
    } while (!f);

    // 3. Create new node between q and f
    r = gng->ops.new_node_between(q, f, gng->ops.new_node_between_data);
    nodeAdd(gng, r);

    // 4. Create q-r and f-r edges and remove q-f edge (which is eqf)
    edgeDel(gng, eqf);
    edgeNew(gng, q, r);
    edgeNew(gng, f, r);

    // 5. Decrease error counters of q and f
    nodeScaleError(gng, q, gng->params.alpha);
    nodeScaleError(gng, f, gng->params.alpha);

    // 6. Set error counter of new node (r)
    r->err  = q->err + f->err;
    r->err /= FER_REAL(2.);
    r->err_cycle = gng->cycle;
    ferPairHeapUpdate(gng->err_heap, &r->err_heap);
}



/*** Node functions ***/

_fer_inline void nodeAdd(fer_gng_t *gng, fer_gng_node_t *n)
{
    n->err       = FER_ZERO;
    n->err_cycle = gng->cycle;
    ferPairHeapAdd(gng->err_heap, &n->err_heap);

    ferNetAddNode(gng->net, &n->node);
}

_fer_inline void nodeRemove(fer_gng_t *gng, fer_gng_node_t *n)
{
    ferPairHeapRemove(gng->err_heap, &n->err_heap);
    ferNetRemoveNode(gng->net, &n->node);
}

_fer_inline void nodeDel(fer_gng_t *gng, fer_gng_node_t *n)
{
    nodeRemove(gng, n);
    gng->ops.del_node(n, gng->ops.del_node_data);
}

_fer_inline void nodeFixError(fer_gng_t *gng, fer_gng_node_t *n)
{
    unsigned long diff;

    diff = gng->cycle - n->err_cycle;
    if (diff > 0 && diff <= gng->beta_lambda_n_len){
        n->err *= gng->beta_lambda_n[diff - 1];
    }else if (diff > 0){
        n->err *= gng->beta_lambda_n[gng->params.lambda - 1];

        diff = diff - gng->beta_lambda_n_len;
        n->err *= pow(gng->beta_n[gng->params.lambda - 1], diff);
    }
    n->err_cycle = gng->cycle;
}

_fer_inline void nodeIncError(fer_gng_t *gng, fer_gng_node_t *n,
                              fer_real_t inc)
{
    nodeFixError(gng, n);
    n->err += inc;
    ferPairHeapUpdate(gng->err_heap, &n->err_heap);
}

_fer_inline void nodeScaleError(fer_gng_t *gng, fer_gng_node_t *n,
                                fer_real_t scale)
{
    nodeFixError(gng, n);
    n->err *= scale;
    ferPairHeapUpdate(gng->err_heap, &n->err_heap);
}



/*** Edge functions ***/
_fer_inline fer_gng_edge_t *edgeNew(fer_gng_t *gng, fer_gng_node_t *n1,
                                                    fer_gng_node_t *n2)
{
    fer_gng_edge_t *e;

    e = FER_ALLOC(fer_gng_edge_t);
    e->age = 0;

    ferNetAddEdge(gng->net, &e->edge, &n1->node, &n2->node);

    return e;
}

_fer_inline void edgeDel(fer_gng_t *gng, fer_gng_edge_t *e)
{
    ferNetRemoveEdge(gng->net, &e->edge);
    free(e);
}



static fer_gng_node_t *nodeWithHighestErr(fer_gng_t *gng)
{
    fer_pairheap_node_t *max;
    fer_gng_node_t *maxn;

    max  = ferPairHeapMin(gng->err_heap);
    maxn = fer_container_of(max, fer_gng_node_t, err_heap);

    return maxn;
}

static fer_gng_node_t *nodeWithHighestErr2(fer_gng_t *gng, fer_gng_node_t *q,
                                           fer_gng_edge_t **edge)
{
    fer_list_t *list, *item;
    fer_net_edge_t *ne;
    fer_gng_edge_t *e_highest;
    fer_net_node_t *nn;
    fer_gng_node_t *n, *n_highest;
    fer_real_t err_highest;

    err_highest = -FER_ONE;
    n_highest = NULL;
    e_highest = NULL;

    list = ferNetNodeEdges(&q->node);
    FER_LIST_FOR_EACH(list, item){
        ne = ferNetEdgeFromNodeList(item);
        nn = ferNetEdgeOtherNode(ne, &q->node);
        n  = fer_container_of(nn, fer_gng_node_t, node);

        nodeFixError(gng, n);

        if (n->err > err_highest){
            err_highest = n->err;
            n_highest   = n;
            e_highest   = fer_container_of(ne, fer_gng_edge_t, edge);
        }
    }

    *edge = e_highest;
    return n_highest;
}

static int errHeapLT(const fer_pairheap_node_t *_n1,
                     const fer_pairheap_node_t *_n2,
                     void *data)
{
    fer_gng_t *gng = (fer_gng_t *)data;
    fer_gng_node_t *n1, *n2;

    n1 = fer_container_of(_n1, fer_gng_node_t, err_heap);
    n2 = fer_container_of(_n2, fer_gng_node_t, err_heap);

    nodeFixError(gng, n1);
    nodeFixError(gng, n2);
    return n1->err > n2->err;
}


static void nodeFinalDel(fer_net_node_t *node, void *data)
{
    fer_gng_t *gng = (fer_gng_t *)data;
    fer_gng_node_t *n;

    n = fer_container_of(node, fer_gng_node_t, node);
    gng->ops.del_node(n, gng->ops.del_node_data);
}

static void delEdge(fer_net_edge_t *edge, void *data)
{
    free(edge);
}
