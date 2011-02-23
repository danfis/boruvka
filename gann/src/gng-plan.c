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
#include <gann/gng-plan.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Initializes algorithm */
static void init(gann_gngp_t *gng);
/** Adapts net to input signal number of step (1, ..., lambda) must be
 *  provided */
static void adapt(gann_gngp_t *gng, size_t step);
/** Adds new node into net */
static void newNode(gann_gngp_t *gng);
/** Finds two nearest nodes to given input signal */
static int nearest(gann_gngp_t *gng, const fer_vec2_t *w,
                   gann_gngp_node_t **n1, gann_gngp_node_t **n2);
/** Learns the part of net around given node. It will adapt it to given
 *  input signal. Also edges with age > age_max are removed along with
 *  abandonded nodes. */
static void learn(gann_gngp_t *gng, size_t step,
                  gann_gngp_node_t *n, const fer_vec2_t *is);
/** Returns node with highest error */
static gann_gngp_node_t *nodeWithMaxErr(gann_gngp_t *gng);
/** Returns node's neighbor with highest error */
static gann_gngp_node_t *nodeNeighborWithMaxErr(gann_gngp_node_t *n);
/** Cuts subnet containing given node */
static void cutSubnet(gann_gngp_t *gng, gann_gngp_node_t *m);

/*** Node functions ***/
static gann_gngp_node_t *nodeNew(gann_gngp_t *gng, const fer_vec2_t *w);
static void nodeDel(gann_gngp_t *gng, gann_gngp_node_t *n);
static void nodeChangeSet(gann_gngp_t *gng, gann_gngp_node_t *n, int set);
static void nodeMoveTowards(gann_gngp_t *gng, gann_gngp_node_t *n,
                            const fer_vec2_t *to, fer_real_t frac);
static void netNodeDel(gann_net_node_t *n, void *);

/*** Edge functions ***/
static gann_gngp_edge_t *edgeNew(gann_gngp_t *gng, gann_gngp_node_t *n1,
                                                   gann_gngp_node_t *n2);
static void edgeDel(gann_gngp_t *gng, gann_gngp_edge_t *e);
static void netEdgeDel(gann_net_edge_t *n, void *);


void gannGNGPOpsInit(gann_gngp_ops_t *ops)
{
    memset(ops, 0, sizeof(gann_gngp_ops_t));
}

void gannGNGPParamsInit(gann_gngp_params_t *params)
{
    params->lambda  = 200;
    params->eb      = 0.05;
    params->en      = 0.0006;
    params->alpha   = 0.95;
    params->beta    = 0.9995;
    params->age_max = 200;

    params->cut_subnet_nodes = 5000;
    params->num_cubes = 10000;
    params->aabb[0] = -FER_ONE;
    params->aabb[1] =  FER_ONE;
    params->aabb[2] = -FER_ONE;
    params->aabb[3] =  FER_ONE;
}

gann_gngp_t *gannGNGPNew(const gann_gngp_ops_t *ops,
                         const gann_gngp_params_t *params)
{
    gann_gngp_t *gng;

    gng = FER_ALLOC(gann_gngp_t);

    gng->net = gannNetNew();
    gng->cubes = ferCubes2New(params->aabb, params->num_cubes);
    gng->params = *params;
    gng->beta_n = NULL;

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

    return gng;
}

void gannGNGPDel(gann_gngp_t *gng)
{
    if (gng->cubes)
        ferCubes2Del(gng->cubes);

    if (gng->net){
        gannNetDel2(gng->net, netNodeDel, NULL,
                              netEdgeDel, NULL);
    }

    if (gng->beta_n)
        free(gng->beta_n);

    free(gng);
}

void gannGNGPRun(gann_gngp_t *gng)
{
    size_t step, cycle;

    cycle = 0;
    init(gng);
    do {
        for (step = 1; step <= gng->params.lambda; step++){
            adapt(gng, step);
        }
        newNode(gng);

        cycle++;
        if (gng->ops.callback && gng->ops.callback_period == cycle){
            gng->ops.callback(gng->ops.callback_data);
            cycle = 0;
        }
    } while (!gng->ops.terminate(gng->ops.terminate_data));
}

static void dumpSet(gann_gngp_t *gng, int set, FILE *out, const char *name,
                    const char *setname, const char *color)
{
    fer_list_t *list, *item;
    gann_net_node_t *nn;
    gann_gngp_node_t *n;
    gann_net_edge_t *e;
    size_t i, id1, id2;

    fprintf(out, "--------\n");
    fprintf(out, "Edge color: %s\n", color);

    if (name){
        fprintf(out, "Name: %s (%s)\n", name, setname);
    }else{
        fprintf(out, "Name: (%s)\n", setname);
    }

    fprintf(out, "Points:\n");
    list = gannNetNodes(gng->net);
    i = 0;
    ferListForEach(list, item){
        nn = ferListEntry(item, gann_net_node_t, list);
        n  = fer_container_of(nn, gann_gngp_node_t, node);
        if (n->set != set)
            continue;

        n->_id = i++;
        ferVec2Print(&n->w, out);
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = gannNetEdges(gng->net);
    ferListForEach(list, item){
        e = ferListEntry(item, gann_net_edge_t, list);

        nn = gannNetEdgeNode(e, 0);
        n  = fer_container_of(nn, gann_gngp_node_t, node);
        id1 = n->_id;
        if (n->set != set)
            continue;

        nn = gannNetEdgeNode(e, 1);
        n  = fer_container_of(nn, gann_gngp_node_t, node);
        id2 = n->_id;
        if (n->set != set)
            continue;

        fprintf(out, "%d %d\n", id1, id2);
    }

    fprintf(out, "--------\n");
}

void gannGNGPDumpSVT(gann_gngp_t *gng, FILE *out, const char *name)
{
    if (gng->set_size[GANN_GNGP_NONE] > 0){
        dumpSet(gng, GANN_GNGP_NONE, out, name, "none", "0.7 0.7 0.7");
    }else{
        fprintf(out, "Points:\n0 0 0\nPoint size: 0\n---\n");
    }

    if (gng->set_size[GANN_GNGP_FREE] > 0){
        dumpSet(gng, GANN_GNGP_FREE, out, name, "free", "0 0 0.8");
    }else{
        fprintf(out, "Points:\n0 0 0\nPoint size: 0\n---\n");
    }

    if (gng->set_size[GANN_GNGP_OBST] > 0){
        dumpSet(gng, GANN_GNGP_OBST, out, name, "obst", "0.8 0 0");
    }else{
        fprintf(out, "Points:\n0 0 0\nPoint size: 0\n---\n");
    }
}


static void init(gann_gngp_t *gng)
{
    const fer_vec2_t *is;
    size_t i;

    // precompute beta^n
    if (gng->beta_n)
        free(gng->beta_n);

    gng->beta_n = FER_ALLOC_ARR(fer_real_t, gng->params.lambda);
    gng->beta_n[0] = gng->params.beta;
    for (i = 1; i < gng->params.lambda; i++){
        gng->beta_n[i] = gng->beta_n[i - 1] * gng->params.beta;
    }

    // create two initial nodes
    is = gng->ops.input_signal(gng->ops.input_signal_data);
    nodeNew(gng, is);
    is = gng->ops.input_signal(gng->ops.input_signal_data);
    nodeNew(gng, is);
}

static void adapt(gann_gngp_t *gng, size_t step)
{
    const fer_vec2_t *is;
    gann_gngp_node_t *n1, *n2;
    gann_gngp_edge_t *e;
    gann_net_edge_t *edge;
    int set;

    // 1. Get random input signal
    is = gng->ops.input_signal(gng->ops.input_signal_data);

    // 2. Find two nearest nodes (n1, n2)
    nearest(gng, is, &n1, &n2);

    // 3. Learn net according to set of n1 and n2
    if (n1->set == n2->set){
        // n1 and n2 both belong to same set (free or obstacle)

        // 3.1. Create edge n1-n2 if doesn't exist
        edge = gannNetNodeCommonEdge(&n1->node, &n2->node);
        if (!edge){
            e = edgeNew(gng, n1, n2);
        }else{
            e = fer_container_of(edge, gann_gngp_edge_t, edge);
        }

        // 3.2. Set age of edge to zero
        e->age = 0;

        // 3.3. Learn net around n1
        learn(gng, step, n1, is);

    }else{
        set = gng->ops.eval(is, gng->ops.eval_data);

        if (set == n1->set){
            learn(gng, step, n1, is);
        }else{
            learn(gng, step, n2, is);
        }
    }
}

static void newNode(gann_gngp_t *gng)
{
    gann_gngp_node_t *n1, *n2, *m;
    gann_net_edge_t *edge;
    gann_gngp_edge_t *e;
    fer_vec2_t w;
    int set;

    // 1. Get node with highest error and its neighbor with highest error
    n1 = nodeWithMaxErr(gng);
    n2 = nodeNeighborWithMaxErr(n1);
    if (!n1 || !n2){
        DBG("%d", gannNetNodeEdgesLen(&n1->node));
        DBG("%lx %lx", (long)n1, (long)n2);
    }

    // 2. Create new node between n1 and n2
    ferVec2Add2(&w, &n1->w, &n2->w);
    ferVec2Scale(&w, FER_REAL(0.5));
    m = nodeNew(gng, &w);

    // 3. Create edges m-n1 and m-n2 and remove edge n1-n2
    edgeNew(gng, m, n1);
    edgeNew(gng, m, n2);
    edge = gannNetNodeCommonEdge(&n1->node, &n2->node);
    e    = fer_container_of(edge, gann_gngp_edge_t, edge);
    edgeDel(gng, e);

    // 4. Decrease error of and n1 and n2 and set up error of m as average
    // of n1 and n2
    n1->err *= gng->params.alpha;
    n2->err *= gng->params.alpha;
    m->err  = n1->err + n2->err;
    m->err /= FER_REAL(2.);

    if (gannNetNodesLen(gng->net) > gng->params.cut_subnet_nodes){
        // 5. Evaluate new node and set up its set properly
        set = gng->ops.eval(&m->w, gng->ops.eval_data);
        nodeChangeSet(gng, m, set);

        if (n1->set != m->set && n2->set != m->set
                && n1->set == n2->set
                && n1->set != GANN_GNGP_NONE){
            fprintf(stdout, "%g %g\n", ferVec2X(&m->w), ferVec2Y(&m->w));
        }

        // 6. Cut m's subnet if necessary
        cutSubnet(gng, m);
    }
}

static int nearest(gann_gngp_t *gng, const fer_vec2_t *w,
                   gann_gngp_node_t **n1, gann_gngp_node_t **n2)
{
    fer_cubes2_el_t *els[2];
    gann_gngp_node_t *n;
    size_t found;

    els[0] = els[1] = NULL;
    found = ferCubes2Nearest(gng->cubes, w, 2, els);
    if (found != 2){
        DBG2("Not found two nearest nodes! This shouldn't happen!");
        return -1;
    }

    n = fer_container_of(els[0], gann_gngp_node_t, cubes);
    *n1 = n;
    n = fer_container_of(els[1], gann_gngp_node_t, cubes);
    *n2 = n;

    return 0;
}

static void learn(gann_gngp_t *gng, size_t step,
                  gann_gngp_node_t *n, const fer_vec2_t *is)
{
    fer_list_t *list, *item, *item_tmp;
    gann_net_node_t *other;
    gann_gngp_node_t *o;
    gann_net_edge_t *edge;
    gann_gngp_edge_t *e;
    fer_real_t err;


    // increase age of all outgoing edges from n, move all neighbor nodes
    // of n towards input signal and remove all edges with age > age_max
    list = gannNetNodeEdges(&n->node);
    ferListForEachSafe(list, item, item_tmp){
        edge  = gannNetEdgeFromNodeList(item);
        e     = fer_container_of(edge, gann_gngp_edge_t, edge);
        other = gannNetEdgeOtherNode(edge, &n->node);
        o     = fer_container_of(other, gann_gngp_node_t, node);

        // increase age
        e->age += 1;

        // remove edge if its age is above age_max, otherwise move node o
        // towards input signal
        if (e->age > gng->params.age_max){
            edgeDel(gng, e);

            // remove also o if not connected into net
            if (gannNetNodeEdgesLen(other) == 0){
                nodeDel(gng, o);
            }
        }else{
            // move o towards input signal
            nodeMoveTowards(gng, o, is, gng->params.en);
        }
    }

    if (gannNetNodeEdgesLen(&n->node) == 0){
        // remove node if it's not connected into net anymore
        nodeDel(gng, n);
    }else{
        // move node towards input signal
        nodeMoveTowards(gng, n, is, gng->params.eb);

        // update error counter of node n
        err = ferVec2Dist2(&n->w, is);
        n->err_local += err * gng->beta_n[gng->params.lambda - step];
    }
}

static gann_gngp_node_t *nodeWithMaxErr(gann_gngp_t *gng)
{
    fer_list_t *list, *item;
    gann_net_node_t *node;
    gann_gngp_node_t *n;
    gann_gngp_node_t *max_n;
    fer_real_t max_err;

    max_err = -FER_ONE;
    max_n   = NULL;

    list = gannNetNodes(gng->net);
    ferListForEach(list, item){
        node = ferListEntry(item, gann_net_node_t, list);
        n    = fer_container_of(node, gann_gngp_node_t, node);

        // fix error
        n->err  = n->err * gng->beta_n[gng->params.lambda - 1];
        n->err += n->err_local;
        n->err_local = FER_ZERO;

        // reset .evaled mark - we need this because of cutSubnet()
        n->evaled = 0;

        if (n->err > max_err){
            max_err = n->err;
            max_n   = n;
        }
    }

    return max_n;
}

static gann_gngp_node_t *nodeNeighborWithMaxErr(gann_gngp_node_t *n)
{
    fer_list_t *list, *item;
    gann_net_node_t *other;
    gann_gngp_node_t *o;
    gann_net_edge_t *edge;
    gann_gngp_node_t *max_n;
    fer_real_t max_err;

    max_err = -FER_ONE;
    max_n   = NULL;

    list = gannNetNodeEdges(&n->node);
    ferListForEach(list, item){
        edge  = gannNetEdgeFromNodeList(item);
        other = gannNetEdgeOtherNode(edge, &n->node);
        o     = fer_container_of(other, gann_gngp_node_t, node);

        if (o->err > max_err){
            max_err = o->err;
            max_n   = o;
        }
    }

    return max_n;
}

static void cutSubnet(gann_gngp_t *gng, gann_gngp_node_t *m)
{
    fer_list_t fifo;
    fer_list_t *list, *item, *item_tmp;
    gann_net_edge_t *edge;
    gann_net_node_t *node;
    gann_gngp_node_t *n, *o;
    gann_gngp_edge_t *e;
    int set;

    // 1. Initialize FIFO queue
    ferListInit(&fifo);

    // 2. Add m into fifo
    ferListAppend(&fifo, &m->fifo);
    m->evaled = 1;

    //DBG("rank(m): %d", gannNetNodeEdgesLen(&m->node));

    while (!ferListEmpty(&fifo)){
        // Pop next item form fifo
        item = ferListNext(&fifo);
        ferListDel(item);
        n = ferListEntry(item, gann_gngp_node_t, fifo);

        //DBG("  rank(n): %d", gannNetNodeEdgesLen(&n->node));
        // Iterate over n's neighbors that are _not_ in same set as m
        list = gannNetNodeEdges(&n->node);
        ferListForEachSafe(list, item, item_tmp){
            edge = gannNetEdgeFromNodeList(item);
            node = gannNetEdgeOtherNode(edge, &n->node);
            o    = fer_container_of(node, gann_gngp_node_t, node);

            // if o is already in same set as m we don't need to check it
            if (o->set == m->set)
                continue;

            // evaluate o, before evaluation we know that o belongs to
            // other set than m
            if (!o->evaled){
                set = gng->ops.eval(&o->w, gng->ops.eval_data);
                nodeChangeSet(gng, o, set);

                if (o->set == m->set){
                    // if o belongs to same set as m add it into fifo (i.e. set
                    // of o was changed)
                    ferListAppend(&fifo, &o->fifo);
                }
            }

            if (o->set != m->set){
                // if set doesn't belong to same set as m, disconnect it
                // from m's subnet
                e = fer_container_of(edge, gann_gngp_edge_t, edge);
                edgeDel(gng, e);

                //DBG("edgesLen(o) %lx", (long)o);
                if (gannNetNodeEdgesLen(&o->node) == 0){
                    nodeDel(gng, o);
                }
            }
            /*

            // if already evaluated in this cycle...
            if (o->evaled){
                // if o doesn't belong to same set as m, disconnect it from
                // n (because we know that n belongs to same set)
                if (o->set != m->set){
                    e = fer_container_of(edge, gann_gngp_edge_t, edge);
                    //DBG("    rank(o): %d", gannNetNodeEdgesLen(&o->node));
                    edgeDel(gng, e);
                    //DBG("    rank(o): %d", gannNetNodeEdgesLen(&o->node));

                    if (gannNetNodeEdgesLen(&o->node) == 0){
                        nodeDel(gng, o);
                    }
                }

                // skip to next node
                continue;
            }

            // evaluate node and record the cycle
            o->set = gng->ops.eval(&o->w, gng->ops.eval_data);
            o->evaled = 1;

            if (o->set == m->set){
                // if o belongs to same set as m add it into fifo
                ferListAppend(&fifo, &o->fifo);
            }else{
                // if set doesn't belong to same set as m, disconnect it
                // from m's subnet
                e = fer_container_of(edge, gann_gngp_edge_t, edge);
                edgeDel(gng, e);

                if (gannNetNodeEdgesLen(&o->node) == 0){
                    nodeDel(gng, o);
                }
            }
            */
        }


        //DBG("  rank(n): %d", gannNetNodeEdgesLen(&n->node));
        //DBG("edgesLen(n) %lx", (long)n);
        if (gannNetNodeEdgesLen(&n->node) == 0){
            /*
            if (n == m){
                DBG("Cutting new node %d", m->set);
            }
            */
            nodeDel(gng, n);
        }
    }
}


/*** Node functions ***/
static gann_gngp_node_t *nodeNew(gann_gngp_t *gng, const fer_vec2_t *w)
{
    gann_gngp_node_t *n;

    n = FER_ALLOC(gann_gngp_node_t);

    n->set = -1;
    nodeChangeSet(gng, n, GANN_GNGP_NONE);
    n->evaled = 0;

    ferVec2Copy(&n->w, w);

    ferCubes2ElInit(&n->cubes, &n->w);
    ferCubes2Add(gng->cubes, &n->cubes);

    n->err_local = FER_ZERO;
    n->err       = FER_ZERO;

    gannNetAddNode(gng->net, &n->node);

    return n;
}

static void nodeDel(gann_gngp_t *gng, gann_gngp_node_t *n)
{
    ferCubes2Remove(gng->cubes, &n->cubes);

    if (gannNetRemoveNode(gng->net, &n->node) != 0){
        DBG2("Can't remove node! You called this prematurely!");
        return;
    }

    free(n);
}

static void nodeChangeSet(gann_gngp_t *gng, gann_gngp_node_t *n, int set)
{
    if (n->set != set){
        if (n->set >= 0)
            gng->set_size[n->set]--;
        n->set = set;
        gng->set_size[n->set]++;
        n->evaled = 1;
    }
}

static void nodeMoveTowards(gann_gngp_t *gng, gann_gngp_node_t *n,
                            const fer_vec2_t *to, fer_real_t frac)
{
    fer_vec2_t move;

    ferVec2Sub2(&move, to, &n->w);
    ferVec2Scale(&move, frac);
    ferVec2Add(&n->w, &move);
    ferCubes2Update(gng->cubes, &n->cubes);
}

static void netNodeDel(gann_net_node_t *_n, void *_)
{
    gann_gngp_node_t *n;
    n = fer_container_of(_n, gann_gngp_node_t, node);
    free(n);
}



/*** Edge functions ***/
static gann_gngp_edge_t *edgeNew(gann_gngp_t *gng, gann_gngp_node_t *n1,
                                                   gann_gngp_node_t *n2)
{
    gann_gngp_edge_t *e;

    e = FER_ALLOC(gann_gngp_edge_t);
    e->age = 0;

    gannNetAddEdge(gng->net, &e->edge, &n1->node, &n2->node);
    return e;
}

static void edgeDel(gann_gngp_t *gng, gann_gngp_edge_t *e)
{
    gannNetRemoveEdge(gng->net, &e->edge);
    free(e);
}

static void netEdgeDel(gann_net_edge_t *_n, void *_)
{
    gann_gngp_edge_t *n;
    n = fer_container_of(_n, gann_gngp_edge_t, edge);
    free(n);
}
