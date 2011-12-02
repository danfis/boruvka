/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
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

#include <limits.h>
#include <fermat/gnnp.h>
#include <fermat/sort.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>


#define IS_FIXED(n) ((n)->fixed != 0)
#define IS_FREE(n) ((n)->fixed == 1)
#define IS_OBST(n) ((n)->fixed == 2)
#define SET_FREE(n) (n)->fixed = 1
#define SET_OBST(n) (n)->fixed = 2

#define PATH_NONE 0
#define PATH_INIT 1
#define PATH_GOAL 2

#define PATH_IS_NONE(n) ((n)->prev_type == PATH_NONE)
#define PATH_IS_INIT(n) ((n)->prev_type == PATH_INIT)
#define PATH_IS_GOAL(n) ((n)->prev_type == PATH_GOAL)

static void netNodeDel(fer_net_node_t *n, void *);
static void netEdgeDel(fer_net_edge_t *e, void *);

static fer_gnnp_node_t *ferGNNPNodeNew(fer_gnnp_t *nn, const fer_vec_t *w);
static void ferGNNPNodeDel(fer_gnnp_t *nn, fer_gnnp_node_t *node);
/** Removes the longest edges incidenting with the node */
static void ferGNNPNodeRemoveLongestEdge(fer_gnnp_t *nn, fer_gnnp_node_t *node);
/** Moves the node towards input signal: w = w + e * (is - w) */
static void ferGNNPNodeMoveTowards(fer_gnnp_t *nn, fer_gnnp_node_t *node,
                                   const fer_vec_t *is, fer_real_t r);

static int init(fer_gnnp_t *nn, const fer_vec_t *init, const fer_vec_t *goal,
                fer_list_t *path);
static void nearest(fer_gnnp_t *nn, const fer_vec_t *is,
                    fer_gnnp_node_t **n1,
                    fer_gnnp_node_t **n2);
static fer_gnnp_node_t *nearestPath(fer_gnnp_t *nn, const fer_vec_t *is);
static void hebbianLearning(fer_gnnp_t *nn, fer_gnnp_node_t *n1,
                                            fer_gnnp_node_t *n2);
static fer_gnnp_node_t *newNode(fer_gnnp_t *nn, fer_gnnp_node_t *wn,
                                const fer_vec_t *is);
static void move(fer_gnnp_t *nn, fer_gnnp_node_t *wn, const fer_vec_t *is);

static int learnPath(fer_gnnp_t *nn, fer_gnnp_node_t *n1,
                                     fer_list_t *path);
static int prunePath(fer_gnnp_t *nn, fer_list_t *path);


_fer_inline void pathSetNone(fer_gnnp_t *nn, fer_gnnp_node_t *n);
_fer_inline void pathConnect(fer_gnnp_t *nn, fer_gnnp_node_t *from,
                                             fer_gnnp_node_t *to);
static void pathRetype(fer_gnnp_t *nn, fer_gnnp_node_t *root);


#define OPS_DATA(name) \
    if (!nn->ops.name ## _data) \
        nn->ops.name ## _data = nn->ops.data;

#define OPS_CHECK(name) \
    if (!nn->ops.name){ \
        fprintf(stderr, "Fermat :: GNNP :: No " #name " callback set.\n"); \
        exit(-1); \
    }

#define OPS_CHECK_DATA(name) \
    OPS_DATA(name) \
    OPS_CHECK(name)


void ferGNNPParamsInit(fer_gnnp_params_t *p)
{
    p->dim  = 2;
    p->ew   = 0.05;
    p->en   = 0.0005;
    p->rmax = 4;
    p->h    = 0.1;

    ferNNParamsInit(&p->nn);
    p->nn.gug.dim    = 2;
    p->nn.vptree.dim = 2;
    p->nn.linear.dim = 2;
}

void ferGNNPOpsInit(fer_gnnp_ops_t *ops)
{
    bzero(ops, sizeof(*ops));
}

fer_gnnp_t *ferGNNPNew(const fer_gnnp_ops_t *ops, const fer_gnnp_params_t *p)
{
    fer_gnnp_t *nn;

    nn = FER_ALLOC(fer_gnnp_t);
    nn->params = *p;
    nn->params.h *= nn->params.h;

    nn->ops    = *ops;
    OPS_CHECK_DATA(input_signal)
    OPS_CHECK_DATA(terminate)
    OPS_CHECK_DATA(eval)
    OPS_DATA(callback)

    nn->net = ferNetNew();
    nn->nn  = ferNNNew(&nn->params.nn);
    nn->nn_path = ferNNNew(&nn->params.nn);

    nn->init = nn->goal = NULL;
    nn->tmpv = ferVecNew(nn->params.dim);

    return nn;
}

void ferGNNPDel(fer_gnnp_t *nn)
{
    ferNetDel2(nn->net, netNodeDel, (void *)nn, netEdgeDel, (void *)nn);
    ferNNDel(nn->nn);
    ferNNDel(nn->nn_path);

    ferVecDel(nn->tmpv);
    FER_FREE(nn);
}

int ferGNNPFindPath(fer_gnnp_t *nn,
                    const fer_vec_t *s, const fer_vec_t *goal,
                    fer_list_t *path)
{
    const fer_vec_t *is;
    fer_gnnp_node_t *n[2], *np;
    fer_real_t dist;
    unsigned int cb = 0U;
    unsigned long c = 0UL;

    // Init
    if (init(nn, s, goal, path) == 1)
        return 0;

    while (!nn->ops.terminate(nn, nn->ops.terminate_data)){
        cb += 1U;

        // get input signal
        is = nn->ops.input_signal(nn, nn->ops.input_signal_data);

        // find two nearest nodes
        nearest(nn, is, &n[0], &n[1]);

        // competitive hebbian learning with removing edges
        hebbianLearning(nn, n[0], n[1]);

        // learn path
        np = nearestPath(nn, is);
        if (learnPath(nn, np, path) == 0){
            // prune the path if found
            if (!prunePath(nn, path))
                return 0;
        }

        if (IS_FIXED(n[0])){
            // the nearest node is fixed (free or obstacle)
            // compute distance between input signal and the winner node
            dist = ferVecDist2(nn->params.dim, is, n[0]->w);

            // if input signal is nearest than a resolution, create a new
            // node
            if (dist > nn->params.h){
                newNode(nn, n[0], is);
            }
        }else{
            // move winner node and its neighborhood towards input signal
            move(nn, n[0], is);
        }

        if (nn->ops.callback && nn->ops.callback_period == cb){
            nn->ops.callback(nn, nn->ops.callback_data);
            cb = 0L;
        }

        c += 1UL;
    }

    return -1;
}

static void dumpNodes(const fer_gnnp_t *nn, FILE *out, int type)
{
    fer_list_t *list, *item;
    fer_net_node_t *netn;
    fer_gnnp_node_t *n;

    fprintf(out, "---\n");

    fprintf(out, "Name: Nodes %d\n", type);
    if (type == 0)
        fprintf(out, "Point color: 0.1 0.1 0.1\n");
    if (type == 1)
        fprintf(out, "Point color: 0.1 0.1 0.8\n");
    if (type == 2)
        fprintf(out, "Point color: 0.8 0.1 0.1\n");
    fprintf(out, "Point size: 1\n");
    fprintf(out, "Points:\n");
    list = ferNetNodes(nn->net);
    FER_LIST_FOR_EACH(list, item){
        netn = FER_LIST_ENTRY(item, fer_net_node_t, list);
        n    = fer_container_of(netn, fer_gnnp_node_t, net);
        if (n->fixed == type){
            ferVecPrint(nn->params.dim, n->w, out);
            fprintf(out, "\n");
        }
    }

    fprintf(out, "---\n");
}

static void dumpPath(const fer_gnnp_t *nn, FILE *out, int type)
{
    fer_list_t *list, *item;
    fer_net_node_t *netn;
    fer_gnnp_node_t *n;

    fprintf(out, "---\n");

    fprintf(out, "Name: paths %d\n", type);
    if (type == 1)
        fprintf(out, "Point color: 0.8 0.1 0.8\n");
    if (type == 2)
        fprintf(out, "Point color: 0.1 0.8 0.8\n");
    fprintf(out, "Point size: 3\n");
    fprintf(out, "Points:\n");
    list = ferNetNodes(nn->net);
    FER_LIST_FOR_EACH(list, item){
        netn = FER_LIST_ENTRY(item, fer_net_node_t, list);
        n    = fer_container_of(netn, fer_gnnp_node_t, net);
        if (n->prev_type == type){
            ferVecPrint(nn->params.dim, n->w, out);
            fprintf(out, "\n");
        }
    }

    fprintf(out, "---\n");
}

static void dumpNet(const fer_gnnp_t *nn, FILE *out)
{
    fer_list_t *list, *item;
    size_t i;
    fer_net_node_t *netn;
    fer_gnnp_node_t *n;
    fer_net_edge_t *e;
    size_t id1, id2;

    fprintf(out, "----\n");
    fprintf(out, "Name: net\n");

    fprintf(out, "Points off: 1\n");
    fprintf(out, "Edge color: 0.5 0.5 0.5\n");
    fprintf(out, "Edge width: 1\n");
    fprintf(out, "Points:\n");
    list = ferNetNodes(nn->net);
    i = 0;
    FER_LIST_FOR_EACH(list, item){
        netn = FER_LIST_ENTRY(item, fer_net_node_t, list);
        n    = fer_container_of(netn, fer_gnnp_node_t, net);
        n->_id = i++;
        ferVecPrint(nn->params.dim, n->w, out);
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = ferNetEdges(nn->net);
    FER_LIST_FOR_EACH(list, item){
        e = FER_LIST_ENTRY(item, fer_net_edge_t, list);

        netn = ferNetEdgeNode(e, 0);
        n    = fer_container_of(netn, fer_gnnp_node_t, net);
        id1 = n->_id;

        netn = ferNetEdgeNode(e, 1);
        n    = fer_container_of(netn, fer_gnnp_node_t, net);
        id2 = n->_id;
        fprintf(out, "%d %d\n", (int)id1, (int)id2);
    }

    fprintf(out, "--------\n");
}

void ferGNNPDumpSVT(const fer_gnnp_t *nn, FILE *out, const char *name)
{
    if (nn->params.dim != 2 && nn->params.dim != 3)
        return;

    dumpPath(nn, out, 1);
    dumpPath(nn, out, 2);
    dumpNodes(nn, out, 0);
    dumpNodes(nn, out, 1);
    dumpNodes(nn, out, 2);
    //dumpNet(nn, out);
}

static void netNodeDel(fer_net_node_t *n, void *_nn)
{
    fer_gnnp_t *nn = (fer_gnnp_t *)_nn;
    fer_gnnp_node_t *node = fer_container_of(n, fer_gnnp_node_t, net);
    ferGNNPNodeDel(nn, node);
}

static void netEdgeDel(fer_net_edge_t *e, void *_nn)
{
    ferNetEdgeDel(e);
}

static fer_gnnp_node_t *ferGNNPNodeNew(fer_gnnp_t *nn, const fer_vec_t *w)
{
    fer_gnnp_node_t *n;

    n = FER_ALLOC(fer_gnnp_node_t);

    n->fixed = 0;
    ferListInit(&n->path);
    n->prev = NULL;
    n->prev_type = PATH_NONE;

    n->w = ferVecClone(nn->params.dim, w);

    ferNetAddNode(nn->net, &n->net);

    ferNNElInit(nn->nn, &n->nn, n->w);
    ferNNElInit(nn->nn_path, &n->nn_path, n->w);
    ferNNAdd(nn->nn, &n->nn);

    return n;
}

static void ferGNNPNodeDel(fer_gnnp_t *nn, fer_gnnp_node_t *node)
{
    ferNetRemoveNode(nn->net, &node->net);
    ferNNRemove(nn->nn, &node->nn);
    ferVecDel(node->w);
    FER_FREE(node);

    /* Nodes are deleted only at the end of the run, so no need to remove
     * it from nn->nodes[] array */
}

static void ferGNNPNodeRemoveLongestEdge(fer_gnnp_t *nn, fer_gnnp_node_t *node)
{
    fer_list_t *list, *item;
    fer_net_edge_t *e, *maxe;
    fer_net_node_t *netn;
    fer_gnnp_node_t *n2;
    fer_real_t len, max;

    max  = -FER_REAL_MAX;
    maxe = NULL;

    list = ferNetNodeEdges(&node->net);
    FER_LIST_FOR_EACH(list, item){
        e = ferNetEdgeFromNodeList(item);
        netn = ferNetEdgeOtherNode(e, &node->net);
        n2   = fer_container_of(netn, fer_gnnp_node_t, net);

        len = ferVecDist2(nn->params.dim, node->w, n2->w);
        if (len > max){
            max  = len;
            maxe = e;
        }
    }

    if (maxe){
        netn = ferNetEdgeOtherNode(maxe, &node->net);
        n2   = fer_container_of(netn, fer_gnnp_node_t, net);

        if (node->prev == n2){
            pathSetNone(nn, node);
        }
        if (n2->prev == node){
            pathSetNone(nn, n2);
        }

        ferNetRemoveEdge(nn->net, maxe);
        ferNetEdgeDel(maxe);
    }

}

static void ferGNNPNodeMoveTowards(fer_gnnp_t *nn, fer_gnnp_node_t *node,
                                   const fer_vec_t *is, fer_real_t r)
{
    ferVecSub2(nn->params.dim, nn->tmpv, is, node->w);
    ferVecScale(nn->params.dim, nn->tmpv, r);
    ferVecAdd(nn->params.dim, node->w, nn->tmpv);
    ferNNUpdate(nn->nn, &node->nn);
    if (node->prev_type != PATH_NONE)
        ferNNUpdate(nn->nn_path, &node->nn_path);
}


static int init(fer_gnnp_t *nn, const fer_vec_t *init, const fer_vec_t *goal,
                fer_list_t *path)
{
    fer_net_edge_t *e;

    nn->init = ferGNNPNodeNew(nn, init);
    SET_FREE(nn->init);
    nn->goal = ferGNNPNodeNew(nn, goal);
    SET_FREE(nn->goal);
    nn->init->prev_type = PATH_INIT;
    ferNNAdd(nn->nn_path, &nn->init->nn_path);
    nn->goal->prev_type = PATH_GOAL;
    ferNNAdd(nn->nn_path, &nn->goal->nn_path);

    e = ferNetEdgeNew();
    ferNetAddEdge(nn->net, e, &nn->init->net, &nn->goal->net);

    ferListInit(path);
    ferListAppend(path, &nn->init->path);
    ferListAppend(path, &nn->goal->path);
    if (!prunePath(nn, path))
        return 1;
    return 0;
}

static void nearest(fer_gnnp_t *nn, const fer_vec_t *is,
                    fer_gnnp_node_t **n1,
                    fer_gnnp_node_t **n2)
{
    fer_nn_el_t *els[2];

    ferNNNearest(nn->nn, is, 2, els);
    *n1 = fer_container_of(els[0], fer_gnnp_node_t, nn);
    *n2 = fer_container_of(els[1], fer_gnnp_node_t, nn);
}

static fer_gnnp_node_t *nearestPath(fer_gnnp_t *nn, const fer_vec_t *is)
{
    fer_nn_el_t *els;
    fer_gnnp_node_t *np;

    ferNNNearest(nn->nn_path, is, 1, &els);
    np = fer_container_of(els, fer_gnnp_node_t, nn_path);

    return np;
}

static int learnPath(fer_gnnp_t *nn, fer_gnnp_node_t *wn,
                                     fer_list_t *path)
{
    fer_list_t *list, *item;
    fer_net_edge_t *e;
    fer_net_node_t *netn;
    fer_gnnp_node_t *o;
    fer_gnnp_node_t *foundpath = NULL;


    /** wn is not OBST and also non-PATH_NONE */

    list = ferNetNodeEdges(&wn->net);
    FER_LIST_FOR_EACH(list, item){
        e = ferNetEdgeFromNodeList(item);
        netn = ferNetEdgeOtherNode(e, &wn->net);
        o    = fer_container_of(netn, fer_gnnp_node_t, net);

        if (IS_OBST(o))
            continue;

        if (PATH_IS_NONE(o)){
            // {o} is not connected init neither to goal node
            pathConnect(nn, o, wn);
        }else if (o->prev_type != wn->prev_type){
            // we found a connection between init and goal node
            foundpath = o;
        }
    }

    if (foundpath){
        // obtain path
        ferListInit(path);

        // first init node
        o = foundpath;
        if (PATH_IS_INIT(wn))
            o = wn;
        while (o != nn->init){
            ferListPrepend(path, &o->path);
            o = o->prev;
        }
        ferListPrepend(path, &nn->init->path);

        // goal node
        o = foundpath;
        if (PATH_IS_GOAL(wn))
            o = wn;
        while (o != nn->goal){
            ferListAppend(path, &o->path);
            o = o->prev;
        }
        ferListAppend(path, &nn->goal->path);

        return 0;
    }

    return -1;
}

static void hebbianLearning(fer_gnnp_t *nn, fer_gnnp_node_t *n1,
                                            fer_gnnp_node_t *n2)
{
    fer_net_edge_t *e;

    // get common edge
    e = ferNetNodeCommonEdge(&n1->net, &n2->net);

    if (e == NULL){
        // remove longest edge if adding a new edge would exceeds the limit
        if (ferNetNodeEdgesLen(&n1->net) >= nn->params.rmax){
            ferGNNPNodeRemoveLongestEdge(nn, n1);
        }
        if (ferNetNodeEdgesLen(&n2->net) >= nn->params.rmax){
            ferGNNPNodeRemoveLongestEdge(nn, n2);
        }

        // add new edge
        e = ferNetEdgeNew();
        ferNetAddEdge(nn->net, e, &n1->net, &n2->net);
    }
}

static fer_gnnp_node_t *newNode(fer_gnnp_t *nn, fer_gnnp_node_t *wn,
                                const fer_vec_t *is)
{
    fer_gnnp_node_t *n;
    fer_net_edge_t *e;

    n = ferGNNPNodeNew(nn, is);

    e = ferNetEdgeNew();
    ferNetAddEdge(nn->net, e, &wn->net, &n->net);

    pathConnect(nn, n, wn);

    return n;
}

static void move(fer_gnnp_t *nn, fer_gnnp_node_t *wn, const fer_vec_t *is)
{
    fer_list_t *list, *item;
    fer_net_edge_t *e;
    fer_net_node_t *netn;
    fer_gnnp_node_t *n;

    // move winner node
    if (!IS_FIXED(wn))
        ferGNNPNodeMoveTowards(nn, wn, is, nn->params.ew);

    // move neighbor nodes
    list = ferNetNodeEdges(&wn->net);
    FER_LIST_FOR_EACH(list, item){
        e = ferNetEdgeFromNodeList(item);
        netn = ferNetEdgeOtherNode(e, &wn->net);
        n    = fer_container_of(netn, fer_gnnp_node_t, net);

        if (!IS_FIXED(n))
            ferGNNPNodeMoveTowards(nn, n, is, nn->params.en);
    }
}


static void findPathDijExpand(fer_dij_node_t *_n,
                              fer_list_t *expand, void *data)
{
    fer_gnnp_t *nn = (fer_gnnp_t *)data;
    fer_list_t *list, *item;
    fer_net_edge_t *e;
    fer_net_node_t *netn;
    fer_gnnp_node_t *n, *root;
    fer_real_t dist;

    root = fer_container_of(_n, fer_gnnp_node_t, dij);
    list = ferNetNodeEdges(&root->net);
    FER_LIST_FOR_EACH(list, item){
        e = ferNetEdgeFromNodeList(item);
        netn = ferNetEdgeOtherNode(e, &root->net);
        n    = fer_container_of(netn, fer_gnnp_node_t, net);

        if (!ferDijNodeClosed(&n->dij) && !IS_OBST(n)){
            dist = ferVecDist(nn->params.dim, root->w, n->w);
            ferDijNodeAdd(&n->dij, expand, dist);
        }
    }
}

static void findPathDijInit(fer_gnnp_t *nn)
{
    fer_list_t *list, *item;
    fer_net_node_t *netn;
    fer_gnnp_node_t *n;

    list = ferNetNodes(nn->net);
    FER_LIST_FOR_EACH(list, item){
        netn = FER_LIST_ENTRY(item, fer_net_node_t, list);
        n    = fer_container_of(netn, fer_gnnp_node_t, net);
        ferDijNodeInit(&n->dij);
    }
}

static int findPathDij(fer_gnnp_t *nn, fer_list_t *path)
{
    fer_dij_ops_t ops;
    fer_dij_t *dij;
    int found;
    fer_gnnp_node_t *n;
    fer_dij_node_t *dn;

    ferListInit(path);
    findPathDijInit(nn);

    // initialize operators
    ferDijOpsInit(&ops);
    ops.expand = findPathDijExpand;
    ops.data   = (void *)nn;

    // create dijkstra algorithm
    dij = ferDijNew(&ops);

    // run dijkstra
    found = ferDijRun(dij, &nn->init->dij, &nn->goal->dij);
    if (found != 0)
        return -1;

    dn = &nn->goal->dij;
    while (dn != &nn->init->dij){
        n = fer_container_of(dn, fer_gnnp_node_t, dij);
        ferListPrepend(path, &n->path);

        dn = dn->prev;
    }
    ferListPrepend(path, &nn->init->path);

    return 0;
}

static int findPath(fer_gnnp_t *nn, fer_list_t *path)
{
    return findPathDij(nn, path);
}


static int _pruneEval(fer_gnnp_t *nn, fer_gnnp_node_t *n)
{
    int eval;

    if (IS_FREE(n)){
        return 0;
    }else if (IS_OBST(n)){
        return 1;
    }

    eval = nn->ops.eval(nn, n->w, nn->ops.eval_data);
    if (eval){
        SET_FREE(n);
        return 0;
    }else{
        SET_OBST(n);
        return 1;
    }
}

static int _pruneBetween(fer_gnnp_t *nn,
                         fer_gnnp_node_t *n1, fer_gnnp_node_t *n2,
                         fer_list_t *path)
{
    fer_gnnp_node_t *n;
    fer_net_edge_t *e;
    fer_real_t dist;
    int ret = 0;

    dist = ferVecDist2(nn->params.dim, n1->w, n2->w);
    if (dist < nn->params.h)
        return 0;

    // create new node half way between n1 and n2
    ferVecAdd2(nn->params.dim, nn->tmpv, n1->w, n2->w);
    ferVecScale(nn->params.dim, nn->tmpv, FER_REAL(0.5));
    n = ferGNNPNodeNew(nn, nn->tmpv);

    // remove an old edge
    e = ferNetNodeCommonEdge(&n1->net, &n2->net);
    ferNetRemoveEdge(nn->net, e);
    ferNetEdgeDel(e);

    // create two new edges n1-n and n-n2
    e = ferNetEdgeNew();
    ferNetAddEdge(nn->net, e, &n1->net, &n->net);
    e = ferNetEdgeNew();
    ferNetAddEdge(nn->net, e, &n->net, &n2->net);

    // evaluate node
    /*
    if (_pruneEval(nn, n))
        return 1;
    if (_pruneBetween(nn, n1, n))
        return 1;
    if (_pruneBetween(nn, n, n2))
        return 1;
    return 0;
    */
    ret |= _pruneEval(nn, n);

    if (dist * FER_REAL(0.5) < nn->params.h){
        ferListAppend(path, &n->path);
        return ret;
    }

    ret |= _pruneBetween(nn, n1, n, path);
    ferListAppend(path, &n->path);
    ret |= _pruneBetween(nn, n, n2, path);
    return ret;
}

static void _prunePath(fer_gnnp_t *nn, fer_list_t *path)
{
    fer_list_t *item;
    fer_gnnp_node_t *n, *p;
    fer_gnnp_node_t *reset_f = NULL, *reset_t = NULL;

    n = NULL;
    FER_LIST_FOR_EACH(path, item){
        p = n;
        n = FER_LIST_ENTRY(item, fer_gnnp_node_t, path);

        if (!IS_FREE(n)){
            reset_f = n;
            break;
        }

        if (p){
            pathConnect(nn, n, p);
        }
    }


    n = NULL;
    item = ferListPrev(path);
    while (item != path){
        p = n;
        n = FER_LIST_ENTRY(item, fer_gnnp_node_t, path);

        if (!IS_FREE(n)){
            reset_t = n;
            break;
        }

        if (p){
            pathConnect(nn, n, p);
        }

        item = ferListPrev(item);
    }

    if (reset_t == NULL)
        return;

    while (reset_f != reset_t){
        pathSetNone(nn, reset_f);
        if (IS_OBST(reset_f))
            reset_f->prev = NULL;
        item = ferListNext(&reset_f->path);
        reset_f = FER_LIST_ENTRY(item, fer_gnnp_node_t, path);
    }
    pathSetNone(nn, reset_f);
}

static int prunePath(fer_gnnp_t *nn, fer_list_t *path)
{
    fer_list_t *item, prune_path;
    fer_gnnp_node_t *n1, *n2;
    int ret = 0;

    prune_path = *path;
    prune_path.next->prev = &prune_path;
    prune_path.prev->next = &prune_path;

    ferListInit(path);

    n2 = NULL;
    n1 = NULL;
    while (!ferListEmpty(&prune_path)){
        n1 = n2;
        item = ferListNext(&prune_path);
        ferListDel(item);
        n2 = FER_LIST_ENTRY(item, fer_gnnp_node_t, path);

        /*
        if (_pruneEval(nn, n2))
            return 1;

        if (n1 && n2){
            if (_pruneBetween(nn, n1, n2))
                return 1;
        }
        */
        ret |= _pruneEval(nn, n2);

        if (n1 && n2){
            ret |= _pruneBetween(nn, n1, n2, path);
        }

        ferListAppend(path, &n2->path);
    }

    if (ret){
        _prunePath(nn, path);
    }

    //return 0;
    return ret;
}

_fer_inline void pathSetNone(fer_gnnp_t *nn, fer_gnnp_node_t *n)
{
    if (n->prev_type != PATH_NONE)
        ferNNRemove(nn->nn_path, &n->nn_path);
    n->prev_type = PATH_NONE;
    pathRetype(nn, n);
}

_fer_inline void pathConnect(fer_gnnp_t *nn, fer_gnnp_node_t *from,
                                             fer_gnnp_node_t *to)
{
    if (from->prev_type != PATH_NONE && to->prev_type == PATH_NONE){
        ferNNRemove(nn->nn_path, &from->nn_path);
    }else if (from->prev_type == PATH_NONE && to->prev_type != PATH_NONE){
        ferNNAdd(nn->nn_path, &from->nn_path);
    }
    from->prev = to;
    from->prev_type = to->prev_type;
    pathRetype(nn, from);
}

static void pathRetype(fer_gnnp_t *nn, fer_gnnp_node_t *root)
{
    fer_list_t *list, *item;
    fer_net_edge_t *e;
    fer_net_node_t *netn;
    fer_gnnp_node_t *n2;

    list = ferNetNodeEdges(&root->net);
    FER_LIST_FOR_EACH(list, item){
        e = ferNetEdgeFromNodeList(item);
        netn = ferNetEdgeOtherNode(e, &root->net);
        n2   = fer_container_of(netn, fer_gnnp_node_t, net);

        if (n2->prev == root && n2->prev_type != root->prev_type){
            if (n2->prev_type != PATH_NONE && root->prev_type == PATH_NONE){
                ferNNRemove(nn->nn_path, &n2->nn_path);
            }else if (n2->prev_type == PATH_NONE && root->prev_type != PATH_NONE){
                ferNNAdd(nn->nn_path, &n2->nn_path);
            }
            n2->prev_type = root->prev_type;
            pathRetype(nn, n2);
        }
    }
}
