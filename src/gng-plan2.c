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

#include <fermat/gng-plan2.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>
#include <fermat/vec2.h>
#include <fermat/vec3.h>

/** Cut obstacle nodes from path */
static int ferGNGPlanCutPath(fer_gng_plan_t *gng, fer_list_t *path);
/** Returns true if edge is whole in FREE space */
static int ferGNGPlanIsEdgeFree(fer_gng_plan_t *gng,
                                const fer_vec_t *from, const fer_vec_t *to);
/** Returns true if whole path is in FREE space */
static int ferGNGPlanIsPathFree(fer_gng_plan_t *gng, fer_list_t *path);

/** Fix given node */
static void ferGNGPlanFixNode(fer_gng_plan_t *gng, fer_gng_plan_node_t *n);

static const void *ferGNGPlanInputSignal(void *);
static void ferGNGPlanInit(fer_gng_node_t **n1, fer_gng_node_t **n2, void *);
static fer_gng_node_t *ferGNGPlanNewNode(const void *input_signal, void *);
static fer_gng_node_t *ferGNGPlanNewNodeBetween(const fer_gng_node_t *n1,
                                                const fer_gng_node_t *n2,
                                                void *);
static void ferGNGPlanDelNode(fer_gng_node_t *n, void *);
static void ferGNGPlanNearest(const void *input_signal,
                              fer_gng_node_t **n1, fer_gng_node_t **n2,
                              void *);
static fer_real_t ferGNGPlanDist2(const void *input_signal,
                                  const fer_gng_node_t *node, void *);
static void ferGNGPlanMoveTowards(fer_gng_node_t *node,
                                  const void *input_signal,
                                  fer_real_t fraction, void *);

/*** Find path ***/
static void ferGNGPlanFindPathExpand(fer_dij_node_t *_n,
                                     fer_list_t *expand, void *_);
/** Initializes all nodes in net for dijkstra search */
static void ferGNGPlanFindPathDijInit(fer_gng_plan_t *gng);
/** Fills given list by path from s to g.
 *  It is assumed that the path exists! */
static void ferGNGPlanObtainPath(fer_gng_plan_node_t *s, fer_gng_plan_node_t *g,
                                 fer_list_t *list);
/** Finds path from {wstart} to {wgoal} */
static int ferGNGPlanFindPath(fer_gng_plan_t *gng,
                              const fer_vec_t *wstart,
                              const fer_vec_t *wgoal,
                              fer_list_t *list);
static int ferGNGPlanTryFindPath(fer_gng_plan_t *gng);

void ferGNGPlanOpsInit(fer_gng_plan_ops_t *ops)
{
    memset(ops, 0, sizeof(*ops));
}


void ferGNGPlanParamsInit(fer_gng_plan_params_t *params)
{
    params->dim           = 2;
    params->max_dist      = FER_REAL(0.1);
    params->max_dist      = FER_REAL(1.);
    params->min_nodes     = 100;

    params->start = NULL;
    params->goal  = NULL;

    ferGNGParamsInit(&params->gng);
    ferGUGParamsInit(&params->gug);

    params->gug.dim = 2;
}


fer_gng_plan_t *ferGNGPlanNew(const fer_gng_plan_ops_t *ops,
                              const fer_gng_plan_params_t *params)
{
    fer_gng_plan_t *gng;
    fer_gng_ops_t gng_ops;
    fer_gug_params_t cells_params;

    gng = FER_ALLOC(fer_gng_plan_t);
    gng->dim           = params->dim;
    gng->max_dist      = params->max_dist;
    gng->min_dist      = params->min_dist;
    gng->min_nodes     = params->min_nodes;

    gng->evals = 0L;


    // initialize GNG operations
    ferGNGOpsInit(&gng_ops);
    gng_ops.callback          = ops->callback;
    gng_ops.callback_period   = ops->callback_period;
    gng_ops.callback_data     = ops->callback_data;
    if (!gng_ops.callback_data)
        gng_ops.callback_data = ops->data;
    gng_ops.data = gng;
    //gng_ops.terminate        = ferGNGPlanTerminate;
    gng_ops.input_signal     = ferGNGPlanInputSignal;
    gng_ops.init             = ferGNGPlanInit;
    gng_ops.new_node         = ferGNGPlanNewNode;
    gng_ops.new_node_between = ferGNGPlanNewNodeBetween;
    gng_ops.del_node         = ferGNGPlanDelNode;
    gng_ops.nearest          = ferGNGPlanNearest;
    gng_ops.dist2            = ferGNGPlanDist2;
    gng_ops.move_towards     = ferGNGPlanMoveTowards;

    // initialize GUG params
    cells_params = params->gug;
    cells_params.dim = params->dim;

    // create GNG object
    gng->gng = ferGNGNew(&gng_ops, &params->gng);

    // create GUG for FREE nodes
    gng->gug = ferGUGNew(&cells_params);

    // init list of obstacle nodes
    ferListInit(&gng->obst);

    // create GUG for OBST nodes
    gng->obst_gug = ferGUGNew(&cells_params);

    // initialize GNG-Plan operations
    gng->ops = *ops;
    if (!gng->ops.eval_data)
        gng->ops.eval_data = ops->eval_data;
    if (!gng->ops.terminate_data)
        gng->ops.terminate_data = ops->terminate_data;
    if (!gng->ops.input_signal_data)
        gng->ops.input_signal_data = ops->input_signal_data;

    ferListInit(&gng->path);
    gng->start = ferVecClone(gng->dim, params->start);
    gng->goal = ferVecClone(gng->dim, params->goal);

    gng->tmpv  = ferVecNew(gng->dim);
    gng->tmpv2 = ferVecNew(gng->dim);

    return gng;
}

void ferGNGPlanDel(fer_gng_plan_t *gng)
{
    fer_list_t *item;
    fer_gng_plan_node_t *n;

    ferGNGDel(gng->gng);
    ferGUGDel(gng->gug);

    ferGUGDel(gng->obst_gug);

    while (!ferListEmpty(&gng->obst)){
        item = ferListNext(&gng->obst);
        ferListDel(item);
        n    = FER_LIST_ENTRY(item, fer_gng_plan_node_t, obst);
        ferVecDel(n->w);
        FER_FREE(n);
    }

    ferVecDel(gng->start);
    ferVecDel(gng->goal);

    ferVecDel(gng->tmpv);
    ferVecDel(gng->tmpv2);

    FER_FREE(gng);
}

void ferGNGPlanRun(fer_gng_plan_t *gng)
{
    unsigned long cycle;
    size_t i;
    int path;

    cycle = 0;
    ferGNGInit(gng->gng);

    do {
        for (i = 0; i < gng->gng->params.lambda; i++){
            ferGNGLearn(gng->gng);
        }
        path = ferGNGPlanTryFindPath(gng);

        if (path == -1)
            ferGNGNewNode(gng->gng);

        cycle++;
        if (gng->ops.callback && gng->ops.callback_period == cycle){
            gng->ops.callback(gng->ops.callback_data);
            cycle = 0L;
        }
    } while (path != 1 && !gng->ops.terminate(gng->ops.terminate_data));
}

fer_real_t ferGNGPlanAvgEdgeLen(fer_gng_plan_t *gng)
{
    fer_real_t avg;
    int num;
    fer_list_t *list, *item;
    fer_gng_edge_t *e;
    fer_gng_node_t *nn[2];
    fer_gng_plan_node_t *n[2];

    num = 0;
    avg = FER_ZERO;

    list = ferGNGEdges(gng->gng);
    FER_LIST_FOR_EACH(list, item){
        e = ferGNGEdgeFromList(item);

        // get start and end node
        ferGNGEdgeNodes(e, nn + 0, nn + 1);
        n[0] = fer_container_of(nn[0], fer_gng_plan_node_t, node);
        n[1] = fer_container_of(nn[1], fer_gng_plan_node_t, node);

        if (n[0]->fixed || n[1]->fixed)
            continue;

        avg += ferVecDist(gng->dim, n[0]->w, n[1]->w);
        num += 1;
    }

    return avg / (fer_real_t)num;
}


static int ferGNGPlanCutPath(fer_gng_plan_t *gng, fer_list_t *path)
{
    fer_list_t *item, *tmpitem;
    fer_gng_plan_node_t *n;
    int eval, cut = -1;

    // check each node in path if it is in free space
    FER_LIST_FOR_EACH_SAFE(path, item, tmpitem){
        n = FER_LIST_ENTRY(item, fer_gng_plan_node_t, path);

        // skip fixed nodes - we know these are in FREE space
        if (n->fixed)
            continue;

        eval = gng->ops.eval(n->w, gng->ops.eval_data);
        gng->evals += 1L;

        // node is in obstacle
        if (eval == FER_GNG_PLAN_OBST){
            // remove node from GUG
            ferGUGRemove(gng->gug, &n->gug);

            // remove node from GNG
            ferGNGNodeRemove(gng->gng, &n->node);

            // add node to obst list
            ferListAppend(&gng->obst, &n->obst);
            ferGUGAdd(gng->obst_gug, &n->gug);

            cut = 0;
            return 0;
        }else{
            // node node as fixed because this is in FREE space
            ferGNGPlanFixNode(gng, n);
        }
    }

    return cut;
}

static int ferGNGPlanIsEdgeFree(fer_gng_plan_t *gng,
                                const fer_vec_t *from, const fer_vec_t *to)
{
    fer_gng_plan_node_t *n;
    fer_gng_node_t *nn;
    fer_real_t dist;
    int eval;

    dist = ferVecDist(gng->dim, from, to);
    if (dist > gng->max_dist){
        // store start node in .tmpv
        ferVecCopy(gng->dim, gng->tmpv, from);

        // store step vector in .tmpv2
        ferVecSub2(gng->dim, gng->tmpv2, to, from);
        ferVecScale(gng->dim, gng->tmpv2,
                    gng->max_dist * ferRecp(ferVecLen(gng->dim, gng->tmpv2)));
    }

    while (dist > gng->max_dist){
        // move to next position on edge
        ferVecAdd(gng->dim, gng->tmpv, gng->tmpv2);
        dist -= gng->max_dist;

        // eval the position
        eval = gng->ops.eval(gng->tmpv, gng->ops.eval_data);
        gng->evals += 1L;
        if (eval == FER_GNG_PLAN_OBST){
            // create new node
            nn = ferGNGPlanNewNode((const void *)gng->tmpv, (void *)gng);
            n  = fer_container_of(nn, fer_gng_plan_node_t, node);

            // remove it from cells
            ferGUGRemove(gng->gug, &n->gug);

            // and add it to obst list
            ferListAppend(&gng->obst, &n->obst);
            ferGUGAdd(gng->obst_gug, &n->gug);

            // return that edge is not in FREE space
            return 0;
        }
    }

    return 1;
}

static int ferGNGPlanIsPathFree(fer_gng_plan_t *gng, fer_list_t *path)
{
    fer_list_t *item;
    fer_gng_plan_node_t *n, *p;
    const fer_vec_t *w;
    int isfree = 1;

    if (ferListEmpty(path))
        return 0;

    // check first node
    item = ferListNext(path);
    n    = FER_LIST_ENTRY(item, fer_gng_plan_node_t, path);
    if (!ferGNGPlanIsEdgeFree(gng, gng->start, n->w))
        return 0;

    // check middle nodes
    isfree = 1;
    p = n;
    w = n->w;
    for (item = ferListNext(item); item != path; item = ferListNext(item)){
        n = FER_LIST_ENTRY(item, fer_gng_plan_node_t, path);
        if (!ferGNGPlanIsEdgeFree(gng, w, n->w)){
            // delete edge that is in OBST space
            ferGNGEdgeBetweenDel(gng->gng, &p->node, &n->node);
            isfree = 0;
        }

        p = n;
        w = n->w;
    }

    if (!isfree)
        return 0;

    // check last node
    item = ferListPrev(path);
    n    = FER_LIST_ENTRY(item, fer_gng_plan_node_t, path);
    if (!ferGNGPlanIsEdgeFree(gng, n->w, gng->goal))
        return 0;

    return 1;
}

static void ferGNGPlanFixNode(fer_gng_plan_t *gng, fer_gng_plan_node_t *n)
{
    if (n->fixed)
        return;

    n->fixed = 1;
    ferGNGNodeNewAtPos(gng->gng, (const void *)n->w);
}

static int ferGNGPlanTryFindPath(fer_gng_plan_t *gng)
{
    int found = 0;

    if (ferGNGNodesLen(gng->gng) <= gng->min_nodes)
        return -1;

    do {
        ferListInit(&gng->path);

        // find path between start and goal
        ferGNGPlanFindPath(gng, gng->start, gng->goal, &gng->path);

        if (!ferListEmpty(&gng->path)){
            found = 1;
        }

        // cut path from obstacle nodes
        if (ferGNGPlanCutPath(gng, &gng->path) != 0){
            // check if path is in FREE space, i.e., if there is (or can be
            // create in FREE space) edges between all nodes of max length gng->max_dist
            if (ferGNGPlanIsPathFree(gng, &gng->path))
                return 1;
        }
    } while (!ferListEmpty(&gng->path));

    if (found)
        return 0;
    return -1;
}

static const void *ferGNGPlanInputSignal(void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    fer_gug_el_t *el;
    fer_gng_plan_node_t *node;
    const fer_vec_t *vec;
    size_t num_els;
    fer_real_t dist;

    do {
        // get input signal
        vec = gng->ops.input_signal(gng->ops.input_signal_data);

        dist = FER_REAL_MAX;
        // find nearest node in obstacles
        num_els = ferGUGNearest(gng->obst_gug, vec, 1, &el);
        if (num_els == 1){
            node = fer_container_of(el, fer_gng_plan_node_t, gug);
            dist = ferVecDist(gng->dim, vec, node->w);
            //DBG("dist: %f", dist);
        }
    } while (dist < gng->max_dist);

    return (void *)vec;
}

static void ferGNGPlanInit(fer_gng_node_t **n1, fer_gng_node_t **n2, void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;

    *n1 = ferGNGPlanNewNode((const void *)gng->start, (void *)gng);
    *n2 = ferGNGPlanNewNode((const void *)gng->goal, (void *)gng);
}

static fer_gng_node_t *ferGNGPlanNewNode(const void *input_signal, void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    fer_gng_plan_node_t *node;

    node = FER_ALLOC(fer_gng_plan_node_t);

    node->w = ferVecClone(gng->dim, (const fer_vec_t *)input_signal);

    ferGUGElInit(&node->gug, node->w);
    ferGUGAdd(gng->gug, &node->gug);

    node->fixed = 0;

    return &node->node;
}

static fer_gng_node_t *ferGNGPlanNewNodeBetween(const fer_gng_node_t *_n1,
                                                const fer_gng_node_t *_n2,
                                                void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    fer_gng_plan_node_t *n1 = (fer_gng_plan_node_t *)_n1;
    fer_gng_plan_node_t *n2 = (fer_gng_plan_node_t *)_n2;

    ferVecAdd2(gng->dim, gng->tmpv, n1->w, n2->w);
    ferVecScale(gng->dim, gng->tmpv, FER_REAL(0.5));

    return ferGNGPlanNewNode((const void *)gng->tmpv, (void *)gng);
}

static void ferGNGPlanDelNode(fer_gng_node_t *_n, void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    fer_gng_plan_node_t *n = (fer_gng_plan_node_t *)_n;

    ferVecDel(n->w);
    ferGUGRemove(gng->gug, &n->gug);
    FER_FREE(n);
}

static void ferGNGPlanNearest(const void *input_signal,
                              fer_gng_node_t **n1_out, fer_gng_node_t **n2_out,
                              void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    fer_gug_el_t *els[2];
    fer_gng_plan_node_t *n1, *n2;

    ferGUGNearest(gng->gug, (const fer_vec_t *)input_signal, 2, els);
    n1 = fer_container_of(els[0], fer_gng_plan_node_t, gug);
    n2 = fer_container_of(els[1], fer_gng_plan_node_t, gug);

    *n1_out = &n1->node;
    *n2_out = &n2->node;
}

static fer_real_t ferGNGPlanDist2(const void *input_signal,
                                  const fer_gng_node_t *node, void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    const fer_gng_plan_node_t *n; 

    n = fer_container_of(node, fer_gng_plan_node_t, node);

    return ferVecDist2(gng->dim, (const fer_vec_t *)input_signal, n->w);
}

static void ferGNGPlanMoveTowards(fer_gng_node_t *node,
                                  const void *input_signal,
                                  fer_real_t fraction, void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    fer_gng_plan_node_t *n; 

    n = fer_container_of(node, fer_gng_plan_node_t, node);

    // don't move fixed nodes
    if (n->fixed)
        return;

    ferVecSub2(gng->dim, gng->tmpv, (const fer_vec_t *)input_signal, n->w);
    ferVecScale(gng->dim, gng->tmpv, fraction);
    ferVecAdd(gng->dim, n->w, gng->tmpv);

    ferGUGUpdate(gng->gug, &n->gug);
}



/*** Find path ***/
static void ferGNGPlanFindPathExpand(fer_dij_node_t *_n,
                                     fer_list_t *expand, void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    fer_list_t *list, *item;
    fer_gng_node_t *go;
    fer_gng_plan_node_t *n, *o;
    fer_net_edge_t *edge;
    fer_net_node_t *node;
    fer_real_t dist;

    n = fer_container_of(_n, fer_gng_plan_node_t, dij);

    list = ferNetNodeEdges(&n->node.node);
    FER_LIST_FOR_EACH(list, item){
        edge = ferNetEdgeFromNodeList(item);
        node = ferNetEdgeOtherNode(edge, &n->node.node);
        go   = fer_container_of(node, fer_gng_node_t, node);
        o    = fer_container_of(go, fer_gng_plan_node_t, node);

        if (!ferDijNodeClosed(&o->dij)){
            dist = ferVecDist(gng->dim, n->w, o->w);
            if (dist < gng->min_dist){
                ferDijNodeAdd(&o->dij, expand, dist);
            }
        }
    }
}

static void ferGNGPlanFindPathDijInit(fer_gng_plan_t *gng)
{
    fer_list_t *list, *item;
    fer_gng_node_t *gn;
    fer_gng_plan_node_t *n;

    list = ferGNGNodes(gng->gng);
    FER_LIST_FOR_EACH(list, item){
        gn = ferGNGNodeFromList(item);
        n  = fer_container_of(gn, fer_gng_plan_node_t, node);
        ferDijNodeInit(&n->dij);
    }
}


static void ferGNGPlanObtainPath(fer_gng_plan_node_t *s, fer_gng_plan_node_t *g,
                                 fer_list_t *list)
{
    fer_gng_plan_node_t *n;
    fer_dij_node_t *dn;

    ferListPrepend(list, &g->path);

    if (g == s)
        return;

    dn = g->dij.prev;
    while (dn != &s->dij){
        n = fer_container_of(dn, fer_gng_plan_node_t, dij);
        ferListPrepend(list, &n->path);

        dn = dn->prev;
    }
}

static int ferGNGPlanFindPath(fer_gng_plan_t *gng,
                              const fer_vec_t *wstart,
                              const fer_vec_t *wgoal,
                              fer_list_t *list)
{
    fer_dij_ops_t ops;
    fer_dij_t *dij;
    fer_gng_plan_node_t *n[2];
    fer_gug_el_t *el[2];
    int result;

    // get nearest nodes to wstart and wgoal
    if (ferGUGNearest(gng->gug, wstart, 1, &el[0]) != 1)
        return -1;
    if (ferGUGNearest(gng->gug, wgoal, 1, &el[1]) != 1)
        return -1;
    n[0] = fer_container_of(el[0], fer_gng_plan_node_t, gug);
    n[1] = fer_container_of(el[1], fer_gng_plan_node_t, gug);

    // initialize whole net
    ferGNGPlanFindPathDijInit(gng);

    // initialize operations
    ferDijOpsInit(&ops);
    ops.expand = ferGNGPlanFindPathExpand;
    ops.data   = (void *)gng;

    // create dijkstra algorithm
    dij = ferDijNew(&ops);

    // run dijkstra
    result = ferDijRun(dij, &n[0]->dij, &n[1]->dij);

    if (result == 0){
        ferGNGPlanObtainPath(n[0], n[1], list);
        ferDijDel(dij);
        return 0;
    }

    ferDijDel(dij);

    return 0;
}


void ferGNGPlanDumpNetSVT(fer_gng_plan_t *gng, FILE *out, const char *name)
{
    fer_list_t *list, *item;
    fer_net_t *net;
    fer_net_node_t *nn;
    fer_gng_node_t *gn;
    fer_net_edge_t *e;
    fer_gng_plan_node_t *n;
    size_t i, id1, id2;

    if (gng->dim != 2 && gng->dim != 3)
        return;

    net = ferGNGPlanNet(gng);

    fprintf(out, "--------\n");

    if (name){
        fprintf(out, "Name: %s FREE\n", name);
    }else{
        fprintf(out, "Name: FREE\n");
    }

    fprintf(out, "Points:\n");
    list = ferNetNodes(net);
    i = 0;
    FER_LIST_FOR_EACH(list, item){
        nn = FER_LIST_ENTRY(item, fer_net_node_t, list);
        gn = ferGNGNodeFromNet(nn);
        n  = fer_container_of(gn, fer_gng_plan_node_t, node);

        n->_id = i++;
        if (gng->dim == 2){
            ferVec2Print((const fer_vec2_t *)n->w, out);
        }else{
            ferVec3Print((const fer_vec3_t *)n->w, out);
        }
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = ferGNGEdges(gng->gng);
    FER_LIST_FOR_EACH(list, item){
        e = FER_LIST_ENTRY(item, fer_net_edge_t, list);

        nn = ferNetEdgeNode(e, 0);
        gn = ferGNGNodeFromNet(nn);
        n  = fer_container_of(gn, fer_gng_plan_node_t, node);
        id1 = n->_id;

        nn = ferNetEdgeNode(e, 1);
        gn = ferGNGNodeFromNet(nn);
        n  = fer_container_of(gn, fer_gng_plan_node_t, node);
        id2 = n->_id;
        fprintf(out, "%d %d\n", (int)id1, (int)id2);
    }

    fprintf(out, "--------\n");
    fflush(out);
}

void ferGNGPlanDumpObstSVT(fer_gng_plan_t *gng, FILE *out, const char *name)
{
    fer_list_t *item;
    fer_gng_plan_node_t *n;

    if (ferListEmpty(&gng->obst))
        return;


    fprintf(out, "--------\n");
    if (name){
        fprintf(out, "Name: %s OBST\n", name);
    }else{
        fprintf(out, "Name: OBST\n");
    }

    fprintf(out, "Point color: 0.8 0.1 0.1\n");
    fprintf(out, "Point size: 2\n");
    fprintf(out, "Points:\n");
    FER_LIST_FOR_EACH(&gng->obst, item){
        n = FER_LIST_ENTRY(item, fer_gng_plan_node_t, obst);

        if (gng->dim == 2){
            ferVec2Print((const fer_vec2_t *)n->w, out);
        }else{
            ferVec3Print((const fer_vec3_t *)n->w, out);
        }
        fprintf(out, "\n");
    }

    fprintf(out, "--------\n");
    fflush(out);
}

void ferGNGPlanDumpPathSVT(fer_gng_plan_t *gng, FILE *out, const char *name)
{
    fer_gng_plan_node_t *n;
    fer_list_t *item;
    size_t i, len;

    if (ferListEmpty(&gng->path))
        return;

    fprintf(out, "--------\n");
    fflush(out);

    if (name){
        fprintf(out, "Name: %s Path\n", name);
    }else{
        fprintf(out, "Name: Path\n");
    }

    fprintf(out, "Point color: 0.1 0.8 0.1\n");
    fprintf(out, "Edge color: 0.1 0.8 0.1\n");
    fprintf(out, "Edge width: 3\n");
    fprintf(out, "Points:\n");
    fflush(out);
    i = 0;
    FER_LIST_FOR_EACH(&gng->path, item){
        n = FER_LIST_ENTRY(item, fer_gng_plan_node_t, path);

        n->_id = i++;
        if (gng->dim == 2){
            ferVec2Print((const fer_vec2_t *)n->w, out);
        }else{
            ferVec3Print((const fer_vec3_t *)n->w, out);
        }
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    len = i - 1;
    for (i = 0; i < len; i++){
        fprintf(out, "%d %d\n", i, i + 1);
    }

    fprintf(out, "--------\n");
    fflush(out);
}

void ferGNGPlanDumpPath(fer_gng_plan_t *gng, FILE *out)
{
    fer_gng_plan_node_t *n;
    fer_list_t *item;
    size_t i;

    if (ferListEmpty(&gng->path))
        return;

    FER_LIST_FOR_EACH(&gng->path, item){
        n = FER_LIST_ENTRY(item, fer_gng_plan_node_t, path);

        for (i = 0; i < gng->dim; i++){
            fprintf(out, "%f ", (float)ferVecGet(n->w, i));
        }
        fprintf(out, "\n");
    }
    fflush(out);
}
