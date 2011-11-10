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

#include <fermat/gnnp.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>


static void netNodeDel(fer_net_node_t *n, void *);
static void netEdgeDel(fer_net_edge_t *e, void *);

static fer_gnnp_node_t *ferGNNPNodeNew(fer_gnnp_t *nn, const fer_vec_t *w);
static void ferGNNPNodeDel(fer_gnnp_t *nn, fer_gnnp_node_t *node);
/** Removes the longest edges incidenting with the node */
static void ferGNNPNodeRemoveLongestEdge(fer_gnnp_t *nn, fer_gnnp_node_t *node);
/** Moves the node towards input signal: w = w + e * (is - w) */
static void ferGNNPNodeMoveTowards(fer_gnnp_t *nn, fer_gnnp_node_t *node,
                                   const fer_vec_t *is, fer_real_t r);
_fer_inline int ferGNNPNodeState(const fer_gnnp_node_t *n);
_fer_inline void ferGNNPNodeSetState(fer_gnnp_t *nn, fer_gnnp_node_t *n, int state);
_fer_inline int ferGNNPNodeIsFree(const fer_gnnp_node_t *n);
_fer_inline void ferGNNPNodeSetFree(fer_gnnp_t *nn, fer_gnnp_node_t *n);
_fer_inline int ferGNNPNodeIsObst(const fer_gnnp_node_t *n);
_fer_inline void ferGNNPNodeSetObst(fer_gnnp_t *nn, fer_gnnp_node_t *n);
_fer_inline int ferGNNPNodeIsLearn(const fer_gnnp_node_t *n);

static void nearest(fer_gnnp_t *nn, const fer_vec_t *is,
                    fer_gnnp_node_t **n1,
                    fer_gnnp_node_t **n2);
static void hebbianLearning(fer_gnnp_t *nn, fer_gnnp_node_t *n1,
                                            fer_gnnp_node_t *n2);
static fer_gnnp_node_t *newNode(fer_gnnp_t *nn, fer_gnnp_node_t *wn,
                                const fer_vec_t *is);
static void move(fer_gnnp_t *nn, fer_gnnp_node_t *wn, const fer_vec_t *is);

static int findPath(fer_gnnp_t *nn, fer_list_t *path);
static int prunePath(fer_gnnp_t *nn, fer_list_t *path);


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
    p->en   = 0.0006;
    p->rmax = 4;
    p->h    = 0.1;
    p->prune_delay = 50;

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

    ferGNNPNodesInit(&nn->nodes, 50000);
    nn->s = nn->g = NULL;

    nn->tmpv = ferVecNew(nn->params.dim);

    return nn;
}

void ferGNNPDel(fer_gnnp_t *nn)
{
    ferNetDel2(nn->net, netNodeDel, (void *)nn, netEdgeDel, (void *)nn);
    ferNNDel(nn->nn);
    ferGNNPNodesDestroy(&nn->nodes);
    ferVecDel(nn->tmpv);
    FER_FREE(nn);
}

int ferGNNPFindPath(fer_gnnp_t *nn,
                    const fer_vec_t *start, const fer_vec_t *goal,
                    fer_list_t *path)
{
    const fer_vec_t *is;
    fer_gnnp_node_t *n[2];
    fer_real_t dist;
    unsigned int cb = 0U;
    unsigned long c = 0UL;
    unsigned long nextprune = 1UL;

    // Init
    nn->s = ferGNNPNodeNew(nn, start);
    ferGNNPNodeSetFree(nn, nn->s);
    nn->g = ferGNNPNodeNew(nn, goal);
    ferGNNPNodeSetFree(nn, nn->g);

    while (!nn->ops.terminate(nn, nn->ops.terminate_data)){
        cb += 1U;

        if (nextprune == c){
            if (findPath(nn, path) == 0){
                if (!prunePath(nn, path))
                    return 0;
                nextprune++;
            }else{
                nextprune += nn->params.prune_delay;
            }
        }

        // get input signal
        is = nn->ops.input_signal(nn, nn->ops.input_signal_data);

        // find two nearest nodes
        nearest(nn, is, &n[0], &n[1]);

        // competitive hebbian learning with removing edges
        hebbianLearning(nn, n[0], n[1]);

        if (ferGNNPNodeIsFree(n[0]) || ferGNNPNodeIsObst(n[0])){
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

void ferGNNPDumpSVT(const fer_gnnp_t *nn, FILE *out, const char *name)
{
    fer_list_t *list, *item;
    size_t i;
    fer_net_node_t *netn;
    fer_gnnp_node_t *n;
    fer_net_edge_t *e;
    size_t id1, id2;

    if (nn->params.dim != 2 && nn->params.dim != 3)
        return;

    fprintf(out, "--------\n");

    if (name){
        fprintf(out, "Name: %s FREE\n", name);
    }else{
        fprintf(out, "Name: FREE\n");
    }
    fprintf(out, "Point color: 0.1 0.1 0.8\n");
    fprintf(out, "Point size: 2\n");
    fprintf(out, "Points:\n");
    for (i = 0; i < nn->nodes.len; i++){
        n = ferGNNPNodesGet(&nn->nodes, i);
        if (ferGNNPNodeIsFree(n)){
            ferVecPrint(nn->params.dim, n->w, out);
            fprintf(out, "\n");
        }
    }
    fprintf(out, "--------\n");

    if (name){
        fprintf(out, "Name: %s OBST\n", name);
    }else{
        fprintf(out, "Name: OBST\n");
    }
    fprintf(out, "Point color: 0.8 0.1 0.1\n");
    fprintf(out, "Point size: 2\n");
    fprintf(out, "Points:\n");
    for (i = 0; i < nn->nodes.len; i++){
        n = ferGNNPNodesGet(&nn->nodes, i);
        if (ferGNNPNodeIsObst(n)){
            ferVecPrint(nn->params.dim, n->w, out);
            fprintf(out, "\n");
        }
    }
    fprintf(out, "--------\n");


    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Points off: 1\n");
    fprintf(out, "Edge color: 0.5 0.5 0.5\n");
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
    n->state = FER_GNNP_STATE_LEARN;
    ferGNNPNodesAdd(&nn->nodes, n);

    n->w = ferVecClone(nn->params.dim, w);

    ferNetAddNode(nn->net, &n->net);

    ferNNElInit(nn->nn, &n->nn, n->w);
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

    ferNetRemoveEdge(nn->net, maxe);
    ferNetEdgeDel(maxe);
}

static void ferGNNPNodeMoveTowards(fer_gnnp_t *nn, fer_gnnp_node_t *node,
                                   const fer_vec_t *is, fer_real_t r)
{
    ferVecSub2(nn->params.dim, nn->tmpv, is, node->w);
    ferVecScale(nn->params.dim, nn->tmpv, r);
    ferVecAdd(nn->params.dim, node->w, nn->tmpv);
    ferNNUpdate(nn->nn, &node->nn);
}

_fer_inline int ferGNNPNodeState(const fer_gnnp_node_t *n)
{
    return (n->state) & 0x3;
}

_fer_inline void ferGNNPNodeSetState(fer_gnnp_t *nn, fer_gnnp_node_t *n, int state)
{
    n->state &= ~0x3;
    n->state |= (state & 0x3);
}

_fer_inline int ferGNNPNodeIsFree(const fer_gnnp_node_t *n)
{
    return ferGNNPNodeState(n) == FER_GNNP_STATE_FREE;
}
_fer_inline void ferGNNPNodeSetFree(fer_gnnp_t *nn, fer_gnnp_node_t *n)
{
    ferGNNPNodeSetState(nn, n, FER_GNNP_STATE_FREE);
}

_fer_inline int ferGNNPNodeIsObst(const fer_gnnp_node_t *n)
{
    return ferGNNPNodeState(n) == FER_GNNP_STATE_OBST;
}
_fer_inline void ferGNNPNodeSetObst(fer_gnnp_t *nn, fer_gnnp_node_t *n)
{
    ferGNNPNodeSetState(nn, n, FER_GNNP_STATE_OBST);
}

_fer_inline int ferGNNPNodeIsLearn(const fer_gnnp_node_t *n)
{
    return ferGNNPNodeState(n) == FER_GNNP_STATE_LEARN;
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

static void hebbianLearning(fer_gnnp_t *nn, fer_gnnp_node_t *n1,
                                            fer_gnnp_node_t *n2)
{
    fer_net_edge_t *e;

    // get common edge
    e = ferNetNodeCommonEdge(&n1->net, &n2->net);

    if (e == NULL){
        // remove longest edge if adding a new edge would exceeds the limit
        if (ferNetNodeEdgesLen(&n1->net) >= nn->params.rmax)
            ferGNNPNodeRemoveLongestEdge(nn, n1);
        if (ferNetNodeEdgesLen(&n2->net) >= nn->params.rmax)
            ferGNNPNodeRemoveLongestEdge(nn, n2);

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

    return n;
}

static void move(fer_gnnp_t *nn, fer_gnnp_node_t *wn, const fer_vec_t *is)
{
    fer_list_t *list, *item;
    fer_net_edge_t *e;
    fer_net_node_t *netn;
    fer_gnnp_node_t *n;

    // move winner node
    ferGNNPNodeMoveTowards(nn, wn, is, nn->params.ew);

    // move neighbor nodes
    list = ferNetNodeEdges(&wn->net);
    FER_LIST_FOR_EACH(list, item){
        e = ferNetEdgeFromNodeList(item);
        netn = ferNetEdgeOtherNode(e, &wn->net);
        n    = fer_container_of(netn, fer_gnnp_node_t, net);

        if (ferGNNPNodeIsLearn(n))
            ferGNNPNodeMoveTowards(nn, n, is, nn->params.en);
    }
}

static int _findPath(fer_gnnp_t *nn, fer_gnnp_node_t *root,
                     fer_gnnp_node_t *goal)
{
    fer_list_t *list, *item;
    fer_net_edge_t *e;
    fer_net_node_t *netn;
    fer_gnnp_node_t *n;
    int ret;

    list = ferNetNodeEdges(&root->net);
    FER_LIST_FOR_EACH(list, item){
        e = ferNetEdgeFromNodeList(item);
        netn = ferNetEdgeOtherNode(e, &root->net);
        n    = fer_container_of(netn, fer_gnnp_node_t, net);

        if (n == goal){
            n->prev = root;
            return 0;
        }

        if (!n->prev && !ferGNNPNodeIsObst(n)){
            n->prev = root;
            ret = _findPath(nn, n, goal);
            if (ret == 0)
                return 0;
        }
    }

    return -1;
}

static int findPath(fer_gnnp_t *nn, fer_list_t *path)
{
    size_t i;
    int found;
    fer_gnnp_node_t *n;

    ferListInit(path);

    for (i = 0; i < nn->nodes.len; i++){
        nn->nodes.arr[i]->prev = NULL;
    }

    found = _findPath(nn, nn->s, nn->g);
    if (found != 0)
        return -1;

    n = nn->g;
    while (n != nn->s){
        ferListPrepend(path, &n->path);
        n = n->prev;
    }
    ferListPrepend(path, &nn->s->path);

    return 0;
}

static int _pruneEval(fer_gnnp_t *nn, fer_gnnp_node_t *n)
{
    int eval;

    if (ferGNNPNodeIsFree(n))
        return 0;
    if (ferGNNPNodeIsObst(n))
        return 1;

    eval = nn->ops.eval(nn, n->w, nn->ops.eval_data);
    if (eval){
        ferGNNPNodeSetFree(nn, n);
        return 0;
    }else{
        ferGNNPNodeSetObst(nn, n);
        return 1;
    }
}

static int _pruneBetween(fer_gnnp_t *nn,
                         fer_gnnp_node_t *n1, fer_gnnp_node_t *n2)
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

    if (dist * FER_REAL(0.5) < nn->params.h)
        return ret;

    ret |= _pruneBetween(nn, n1, n);
    ret |= _pruneBetween(nn, n, n2);
    return ret;
}

static int prunePath(fer_gnnp_t *nn, fer_list_t *path)
{
    fer_list_t *item;
    fer_gnnp_node_t *n1, *n2;
    int ret = 0;

    n2 = NULL;
    n1 = NULL;
    FER_LIST_FOR_EACH(path, item){
        n1 = n2;
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
            ret |= _pruneBetween(nn, n1, n2);
        }
    }

    //return 0;
    return ret;
}
