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

#include <string.h>
#include <fermat/gng-plan.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Initializes algorithm */
static void init(fer_gngp_t *gng);
/** Adapts net to input signal number of step (1, ..., lambda) must be
 *  provided */
static void adapt(fer_gngp_t *gng);
/** Adds new node into net */
static void newNode(fer_gngp_t *gng);
/** Finds two nearest nodes to given input signal */
static int nearest(fer_gngp_t *gng, const fer_vec_t *w,
                   fer_gngp_node_t **n1, fer_gngp_node_t **n2);
/** Learns the part of net around given node. It will adapt it to given
 *  input signal. Also edges with age > age_max are removed along with
 *  abandonded nodes. */
static void learn(fer_gngp_t *gng, size_t step,
                  fer_gngp_node_t *n, const fer_vec_t *is);
/** Returns node with highest error */
static fer_gngp_node_t *nodeWithMaxErr(fer_gngp_t *gng);
/** Returns node's neighbor with highest error */
static fer_gngp_node_t *nodeNeighborWithMaxErr(fer_gngp_t *gng, fer_gngp_node_t *n);
/** Cuts subnet containing given node */
static void cutSubnet(fer_gngp_t *gng, fer_gngp_node_t *m);
/** Creates new node at given position and connects it between two nearest
 *  nodes */
static fer_gngp_node_t *connectNewNode(fer_gngp_t *gng, const fer_vec_t *w);

/*** Find path ***/
static void findPathExpand(fer_dij_node_t *_n, fer_list_t *expand, void *_);
/** Initializes all nodes in net for dijkstra search */
static void findPathDijInit(fer_gngp_t *gng);
/** Fills given list by path from s to g.
 *  It is assumed that the path exists! */
static void obtainPath(fer_gngp_node_t *s, fer_gngp_node_t *g,
                       fer_list_t *list);

/** Should return true if n1 > n2 - this is used for err-heap */
static int errHeapLT(const fer_pairheap_node_t *n1,
                     const fer_pairheap_node_t *n2, void *);




/*** Node functions ***/
static fer_gngp_node_t *nodeNew(fer_gngp_t *gng, const fer_vec_t *w);
static void nodeDel(fer_gngp_t *gng, fer_gngp_node_t *n);
static void nodeDelWithEdges(fer_gngp_t *gng, fer_gngp_node_t *n);
static void nodeChangeSet(fer_gngp_t *gng, fer_gngp_node_t *n, int set);
static void nodeMoveTowards(fer_gngp_t *gng, fer_gngp_node_t *n,
                            const fer_vec_t *to, fer_real_t frac,
                            int incerr);
/** Fixes node's error counter, i.e. applies correct beta^(n * lambda) */
_fer_inline void nodeFixError(fer_gngp_t *gng, fer_gngp_node_t *n);
/** Increment error counter */
_fer_inline void nodeIncError(fer_gngp_t *gng, fer_gngp_node_t *n,
                              fer_real_t inc);
/** Scales error counter */
_fer_inline void nodeScaleError(fer_gngp_t *gng, fer_gngp_node_t *n,
                                fer_real_t scale);
//_fer_inline void nodeCheckError(fer_gngp_t *gng, fer_gngp_node_t *n);
static void netNodeDel(fer_net_node_t *n, void *);

/*** Edge functions ***/
static fer_gngp_edge_t *edgeNew(fer_gngp_t *gng, fer_gngp_node_t *n1,
                                                   fer_gngp_node_t *n2);
static void edgeDel(fer_gngp_t *gng, fer_gngp_edge_t *e);
static void netEdgeDel(fer_net_edge_t *n, void *);


void ferGNGPOpsInit(fer_gngp_ops_t *ops)
{
    memset(ops, 0, sizeof(fer_gngp_ops_t));
}

void ferGNGPParamsInit(fer_gngp_params_t *params)
{
    params->d       = 2;
    params->lambda  = 200;
    params->eb      = 0.05;
    params->en      = 0.0006;
    params->alpha   = 0.95;
    params->beta    = 0.9995;
    params->age_max = 200;

    params->warm_start = 5000;

    ferGUGParamsInit(&params->gug);
}

fer_gngp_t *ferGNGPNew(const fer_gngp_ops_t *ops,
                       const fer_gngp_params_t *params)
{
    fer_gngp_t *gng;
    fer_gug_params_t pgug;

    gng = FER_ALLOC(fer_gngp_t);

    gng->net   = ferNetNew();

    pgug   = params->gug;
    pgug.dim = params->d;
    gng->gug = ferGUGNew(&pgug);

    gng->params = *params;
    gng->beta_n = NULL;
    gng->beta_lambda_n = NULL;

    gng->ops = *ops;
    if (gng->ops.input_signal_data == NULL)
        gng->ops.input_signal_data = gng->ops.data;
    if (gng->ops.terminate_data == NULL)
        gng->ops.terminate_data = gng->ops.data;
    if (gng->ops.eval_data == NULL)
        gng->ops.eval_data = gng->ops.data;
    if (gng->ops.callback_data == NULL)
        gng->ops.callback_data = gng->ops.data;
    // TODO: check for non-null ops

    gng->set_size[0] = gng->set_size[1] = gng->set_size[2] = 0;

    gng->err_heap = NULL;

    return gng;
}

void ferGNGPDel(fer_gngp_t *gng)
{
    if (gng->gug)
        ferGUGDel(gng->gug);

    if (gng->net){
        ferNetDel2(gng->net, netNodeDel, NULL,
                             netEdgeDel, NULL);
    }

    if (gng->beta_n)
        free(gng->beta_n);
    if (gng->beta_lambda_n)
        free(gng->beta_lambda_n);
    if (gng->err_heap)
        ferPairHeapDel(gng->err_heap);

    free(gng);
}

void ferGNGPRun(fer_gngp_t *gng)
{
    size_t cycle;

    cycle = 0;
    init(gng);
    do {
        for (gng->step = 1; gng->step <= gng->params.lambda; gng->step++){
            adapt(gng);
        }
        newNode(gng);

        cycle++;
        if (gng->ops.callback && gng->ops.callback_period == cycle){
            gng->ops.callback(gng->ops.callback_data);
            cycle = 0;
        }

        gng->cycle++;
    } while (!gng->ops.terminate(gng->ops.terminate_data));
}

static void dumpSet(fer_gngp_t *gng, int set, FILE *out, const char *name,
                    const char *setname, const char *color)
{
    fer_list_t *list, *item;
    fer_net_node_t *nn;
    fer_gngp_node_t *n;
    fer_net_edge_t *e;
    size_t i, id1, id2;

    fprintf(out, "--------\n");
    fprintf(out, "Edge color: %s\n", color);

    if (name){
        fprintf(out, "Name: %s (%s)\n", name, setname);
    }else{
        fprintf(out, "Name: (%s)\n", setname);
    }

    fprintf(out, "Points:\n");
    list = ferNetNodes(gng->net);
    i = 0;
    FER_LIST_FOR_EACH(list, item){
        nn = FER_LIST_ENTRY(item, fer_net_node_t, list);
        n  = fer_container_of(nn, fer_gngp_node_t, node);
        if (n->set != set)
            continue;

        n->_id = i++;
        ferVecPrint(gng->params.d, n->w, out);
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = ferNetEdges(gng->net);
    FER_LIST_FOR_EACH(list, item){
        e = FER_LIST_ENTRY(item, fer_net_edge_t, list);

        nn = ferNetEdgeNode(e, 0);
        n  = fer_container_of(nn, fer_gngp_node_t, node);
        id1 = n->_id;
        if (n->set != set)
            continue;

        nn = ferNetEdgeNode(e, 1);
        n  = fer_container_of(nn, fer_gngp_node_t, node);
        id2 = n->_id;
        if (n->set != set)
            continue;

        fprintf(out, "%d %d\n", (int)id1, (int)id2);
    }

    fprintf(out, "--------\n");
}

void ferGNGPDumpSVT(fer_gngp_t *gng, FILE *out, const char *name)
{
    if (gng->set_size[FER_GNGP_NONE] > 0){
        dumpSet(gng, FER_GNGP_NONE, out, name, "none", "0.7 0.7 0.7");
    }else{
        fprintf(out, "Points:\n0 0 0\nPoint size: 0\n---\n");
    }

    if (gng->set_size[FER_GNGP_FREE] > 0){
        dumpSet(gng, FER_GNGP_FREE, out, name, "free", "0 0 0.8");
    }else{
        fprintf(out, "Points:\n0 0 0\nPoint size: 0\n---\n");
    }

    if (gng->set_size[FER_GNGP_OBST] > 0){
        dumpSet(gng, FER_GNGP_OBST, out, name, "obst", "0.8 0 0");
    }else{
        fprintf(out, "Points:\n0 0 0\nPoint size: 0\n---\n");
    }
}

void ferGNGPDumpNodes(fer_gngp_t *gng, FILE *out)
{
    fer_list_t *list, *item;
    fer_net_node_t *nn;
    fer_gngp_node_t *n;

    list = ferNetNodes(gng->net);
    FER_LIST_FOR_EACH(list, item){
        nn = FER_LIST_ENTRY(item, fer_net_node_t, list);
        n  = fer_container_of(nn, fer_gngp_node_t, node);

        ferVecPrint(gng->params.d, n->w, out);
        fprintf(out, " %d\n", n->set);
    }
}


static void init(fer_gngp_t *gng)
{
    const fer_vec_t *is;
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

    // create two initial nodes
    is = gng->ops.input_signal(gng->ops.input_signal_data);
    nodeNew(gng, is);
    is = gng->ops.input_signal(gng->ops.input_signal_data);
    nodeNew(gng, is);
}

static void adapt(fer_gngp_t *gng)
{
    const fer_vec_t *is;
    fer_gngp_node_t *n1 = NULL, *n2 = NULL;
    fer_gngp_edge_t *e;
    fer_net_edge_t *edge;
    int set;

    // 1. Get random input signal
    is = gng->ops.input_signal(gng->ops.input_signal_data);

    // 2. Find two nearest nodes (n1, n2)
    nearest(gng, is, &n1, &n2);

    // 3. Learn net according to set of n1 and n2
    if (n1->set == n2->set
            || n1->set == FER_GNGP_NONE
            || n2->set == FER_GNGP_NONE){
        // n1 and n2 both belong to same set (free or obstacle)
        // or we don't know to which set belongs one of them

        // 3.1. Create edge n1-n2 if doesn't exist
        edge = ferNetNodeCommonEdge(&n1->node, &n2->node);
        if (!edge){
            e = edgeNew(gng, n1, n2);
        }else{
            e = fer_container_of(edge, fer_gngp_edge_t, edge);
        }

        // 3.2. Set age of edge to zero
        e->age = 0;

        // 3.3. Learn net around n1
        learn(gng, gng->step, n1, is);

    }else{
        set = gng->ops.eval(is, gng->ops.eval_data);

        if (set == n1->set){
            learn(gng, gng->step, n1, is);
        }else{
            learn(gng, gng->step, n2, is);
        }
    }
}

static void newNode(fer_gngp_t *gng)
{
    fer_gngp_node_t *n1, *n2, *m;
    fer_net_edge_t *edge;
    fer_gngp_edge_t *e;
    fer_vec_t *w;
    int set;

    w = ferVecNew(gng->params.d);

    // 1. Get node with highest error and its neighbor with highest error
    n1 = nodeWithMaxErr(gng);
    n2 = nodeNeighborWithMaxErr(gng, n1);
    if (!n1 || !n2){
        // TODO: put assert here - this shouldn't happen
        DBG("%d", (int)ferNetNodeEdgesLen(&n1->node));
        DBG("%lx %lx", (long)n1, (long)n2);
        fprintf(stderr, "Error: It somehow happens that there is unconnected "
                        "node in mesh. This is error that shouldn't happen!\n");
        return;
    }

    // 2. Create new node between n1 and n2
    ferVecAdd2(gng->params.d, w, n1->w, n2->w);
    ferVecScale(gng->params.d, w, FER_REAL(0.5));
    m = nodeNew(gng, w);

    // 3. Create edges m-n1 and m-n2 and remove edge n1-n2
    edgeNew(gng, m, n1);
    edgeNew(gng, m, n2);
    edge = ferNetNodeCommonEdge(&n1->node, &n2->node);
    e    = fer_container_of(edge, fer_gngp_edge_t, edge);
    edgeDel(gng, e);

    // 4. Decrease error of and n1 and n2 and set up error of m as average
    // of n1 and n2
    nodeScaleError(gng, n1, gng->params.alpha);
    nodeScaleError(gng, n2, gng->params.alpha);
    m->err  = n1->err + n2->err;
    m->err /= FER_REAL(2.);
    m->err_cycle = gng->cycle;
    ferPairHeapUpdate(gng->err_heap, &m->err_heap);

    if (ferNetNodesLen(gng->net) > gng->params.warm_start){
        // 5. Evaluate new node and set up its set properly
        set = gng->ops.eval(m->w, gng->ops.eval_data);
        nodeChangeSet(gng, m, set);

        // 6. Cut m's subnet if necessary
        cutSubnet(gng, m);
        if (ferNetNodeEdgesLen(&m->node) == 0){
            // the new node was completely cut from net, so there is some
            // kind of island surrounded by nodes from other set.
            // So, create the island by three nodes positioned at same
            // place - we will assume that island will spread.

            // create new nodes
            n1 = nodeNew(gng, m->w);
            n2 = nodeNew(gng, m->w);

            // connect triplet of nodes
            edgeNew(gng, m, n1);
            edgeNew(gng, m, n2);
            edgeNew(gng, n1, n2);

            // assign all nodes to same set
            nodeChangeSet(gng, n1, set);
            nodeChangeSet(gng, n2, set);

            // set error of all nodes to same value
            n1->err = n2->err = m->err;
            ferPairHeapUpdate(gng->err_heap, &n1->err_heap);
            ferPairHeapUpdate(gng->err_heap, &n2->err_heap);
        }
    }

    ferVecDel(w);
}

static int nearest(fer_gngp_t *gng, const fer_vec_t *w,
                   fer_gngp_node_t **n1, fer_gngp_node_t **n2)
{
    fer_gug_el_t *els[2];
    fer_gngp_node_t *n;
    size_t found;

    els[0] = els[1] = NULL;
    found = ferGUGNearest(gng->gug, w, 2, els);
    if (found != 2){
        DBG2("Not found two nearest nodes! This shouldn't happen!");
        return -1;
    }

    n = fer_container_of(els[0], fer_gngp_node_t, gug);
    *n1 = n;
    n = fer_container_of(els[1], fer_gngp_node_t, gug);
    *n2 = n;

    return 0;
}

static void learn(fer_gngp_t *gng, size_t step,
                  fer_gngp_node_t *n, const fer_vec_t *is)
{
    fer_list_t *list, *item, *item_tmp;
    fer_net_node_t *other;
    fer_gngp_node_t *o;
    fer_net_edge_t *edge;
    fer_gngp_edge_t *e;


    // increase age of all outgoing edges from n, move all neighbor nodes
    // of n towards input signal and remove all edges with age > age_max
    list = ferNetNodeEdges(&n->node);
    FER_LIST_FOR_EACH_SAFE(list, item, item_tmp){
        edge  = ferNetEdgeFromNodeList(item);
        e     = fer_container_of(edge, fer_gngp_edge_t, edge);
        other = ferNetEdgeOtherNode(edge, &n->node);
        o     = fer_container_of(other, fer_gngp_node_t, node);

        // increase age
        e->age += 1;

        // remove edge if its age is above age_max, otherwise move node o
        // towards input signal
        if (e->age > gng->params.age_max){
            edgeDel(gng, e);

            // remove also o if not connected into net
            if (ferNetNodeEdgesLen(other) == 0){
                nodeDel(gng, o);
            }
        }else{
            // move o towards input signal
            nodeMoveTowards(gng, o, is, gng->params.en, 0);
        }
    }

    if (ferNetNodeEdgesLen(&n->node) == 0){
        // remove node if it's not connected into net anymore
        nodeDel(gng, n);
    }else{
        // move node towards input signal and increase error
        nodeMoveTowards(gng, n, is, gng->params.eb, 1);
    }
}

static fer_gngp_node_t *nodeWithMaxErr(fer_gngp_t *gng)
{
    fer_pairheap_node_t *max;
    fer_gngp_node_t *maxn;

    max  = ferPairHeapMin(gng->err_heap);
    maxn = fer_container_of(max, fer_gngp_node_t, err_heap);

#if 0
    {
        fer_list_t *list, *item;
        fer_net_node_t *node;
        fer_gngp_node_t *n;
        fer_gngp_node_t *mmax;

        mmax = NULL;
        list = ferNetNodes(gng->net);
        FER_LIST_FOR_EACH(list, item){
            node = FER_LIST_ENTRY(item, fer_net_node_t, list);
            n    = fer_container_of(node, fer_gngp_node_t, node);

            if (!mmax || n->err > mmax->err){
                mmax = n;
            }
        }
        if (mmax != maxn){
            DBG("mmax (%lx) != maxn (%lx) [%d]", (long)mmax, (long)maxn, (int)ferNetNodesLen(gng->net));
            DBG("mmax->err: %g, maxn->err: %g", mmax->err, maxn->err);
        }
    }
#endif

    return maxn;
}


static fer_gngp_node_t *nodeNeighborWithMaxErr(fer_gngp_t *gng, fer_gngp_node_t *n)
{
    fer_list_t *list, *item;
    fer_net_node_t *other;
    fer_gngp_node_t *o;
    fer_net_edge_t *edge;
    fer_gngp_node_t *max_n;
    fer_real_t max_err;

    max_err = -FER_ONE;
    max_n   = NULL;

    list = ferNetNodeEdges(&n->node);
    FER_LIST_FOR_EACH(list, item){
        edge  = ferNetEdgeFromNodeList(item);
        other = ferNetEdgeOtherNode(edge, &n->node);
        o     = fer_container_of(other, fer_gngp_node_t, node);

        nodeFixError(gng, o);

        if (o->err > max_err){
            max_err = o->err;
            max_n   = o;
        }
    }

    return max_n;
}

static void cutSubnet(fer_gngp_t *gng, fer_gngp_node_t *m)
{
    fer_list_t fifo;
    fer_list_t *list, *item, *item_tmp;
    fer_net_edge_t *edge;
    fer_net_node_t *node;
    fer_gngp_node_t *n, *o;
    fer_gngp_edge_t *e;
    int set;

    // 1. Initialize FIFO queue
    ferListInit(&fifo);

    // 2. Add m into fifo
    ferListAppend(&fifo, &m->fifo);
    m->evaled = gng->cycle;

    //DBG("rank(m): %d", ferNetNodeEdgesLen(&m->node));

    while (!ferListEmpty(&fifo)){
        // Pop next item form fifo
        item = ferListNext(&fifo);
        ferListDel(item);
        n = FER_LIST_ENTRY(item, fer_gngp_node_t, fifo);

        //DBG("  rank(n): %d", ferNetNodeEdgesLen(&n->node));
        // Iterate over n's neighbors that are _not_ in same set as m
        list = ferNetNodeEdges(&n->node);
        FER_LIST_FOR_EACH_SAFE(list, item, item_tmp){
            edge = ferNetEdgeFromNodeList(item);
            node = ferNetEdgeOtherNode(edge, &n->node);
            o    = fer_container_of(node, fer_gngp_node_t, node);

            // if o is already in same set as m we don't need to check it
            if (o->set == m->set)
                continue;

            // evaluate o, before evaluation we know that o belongs to
            // other set than m
            if (o->evaled != gng->cycle){
                set = gng->ops.eval(o->w, gng->ops.eval_data);
                nodeChangeSet(gng, o, set);
                o->evaled = gng->cycle;

                if (o->set == m->set){
                    // if o belongs to same set as m add it into fifo (i.e. set
                    // of o was changed)
                    ferListAppend(&fifo, &o->fifo);
                }
            }

            if (o->set != m->set && o->set != FER_GNGP_NONE){
                // if set doesn't belong to same set as m as is already
                // assigne to some set (other than NONE), disconnect it
                // from m's subnet
                e = fer_container_of(edge, fer_gngp_edge_t, edge);
                edgeDel(gng, e);

                //DBG("edgesLen(o) %lx", (long)o);
                if (ferNetNodeEdgesLen(&o->node) == 0){
                    nodeDel(gng, o);
                }
            }
        }


        //DBG("  rank(n): %d", ferNetNodeEdgesLen(&n->node));
        //DBG("edgesLen(n) %lx", (long)n);
        if (ferNetNodeEdgesLen(&n->node) == 0 && n != m){
            nodeDel(gng, n);
        }
    }
}



/*** Find path ***/
static void findPathExpand(fer_dij_node_t *_n, fer_list_t *expand, void *data)
{
    fer_gngp_t *gng = (fer_gngp_t *)data;
    fer_list_t *list, *item;
    fer_gngp_node_t *n, *o;
    fer_net_edge_t *edge;
    fer_net_node_t *node;
    fer_real_t dist;

    n = fer_container_of(_n, fer_gngp_node_t, dij);

    list = ferNetNodeEdges(&n->node);
    FER_LIST_FOR_EACH(list, item){
        edge = ferNetEdgeFromNodeList(item);
        node = ferNetEdgeOtherNode(edge, &n->node);
        o    = fer_container_of(node, fer_gngp_node_t, node);

        if (!ferDijNodeClosed(&o->dij) && o->set == n->set){
            dist = ferVecDist(gng->params.d, n->w, o->w);
            ferDijNodeAdd(&o->dij, expand, dist);
        }
    }
}

/** Initializes all nodes in net for dijkstra search */
static void findPathDijInit(fer_gngp_t *gng)
{
    fer_list_t *list, *item;
    fer_net_node_t *node;
    fer_gngp_node_t *n;

    list = ferNetNodes(gng->net);
    FER_LIST_FOR_EACH(list, item){
        node = FER_LIST_ENTRY(item, fer_net_node_t, list);
        n    = fer_container_of(node, fer_gngp_node_t, node);
        ferDijNodeInit(&n->dij);
    }
}

/** Fills given list by path from s to g.
 *  It is assumed that the path exists! */
static void obtainPath(fer_gngp_node_t *s, fer_gngp_node_t *g,
                       fer_list_t *list)
{
    fer_gngp_node_t *n;
    fer_dij_node_t *dn;

    ferListPrepend(list, &g->path);
    dn = g->dij.prev;
    while (dn != &s->dij){
        n = fer_container_of(dn, fer_gngp_node_t, dij);
        ferListPrepend(list, &n->path);

        dn = dn->prev;
    }
}

static int errHeapLT(const fer_pairheap_node_t *_n1,
                     const fer_pairheap_node_t *_n2,
                     void *data)
{
    fer_gngp_t *gng = (fer_gngp_t *)data;
    fer_gngp_node_t *n1, *n2;

    n1 = fer_container_of(_n1, fer_gngp_node_t, err_heap);
    n2 = fer_container_of(_n2, fer_gngp_node_t, err_heap);

    nodeFixError(gng, n1);
    nodeFixError(gng, n2);
    return n1->err > n2->err;
}


int ferGNGPFindPath(fer_gngp_t *gng,
                    const fer_vec_t *wstart, const fer_vec_t *wgoal,
                    fer_list_t *list)
{
    fer_dij_ops_t ops;
    fer_dij_t *dij;
    fer_gngp_node_t *start, *goal;
    int result;

    // create start and goal nodes
    start = connectNewNode(gng, wstart);
    goal  = connectNewNode(gng, wgoal);

    // initialize whole net
    findPathDijInit(gng);

    // initialize operations
    ferDijOpsInit(&ops);
    ops.expand = findPathExpand;
    ops.data   = (void *)gng;

    // create dijkstra algorithm
    dij = ferDijNew(&ops);

    // run dijkstra
    result = ferDijRun(dij, &start->dij, &goal->dij);

    if (result == 0){
        obtainPath(start, goal, list);
        ferDijDel(dij);
        return 0;
    }

    ferDijDel(dij);

    // remove previously created nodes
    nodeDelWithEdges(gng, start);
    nodeDelWithEdges(gng, goal);

    return -1;
}


static fer_gngp_node_t *connectNewNode(fer_gngp_t *gng, const fer_vec_t *w)
{
    fer_gngp_node_t *n;
    fer_gngp_node_t *n1 = NULL, *n2 = NULL;

    nearest(gng, w, &n1, &n2);
    n = nodeNew(gng, w);
    nodeChangeSet(gng, n, FER_GNGP_FREE);
    edgeNew(gng, n, n1);
    edgeNew(gng, n, n2);

    return n;
}



/*** Node functions ***/
static fer_gngp_node_t *nodeNew(fer_gngp_t *gng, const fer_vec_t *w)
{
    fer_gngp_node_t *n;

    n = FER_ALLOC(fer_gngp_node_t);

    n->set = -1;
    nodeChangeSet(gng, n, FER_GNGP_NONE);
    n->evaled = 0;

    n->w = ferVecClone(gng->params.d, w);

    ferGUGElInit(&n->gug, n->w);
    ferGUGAdd(gng->gug, &n->gug);

    n->err = FER_ZERO;
    n->err_cycle = gng->cycle;
    ferPairHeapAdd(gng->err_heap, &n->err_heap);

    ferNetAddNode(gng->net, &n->node);

    return n;
}

static void nodeDel(fer_gngp_t *gng, fer_gngp_node_t *n)
{
    ferVecDel(n->w);

    ferGUGRemove(gng->gug, &n->gug);

    ferPairHeapRemove(gng->err_heap, &n->err_heap);

    if (ferNetRemoveNode(gng->net, &n->node) != 0){
        DBG2("Can't remove node! You called this prematurely!");
        return;
    }

    free(n);
}

static void nodeDelWithEdges(fer_gngp_t *gng, fer_gngp_node_t *n)
{
    fer_list_t *list, *item, *item_tmp;
    fer_net_edge_t *edge;
    fer_gngp_edge_t *e;

    list = ferNetNodeEdges(&n->node);
    FER_LIST_FOR_EACH_SAFE(list, item, item_tmp){
        edge = ferNetEdgeFromNodeList(item);
        e    = fer_container_of(edge, fer_gngp_edge_t, edge);
        edgeDel(gng, e);
    }
    nodeDel(gng, n);
}

static void nodeChangeSet(fer_gngp_t *gng, fer_gngp_node_t *n, int set)
{
    if (n->set != set){
        if (n->set >= 0)
            gng->set_size[n->set]--;
        n->set = set;
        gng->set_size[n->set]++;
    }
}

static void nodeMoveTowards(fer_gngp_t *gng, fer_gngp_node_t *n,
                            const fer_vec_t *to, fer_real_t frac,
                            int incerr)
{
    fer_vec_t *move;
    fer_real_t err;

    move = ferVecNew(gng->params.d);

    ferVecSub2(gng->params.d, move, to, n->w);
    ferVecScale(gng->params.d, move, frac);
    ferVecAdd(gng->params.d, n->w, move);
    ferGUGUpdate(gng->gug, &n->gug);

    // increase error counter
    if (incerr){
        err  = ferVecDist2(gng->params.d, n->w, to);
        err *= gng->beta_n[gng->params.lambda - gng->step];
        nodeIncError(gng, n, err);
    }

    ferVecDel(move);
}

_fer_inline void nodeFixError(fer_gngp_t *gng, fer_gngp_node_t *n)
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

_fer_inline void nodeIncError(fer_gngp_t *gng, fer_gngp_node_t *n,
                              fer_real_t inc)
{
    nodeFixError(gng, n);
    n->err += inc;
    ferPairHeapUpdate(gng->err_heap, &n->err_heap);
}

_fer_inline void nodeScaleError(fer_gngp_t *gng, fer_gngp_node_t *n,
                                fer_real_t scale)
{
    nodeFixError(gng, n);
    n->err *= scale;
    ferPairHeapUpdate(gng->err_heap, &n->err_heap);
}

static void netNodeDel(fer_net_node_t *_n, void *_)
{
    fer_gngp_node_t *n;
    n = fer_container_of(_n, fer_gngp_node_t, node);
    ferVecDel(n->w);
    free(n);
}



/*** Edge functions ***/
static fer_gngp_edge_t *edgeNew(fer_gngp_t *gng, fer_gngp_node_t *n1,
                                                 fer_gngp_node_t *n2)
{
    fer_gngp_edge_t *e;

    e = FER_ALLOC(fer_gngp_edge_t);
    e->age = 0;

    ferNetAddEdge(gng->net, &e->edge, &n1->node, &n2->node);
    return e;
}

static void edgeDel(fer_gngp_t *gng, fer_gngp_edge_t *e)
{
    ferNetRemoveEdge(gng->net, &e->edge);
    free(e);
}

static void netEdgeDel(fer_net_edge_t *_n, void *_)
{
    fer_gngp_edge_t *n;
    n = fer_container_of(_n, fer_gngp_edge_t, edge);
    free(n);
}
