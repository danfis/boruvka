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

/**
 * Here is implemented Growing Neural Gas algorithm  completely in static
 * functions. The purpose is to be able to get rid of some indirect calls
 * of functions (see fer_gng_t.ops struct).
 *
 * Indirect calls can be very expensive, so if you need to speed up
 * an algorithm and you really know what you are doing you can use these
 * functions.
 *
 * All functions marked as "public API" corresponds to same GNGs' functions
 * without initial underscore.
 *
 * Before including this file macros OPS() and OPS_DATA() must be defined
 * because these macros are used to expand to "operation" calls as defined
 * in fer_gng_ops_t struct.
 * Operations that are covered by OPS() and OPS_DATA() macros are:
 *    - new_node
 *    - new_node_between
 *    - input_signal
 *    - nearest
 *    - dist2
 *    - move_towards
 *
 * For example how to use this file look into gng.c.
 */


/** For public API */
static void _ferGNGRun(fer_gng_t *gng);
static void _ferGNGInit(fer_gng_t *gng);
static void _ferGNGLearn(fer_gng_t *gng, size_t step);
static void _ferGNGNewNode(fer_gng_t *gng);
#ifndef NO_CONNECT_NEW_NODE
static fer_gng_node_t *_ferGNGConnectNewNode(fer_gng_t *gng, const void *is);
#endif

/** Node functions */
/** Adds node into network */
_fer_inline void nodeAdd(fer_gng_t *gng, fer_gng_node_t *n);
/** Removes node from network */
_fer_inline void nodeRemove(fer_gng_t *gng, fer_gng_node_t *n);
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

static void _ferGNGRun(fer_gng_t *gng)
{
    unsigned long cycle;

    cycle = 0;
    _ferGNGInit(gng);

    do {
        for (gng->step = 1; gng->step <= gng->params.lambda; gng->step++){
            _ferGNGLearn(gng, gng->step);
        }
        _ferGNGNewNode(gng);

        cycle++;
        if (gng->ops.callback && gng->ops.callback_period == cycle){
            gng->ops.callback(gng->ops.callback_data);
            cycle = 0L;
        }

        gng->cycle++;
    } while (!gng->ops.terminate(gng->ops.terminate_data));
}

static void _ferGNGInit(fer_gng_t *gng)
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
        OPS(gng, init)(&n1, &n2, OPS_DATA(gng, init));
    }else{
        is = OPS(gng, input_signal)(OPS_DATA(gng, input_signal));
        n1 = OPS(gng, new_node)(is, OPS_DATA(gng, new_node));

        is = OPS(gng, input_signal)(OPS_DATA(gng, input_signal));
        n2 = OPS(gng, new_node)(is, OPS_DATA(gng, new_node));
    }

    nodeAdd(gng, n1);
    nodeAdd(gng, n2);
    edgeNew(gng, n1, n2);
}

static void _ferGNGLearn(fer_gng_t *gng, size_t step)
{
    const void *input_signal;
    fer_net_node_t *nn;
    fer_gng_node_t *n1, *n2, *n;
    fer_net_edge_t *nedge;
    fer_gng_edge_t *edge;
    fer_real_t dist2;
    fer_list_t *list, *item, *item_tmp;

    // 1. Get input signal
    input_signal = OPS(gng, input_signal)(OPS_DATA(gng, input_signal));

    // 2. Find two nearest nodes to input signal
    OPS(gng, nearest)(input_signal, &n1, &n2, OPS_DATA(gng, nearest));

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
    dist2 = OPS(gng, dist2)(input_signal, n1, OPS_DATA(gng, dist2));
    nodeIncError(gng, n1, dist2 * gng->beta_n[gng->params.lambda - step]);

    // 5. Adapt nodes to input signal using fractions eb and en
    // + 6. Increment age of all edges by one
    // + 7. Remove edges with age higher than age_max
    OPS(gng, move_towards)(n1, input_signal, gng->params.eb,
                           OPS_DATA(gng, move_towards));
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
                nodeRemove(gng, n);
                n = NULL;
            }
        }

        // move node (5.)
        if (n){
            OPS(gng, move_towards)(n, input_signal, gng->params.en,
                OPS_DATA(gng, move_towards));
        }
    }

    // remove winning node if not connected into net
    if (ferNetNodeEdgesLen(&n1->node) == 0){
        // remove node if not connected into net anymore
        nodeRemove(gng, n1);
    }
}

static void _ferGNGNewNode(fer_gng_t *gng)
{
    fer_gng_node_t *q, *f, *r;
    fer_gng_edge_t *eqf;

    // 1. Get node with highest error counter
    q = nodeWithHighestErr(gng);

    // 2. Get q's neighbor with highest error counter
    f = nodeWithHighestErr2(gng, q, &eqf);
    if (!f){
        ERR2("Node with highest error counter doesn't have any neighbors! "
             "This shouldn't happen - something's wrong with algorithm.");
        return;
    }

    // 3. Create new node between q and f
    r = OPS(gng, new_node_between)(q, f, OPS_DATA(gng, new_node_between));
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

#ifndef NO_CONNECT_NEW_NODE
static fer_gng_node_t *_ferGNGConnectNewNode(fer_gng_t *gng, const void *is)
{
    fer_gng_node_t *r, *n1, *n2;
    fer_gng_edge_t *edge;

    OPS(gng, nearest)(is, &n1, &n2, OPS_DATA(gng, nearest));

    r = OPS(gng, new_node)(is, OPS_DATA(gng, new_node));
    nodeAdd(gng, r);

    edge = edgeNew(gng, r, n1);
    edge->age = 0;
    edge = edgeNew(gng, r, n2);
    edge->age = 0;

    return r;
}
#endif


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
