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

#include <stdio.h>
#include <fermat/gng-eu.h>
#include <fermat/nearest-linear.h>
#include <fermat/vec3.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Operations for fer_gng_ops_t struct */
static fer_gng_eu_node_t *ferGNGEuNodeNew(fer_gng_eu_t *gng, const fer_vec_t *is);
static fer_gng_eu_node_t *ferGNGEuNodeNewBetween(fer_gng_eu_t *gng,
                                                 const fer_gng_eu_node_t *n1,
                                                 const fer_gng_eu_node_t *n2);

static const void *ferGNGEuInputSignal(void *);
static void ferGNGEuNearest(fer_gng_eu_t *gng,
                            const fer_vec_t *is,
                            fer_gng_eu_node_t **n1,
                            fer_gng_eu_node_t **n2);

_fer_inline fer_real_t ferGNGEuDist2(fer_gng_eu_t *gng,
                                     const fer_vec_t *is,
                                     const fer_gng_eu_node_t *node);

_fer_inline void ferGNGEuMoveTowards(fer_gng_eu_t *gng,
                                     fer_gng_eu_node_t *node,
                                     const fer_vec_t *is,
                                     fer_real_t fraction);






/** Should return true if n1 > n2 - this is used for err-heap */
static int errHeapLT(const fer_pairheap_node_t *n1,
                     const fer_pairheap_node_t *n2, void *);


/** Delete callbacks */
static void nodeFinalDel(fer_net_node_t *node, void *data);
static void delEdge(fer_net_edge_t *edge, void *data);

void ferGNGEuOpsInit(fer_gng_eu_ops_t *ops)
{
    bzero(ops, sizeof(fer_gng_eu_ops_t));
}

void ferGNGEuParamsInit(fer_gng_eu_params_t *params)
{
    params->dim = 2;

    params->lambda  = 200;
    params->eb      = 0.05;
    params->en      = 0.0006;
    params->alpha   = 0.95;
    params->beta    = 0.9995;
    params->age_max = 200;

    params->use_nn = FER_NN_GUG;
    ferNNParamsInit(&params->nn);
}


fer_gng_eu_t *ferGNGEuNew(const fer_gng_eu_ops_t *ops,
                     const fer_gng_eu_params_t *params)
{
    fer_gng_eu_t *gng_eu;
    fer_nn_params_t nnp;
    size_t i;
    fer_real_t maxbeta;

    gng_eu = FER_ALLOC(fer_gng_eu_t);

    gng_eu->net = ferNetNew();

    gng_eu->ops    = *ops;
    gng_eu->params = *params;

    // set up ops data pointers
    if (!gng_eu->ops.new_node_data)
        gng_eu->ops.new_node_data = gng_eu->ops.data;
    if (!gng_eu->ops.del_node_data)
        gng_eu->ops.del_node_data = gng_eu->ops.data;
    if (!gng_eu->ops.input_signal_data)
        gng_eu->ops.input_signal_data = gng_eu->ops.data;
    if (!gng_eu->ops.terminate_data)
        gng_eu->ops.terminate_data = gng_eu->ops.data;
    if (!gng_eu->ops.callback_data)
        gng_eu->ops.callback_data = gng_eu->ops.data;


    // initialize error heap
    gng_eu->err_heap = ferPairHeapNew(errHeapLT, (void *)gng_eu);

    // precompute beta^n
    gng_eu->beta_n = FER_ALLOC_ARR(fer_real_t, gng_eu->params.lambda);
    gng_eu->beta_n[0] = gng_eu->params.beta;
    for (i = 1; i < gng_eu->params.lambda; i++){
        gng_eu->beta_n[i] = gng_eu->beta_n[i - 1] * gng_eu->params.beta;
    }

    // precompute beta^(n * lambda)
    maxbeta = gng_eu->beta_n[gng_eu->params.lambda - 1];
    gng_eu->beta_lambda_n_len = 1000;
    gng_eu->beta_lambda_n = FER_ALLOC_ARR(fer_real_t, gng_eu->beta_lambda_n_len);
    gng_eu->beta_lambda_n[0] = maxbeta;
    for (i = 1; i < gng_eu->beta_lambda_n_len; i++){
        gng_eu->beta_lambda_n[i] = gng_eu->beta_lambda_n[i - 1] * maxbeta;
    }

    gng_eu->cycle = 1L;
    gng_eu->step  = 1;


    // initialize nncells
    gng_eu->nn = NULL;
    if (params->use_nn){
        nnp = params->nn;
        nnp.gug.dim = params->dim;
        nnp.vptree.dim = params->dim;
        gng_eu->nn = ferNNNew(params->use_nn, &nnp);
    }

    // initialize temporary vector
    if (gng_eu->params.dim == 2){
        gng_eu->tmpv = (fer_vec_t *)ferVec2New(FER_ZERO, FER_ZERO);
    }else if (gng_eu->params.dim == 3){
        gng_eu->tmpv = (fer_vec_t *)ferVec3New(FER_ZERO, FER_ZERO, FER_ZERO);
    }else{
        gng_eu->tmpv = ferVecNew(gng_eu->params.dim);
    }

    return gng_eu;
}

void ferGNGEuDel(fer_gng_eu_t *gng_eu)
{
    if (gng_eu->beta_n)
        FER_FREE(gng_eu->beta_n);
    if (gng_eu->beta_lambda_n)
        FER_FREE(gng_eu->beta_lambda_n);

    if (gng_eu->net){
        ferNetDel2(gng_eu->net, nodeFinalDel, gng_eu,
                              delEdge, gng_eu);
    }

    if (gng_eu->err_heap)
        ferPairHeapDel(gng_eu->err_heap);

    if (gng_eu->nn)
        ferNNDel(gng_eu->nn);

    if (gng_eu->params.dim == 2){
        ferVec2Del((fer_vec2_t *)gng_eu->tmpv);
    }else if (gng_eu->params.dim == 3){
        ferVec3Del((fer_vec3_t *)gng_eu->tmpv);
    }else{
        ferVecDel(gng_eu->tmpv);
    }

    FER_FREE(gng_eu);
}


void ferGNGEuRun(fer_gng_eu_t *gng_eu)
{
    unsigned long cycle;
    size_t i;

    cycle = 0;
    ferGNGEuInit(gng_eu);

    do {
        for (i = 0; i < gng_eu->params.lambda; i++){
            ferGNGEuLearn(gng_eu);
        }
        ferGNGEuNewNode(gng_eu);

        cycle++;
        if (gng_eu->ops.callback && gng_eu->ops.callback_period == cycle){
            gng_eu->ops.callback(gng_eu->ops.callback_data);
            cycle = 0L;
        }
    } while (!gng_eu->ops.terminate(gng_eu->ops.terminate_data));
}

void ferGNGEuInit(fer_gng_eu_t *gng_eu)
{
    const fer_vec_t *is;
    fer_gng_eu_node_t *n1 = NULL, *n2 = NULL;

    gng_eu->cycle = 1L;
    gng_eu->step  = 1;

    is = gng_eu->ops.input_signal(gng_eu->ops.input_signal_data);
    n1 = ferGNGEuNodeNew(gng_eu, is);

    is = gng_eu->ops.input_signal(gng_eu->ops.input_signal_data);
    n2 = ferGNGEuNodeNew(gng_eu, is);

    ferGNGEuEdgeNew(gng_eu, n1, n2);
}

void ferGNGEuLearn(fer_gng_eu_t *gng_eu)
{
    const fer_vec_t *input_signal;
    fer_net_node_t *nn;
    fer_gng_eu_node_t *n1, *n2, *n;
    fer_net_edge_t *nedge;
    fer_gng_eu_edge_t *edge;
    fer_real_t dist2;
    fer_list_t *list, *item, *item_tmp;

    if (gng_eu->step > gng_eu->params.lambda){
        gng_eu->cycle += 1L;
        gng_eu->step = 1;
    }

    // 1. Get input signal
    input_signal = gng_eu->ops.input_signal(gng_eu->ops.input_signal_data);

    // 2. Find two nearest nodes to input signal
    ferGNGEuNearest(gng_eu, input_signal, &n1, &n2);

    // 3. Create connection between n1 and n2 if doesn't exist and set age
    //    to zero
    ferGNGEuHebbianLearning(gng_eu, n1, n2);

    // 4. Increase error counter of winner node
    dist2 = ferGNGEuDist2(gng_eu, input_signal, n1);
    ferGNGEuNodeIncError(gng_eu, n1, dist2 * gng_eu->beta_n[gng_eu->params.lambda - gng_eu->step]);

    // 5. Adapt nodes to input signal using fractions eb and en
    // + 6. Increment age of all edges by one
    // + 7. Remove edges with age higher than age_max
    ferGNGEuMoveTowards(gng_eu, n1, input_signal, gng_eu->params.eb);
    // adapt also direct topological neighbors of winner node
    list = ferNetNodeEdges(&n1->node);
    FER_LIST_FOR_EACH_SAFE(list, item, item_tmp){
        nedge = ferNetEdgeFromNodeList(item);
        edge  = fer_container_of(nedge, fer_gng_eu_edge_t, edge);
        nn   = ferNetEdgeOtherNode(&edge->edge, &n1->node);
        n    = fer_container_of(nn, fer_gng_eu_node_t, node);

        // increase age (6.)
        edge->age += 1;

        // remove edge if it has age higher than age_max (7.)
        if (edge->age > gng_eu->params.age_max){
            ferGNGEuEdgeDel(gng_eu, edge);

            if (ferNetNodeEdgesLen(nn) == 0){
                // remove node if not connected into net anymore
                ferGNGEuNodeDel(gng_eu, n);
                n = NULL;
            }
        }

        // move node (5.)
        if (n){
            ferGNGEuMoveTowards(gng_eu, n, input_signal, gng_eu->params.en);
        }
    }

    // remove winning node if not connected into net
    if (ferNetNodeEdgesLen(&n1->node) == 0){
        // remove node if not connected into net anymore
        ferGNGEuNodeDel(gng_eu, n1);
    }

    ++gng_eu->step;
}

void ferGNGEuNewNode(fer_gng_eu_t *gng_eu)
{
    fer_gng_eu_node_t *q, *f, *r;
    fer_gng_eu_edge_t *eqf;

    // 1. Get node with highest error counter and its neighbor with
    // highest error counter
    ferGNGEuNodeWithHighestError2(gng_eu, &q, &f, &eqf);

    // 3. Create new node between q and f
    r = ferGNGEuNodeNewBetween(gng_eu, q, f);

    // 4. Create q-r and f-r edges and remove q-f edge (which is eqf)
    ferGNGEuEdgeDel(gng_eu, eqf);
    ferGNGEuEdgeNew(gng_eu, q, r);
    ferGNGEuEdgeNew(gng_eu, f, r);

    // 5. Decrease error counters of q and f
    ferGNGEuNodeScaleError(gng_eu, q, gng_eu->params.alpha);
    ferGNGEuNodeScaleError(gng_eu, f, gng_eu->params.alpha);

    // 6. Set error counter of new node (r)
    r->err  = q->err + f->err;
    r->err /= FER_REAL(2.);
    r->err_cycle = gng_eu->cycle;
    ferPairHeapUpdate(gng_eu->err_heap, &r->err_heap);
}


fer_gng_eu_node_t *ferGNGEuNodeWithHighestError(fer_gng_eu_t *gng_eu)
{
    fer_pairheap_node_t *max;
    fer_gng_eu_node_t *maxn;

    max  = ferPairHeapMin(gng_eu->err_heap);
    maxn = fer_container_of(max, fer_gng_eu_node_t, err_heap);

    /*
    if (maxn->err_cycle != gng_eu->cycle){
        DBG2("");
    }

    {
        fer_list_t *list, *item;
        fer_net_node_t *nn;
        fer_gng_eu_node_t *n, *__maxn = NULL;
        fer_real_t max;

        max = -FER_REAL_MAX;
        list = ferNetNodes(gng_eu->net);
        FER_LIST_FOR_EACH(list, item){
            nn = FER_LIST_ENTRY(item, fer_net_node_t, list);
            n  = fer_container_of(nn, fer_gng_eu_node_t, node);

            ferGNGEuNodeFixError(gng_eu, n);
            if (n->err > max){
                max = n->err;
                __maxn = n;
            }
        }

        if (maxn != __maxn){
            DBG("%.30f %.30f", max, maxn->err);
        }
    }
    */

    return maxn;
}

void ferGNGEuHebbianLearning(fer_gng_eu_t *gng_eu,
                             fer_gng_eu_node_t *n1, fer_gng_eu_node_t *n2)
{
    fer_net_edge_t *nedge;
    fer_gng_eu_edge_t *edge;

    nedge = ferNetNodeCommonEdge(&n1->node, &n2->node);
    if (!nedge){
        edge = ferGNGEuEdgeNew(gng_eu, n1, n2);
    }else{
        edge = fer_container_of(nedge, fer_gng_eu_edge_t, edge);
    }
    edge->age = 0;
}

static void __ferGNGEuNodeWithHighestError2(fer_gng_eu_t *gng_eu,
                                          fer_gng_eu_node_t **n1,
                                          fer_gng_eu_node_t **n2,
                                          fer_gng_eu_edge_t **edge)
{
    fer_gng_eu_node_t *q;
    fer_list_t *list, *item;
    fer_net_edge_t *ne;
    fer_gng_eu_edge_t *e_highest;
    fer_net_node_t *nn;
    fer_gng_eu_node_t *n, *n_highest;
    fer_real_t err_highest;

    q = ferGNGEuNodeWithHighestError(gng_eu);

    err_highest = -FER_ONE;
    n_highest = NULL;
    e_highest = NULL;

    list = ferNetNodeEdges(&q->node);
    FER_LIST_FOR_EACH(list, item){
        ne = ferNetEdgeFromNodeList(item);
        nn = ferNetEdgeOtherNode(ne, &q->node);
        n  = fer_container_of(nn, fer_gng_eu_node_t, node);

        ferGNGEuNodeFixError(gng_eu, n);
        ferPairHeapUpdate(gng_eu->err_heap, &n->err_heap);

        if (n->err > err_highest){
            err_highest = n->err;
            n_highest   = n;
            e_highest   = fer_container_of(ne, fer_gng_eu_edge_t, edge);
        }
    }

    if (n1)
        *n1 = q;
    if (n2)
        *n2 = n_highest;
    if (edge)
        *edge = e_highest;
}

void ferGNGEuNodeWithHighestError2(fer_gng_eu_t *gng_eu,
                                 fer_gng_eu_node_t **n1, fer_gng_eu_node_t **n2,
                                 fer_gng_eu_edge_t **edge)
{
    do {
        // 1. Get node with highest error counter and its neighbor with
        // highest error counter
        __ferGNGEuNodeWithHighestError2(gng_eu, n1, n2, edge);

        // Node with highest error counter doesn't have any neighbors!
        // Generally, this shouldn't happen but if it does, it means that
        // user had to delete some node from outside. In this case delete
        // the {n1} node and try to find next node with highest error
        // counter.
        if (n2 && !*n2 && n1 && *n1){
            ferGNGEuNodeDel(gng_eu, *n1);
        }
    } while (n2 && !*n2);
}


void ferGNGEuDumpSVT(fer_gng_eu_t *gng_eu, FILE *out, const char *name)
{
    fer_list_t *list, *item;
    fer_net_node_t *nn;
    fer_gng_eu_node_t *n;
    fer_net_edge_t *e;
    size_t i, id1, id2;

    if (gng_eu->params.dim != 2 && gng_eu->params.dim != 3)
        return;

    fprintf(out, "--------\n");

    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Points:\n");
    list = ferGNGEuNodes(gng_eu);
    i = 0;
    FER_LIST_FOR_EACH(list, item){
        n = ferGNGEuNodeFromList(item);

        n->_id = i++;
        if (gng_eu->params.dim == 2){
            ferVec2Print((const fer_vec2_t *)n->w, out);
        }else{
            ferVec3Print((const fer_vec3_t *)n->w, out);
        }
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = ferGNGEuEdges(gng_eu);
    FER_LIST_FOR_EACH(list, item){
        e = FER_LIST_ENTRY(item, fer_net_edge_t, list);

        nn = ferNetEdgeNode(e, 0);
        n  = ferGNGEuNodeFromNet(nn);
        id1 = n->_id;

        nn = ferNetEdgeNode(e, 1);
        n  = ferGNGEuNodeFromNet(nn);
        id2 = n->_id;
        fprintf(out, "%d %d\n", (int)id1, (int)id2);
    }

    fprintf(out, "--------\n");
}






/*** Node functions ***/
void ferGNGEuNodeDisconnect(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *node)
{
    fer_list_t *edges, *item, *itemtmp;
    fer_net_edge_t *ne;
    fer_gng_eu_edge_t *edge;

    // remove incidenting edges
    edges = ferNetNodeEdges(&node->node);
    FER_LIST_FOR_EACH_SAFE(edges, item, itemtmp){
        ne = ferNetEdgeFromNodeList(item);
        edge = ferGNGEuEdgeFromNet(ne);
        ferGNGEuEdgeDel(gng_eu, edge);
    }
}

fer_gng_eu_node_t *ferGNGEuNodeNewAtPos(fer_gng_eu_t *gng_eu, const void *is)
{
    fer_gng_eu_node_t *r, *n1, *n2;
    fer_gng_eu_edge_t *edge;

    ferGNGEuNearest(gng_eu, is, &n1, &n2);

    r = ferGNGEuNodeNew(gng_eu, (const fer_vec_t *)is);

    edge = ferGNGEuEdgeNew(gng_eu, r, n1);
    edge->age = 0;
    edge = ferGNGEuEdgeNew(gng_eu, r, n2);
    edge->age = 0;

    return r;
}


/*** Edge functions ***/
fer_gng_eu_edge_t *ferGNGEuEdgeNew(fer_gng_eu_t *gng_eu, fer_gng_eu_node_t *n1,
                                              fer_gng_eu_node_t *n2)
{
    fer_gng_eu_edge_t *e;

    e = FER_ALLOC(fer_gng_eu_edge_t);
    e->age = 0;

    ferNetAddEdge(gng_eu->net, &e->edge, &n1->node, &n2->node);

    return e;
}

void ferGNGEuEdgeDel(fer_gng_eu_t *gng_eu, fer_gng_eu_edge_t *e)
{
    ferNetRemoveEdge(gng_eu->net, &e->edge);
    FER_FREE(e);
}

void ferGNGEuEdgeBetweenDel(fer_gng_eu_t *gng_eu,
                          fer_gng_eu_node_t *n1, fer_gng_eu_node_t *n2)
{
    fer_gng_eu_edge_t *e;

    if ((e = ferGNGEuEdgeBetween(gng_eu, n1, n2)) != NULL)
        ferGNGEuEdgeDel(gng_eu, e);
}







static int errHeapLT(const fer_pairheap_node_t *_n1,
                     const fer_pairheap_node_t *_n2,
                     void *data)
{
    fer_gng_eu_t *gng_eu = (fer_gng_eu_t *)data;
    fer_gng_eu_node_t *n1, *n2;

    n1 = fer_container_of(_n1, fer_gng_eu_node_t, err_heap);
    n2 = fer_container_of(_n2, fer_gng_eu_node_t, err_heap);

    ferGNGEuNodeFixError(gng_eu, n1);
    ferGNGEuNodeFixError(gng_eu, n2);
    return n1->err > n2->err;
}


static void nodeFinalDel(fer_net_node_t *node, void *data)
{
    fer_gng_eu_t *gng_eu = (fer_gng_eu_t *)data;
    fer_gng_eu_node_t *n;

    n = fer_container_of(node, fer_gng_eu_node_t, node);
    ferGNGEuNodeDel(gng_eu, n);
}

static void delEdge(fer_net_edge_t *edge, void *data)
{
    FER_FREE(edge);
}




static fer_gng_eu_node_t *ferGNGEuNodeNew(fer_gng_eu_t *gng, const fer_vec_t *is)
{
    fer_gng_eu_node_t *n;

    if (gng->ops.new_node){
        n = gng->ops.new_node(is, gng->ops.new_node_data);
    }else{
        n = FER_ALLOC(fer_gng_eu_node_t);
    }
    ferGNGEuNodeAdd(gng, n, is);

    return n;
}

static fer_gng_eu_node_t *ferGNGEuNodeNewBetween(fer_gng_eu_t *gng,
                                                 const fer_gng_eu_node_t *n1,
                                                 const fer_gng_eu_node_t *n2)
{
    if (gng->params.dim == 2){
        ferVec2Add2((fer_vec2_t *)gng->tmpv, (const fer_vec2_t *)n1->w,
                                             (const fer_vec2_t *)n2->w);
        ferVec2Scale((fer_vec2_t *)gng->tmpv, FER_REAL(0.5));
    }else if (gng->params.dim == 3){
        ferVec3Add2((fer_vec3_t *)gng->tmpv, (const fer_vec3_t *)n1->w,
                                             (const fer_vec3_t *)n2->w);
        ferVec3Scale((fer_vec3_t *)gng->tmpv, FER_REAL(0.5));
    }else{
        ferVecAdd2(gng->params.dim, gng->tmpv, n1->w, n2->w);
        ferVecScale(gng->params.dim, gng->tmpv, FER_REAL(0.5));
    }

    return ferGNGEuNodeNew(gng, gng->tmpv);
}

static const void *ferGNGEuInputSignal(void *data)
{
    /*
    fer_gng_eu_t *gng = (fer_gng_eu_t *)data;
    const fer_vec_t *v;

    if (ferPCItEnd(&gng->pcit)){
        ferPCPermutate(gng->pc);
        ferPCItInit(&gng->pcit, gng->pc);
    }

    v = ferPCItGet(&gng->pcit);
    ferPCItNext(&gng->pcit);

    return v;
    */
    // TODO
    return NULL;
}


static fer_real_t dist22(void *is, fer_list_t *nlist, void *data)
{
    fer_gng_eu_t *gng = (fer_gng_eu_t *)data;
    fer_gng_eu_node_t *n;

    n = ferGNGEuNodeFromList(nlist);
    if (gng->params.dim == 2){
        return ferVec2Dist2((const fer_vec2_t *)is, (const fer_vec2_t *)n->w);
    }else if (gng->params.dim == 3){
        return ferVec3Dist2((const fer_vec3_t *)is, (const fer_vec3_t *)n->w);
    }else{
        return ferVecDist2(gng->params.dim, (const fer_vec_t *)is, n->w);
    }
}


_fer_inline void ferGNGEuNearestLinear(fer_gng_eu_t *gng,
                                       const fer_vec_t *is,
                                       fer_gng_eu_node_t **n1,
                                       fer_gng_eu_node_t **n2)
{
    fer_list_t *ns[2];

    ns[0] = ns[1] = NULL;
    ferNearestLinear(ferGNGEuNodes(gng), (void *)is, dist22, ns, 2,
                     (void *)gng);

    *n1 = ferGNGEuNodeFromList(ns[0]);
    *n2 = ferGNGEuNodeFromList(ns[1]);
}

_fer_inline void ferGNGEuNearestCells(fer_gng_eu_t *gng,
                                      const fer_vec_t *is,
                                      fer_gng_eu_node_t **n1,
                                      fer_gng_eu_node_t **n2)
{
    fer_nn_el_t *els[2];

    *n1 = *n2 = NULL;

    ferNNNearest(gng->nn, is, 2, els);

    *n1 = fer_container_of(els[0], fer_gng_eu_node_t, nn);
    *n2 = fer_container_of(els[1], fer_gng_eu_node_t, nn);
}

static void ferGNGEuNearest(fer_gng_eu_t *gng,
                            const fer_vec_t *is,
                            fer_gng_eu_node_t **n1,
                            fer_gng_eu_node_t **n2)
{
    if (gng->nn){
        ferGNGEuNearestCells(gng, is, n1, n2);
    }else{
        ferGNGEuNearestLinear(gng, is, n1, n2);
    }
}



_fer_inline fer_real_t ferGNGEuDist2(fer_gng_eu_t *gng,
                                     const fer_vec_t *is,
                                     const fer_gng_eu_node_t *n)
{
    if (gng->params.dim == 2){
        return ferVec2Dist2((const fer_vec2_t *)is, (const fer_vec2_t *)n->w);
    }else if (gng->params.dim == 3){
        return ferVec3Dist2((const fer_vec3_t *)is, (const fer_vec3_t *)n->w);
    }else{
        return ferVecDist2(gng->params.dim, is, n->w);
    }
}

_fer_inline void ferGNGEuMoveTowards(fer_gng_eu_t *gng,
                                     fer_gng_eu_node_t *n,
                                     const fer_vec_t *is,
                                     fer_real_t fraction)
{
    if (gng->params.dim == 2){
        ferVec2Sub2((fer_vec2_t *)gng->tmpv, (const fer_vec2_t *)is,
                                             (const fer_vec2_t *)n->w);
        ferVec2Scale((fer_vec2_t *)gng->tmpv, fraction);
        ferVec2Add((fer_vec2_t *)n->w, (const fer_vec2_t *)gng->tmpv);
    }else if (gng->params.dim == 3){
        ferVec3Sub2((fer_vec3_t *)gng->tmpv, (const fer_vec3_t *)is,
                                             (const fer_vec3_t *)n->w);
        ferVec3Scale((fer_vec3_t *)gng->tmpv, fraction);
        ferVec3Add((fer_vec3_t *)n->w, (const fer_vec3_t *)gng->tmpv);
    }else{
        ferVecSub2(gng->params.dim, gng->tmpv, is, n->w);
        ferVecScale(gng->params.dim, gng->tmpv, fraction);
        ferVecAdd(gng->params.dim, n->w, gng->tmpv);
    }

    if (gng->nn){
        ferNNUpdate(gng->nn, &n->nn);
    }
}
