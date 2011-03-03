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
static fer_gng_node_t *_ferGNGConnectNewNode(fer_gng_t *gng, const void *is);

/** Node functions */
/** Initialize node */
_fer_inline void nodeInit(fer_gng_t *gng, fer_gng_node_t *n);
/** Adds node into network */
_fer_inline void nodeAdd(fer_gng_t *gng, fer_gng_node_t *n);

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

static void _ferGNGRun(fer_gng_t *gng)
{
    size_t step;
    unsigned long cb_step;

    _ferGNGInit(gng);

    step = 1;
    cb_step = 1L;
    while (!gng->ops.terminate(gng->ops.terminate_data)){
        _ferGNGLearn(gng, step);

        if (fer_unlikely(step >= gng->params.lambda)){
            _ferGNGNewNode(gng);
            step = 0;

            if (gng->ops.callback
                    && cb_step == gng->ops.callback_period){
                gng->ops.callback(gng->ops.callback_data);
                cb_step = 0L;
            }
            cb_step++;
        }

        step++;
    }
}

static void _ferGNGInit(fer_gng_t *gng)
{
    const void *is;
    fer_gng_node_t *n1, *n2;
    size_t i;

    // precompute beta^n
    if (gng->beta_n)
        free(gng->beta_n);
    gng->beta_n = FER_ALLOC_ARR(fer_real_t, gng->params.lambda);
    gng->beta_n[0] = gng->params.beta;
    for (i = 1; i < gng->params.lambda; i++){
        gng->beta_n[i] = gng->beta_n[i - 1] * gng->params.beta;
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
    n1->err_local += dist2 * gng->beta_n[gng->params.lambda - step];

    // 5. Adapt nodes to input signal using fractions eb and en
    // + 6. Increment age of all edges by one
    // + 7. Remove edges with age higher than age_max
    OPS(gng, move_towards)(n1, input_signal, gng->params.eb,
                           OPS_DATA(gng, move_towards));
    // adapt also direct topological neighbors of winner node
    list = ferNetNodeEdges(&n1->node);
    ferListForEachSafe(list, item, item_tmp){
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
                ferNetRemoveNode(gng->net, nn);
                gng->ops.del_node(n, gng->ops.del_node_data);
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
        ferNetRemoveNode(gng->net, &n1->node);
        gng->ops.del_node(n1, gng->ops.del_node_data);
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
    q->err *= gng->params.alpha;
    f->err *= gng->params.alpha;

    // 6. Set error counter of new node (r)
    r->err  = q->err + f->err;
    r->err /= FER_REAL(2.);
}

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


/*** Node functions ***/

_fer_inline void nodeInit(fer_gng_t *gng, fer_gng_node_t *n)
{
    n->err_local = FER_ZERO;
    n->err = FER_ZERO;
}

_fer_inline void nodeAdd(fer_gng_t *gng, fer_gng_node_t *n)
{
    nodeInit(gng, n);
    ferNetAddNode(gng->net, &n->node);
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
    fer_list_t *list, *item;
    fer_net_node_t *nn;
    fer_gng_node_t *n, *n_highest;
    fer_real_t err_highest;

    err_highest = -FER_ONE;
    n_highest   = NULL;

    list = ferNetNodes(gng->net);
    ferListForEach(list, item){
        nn = ferListEntry(item, fer_net_node_t, list);
        n  = fer_container_of(nn, fer_gng_node_t, node);

        n->err  = n->err * gng->beta_n[gng->params.lambda];
        n->err += n->err_local;
        n->err_local = FER_ZERO;

        if (n->err > err_highest){
            err_highest = n->err;
            n_highest   = n;
        }
    }

    return n_highest;
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
    ferListForEach(list, item){
        ne = ferNetEdgeFromNodeList(item);
        nn = ferNetEdgeOtherNode(ne, &q->node);
        n  = fer_container_of(nn, fer_gng_node_t, node);

        if (n->err > err_highest){
            err_highest = n->err;
            n_highest   = n;
            e_highest   = fer_container_of(ne, fer_gng_edge_t, edge);
        }
    }

    *edge = e_highest;
    return n_highest;
}

