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


static void netNodeDel(fer_net_node_t *n, void *);
static void netEdgeDel(fer_net_edge_t *e, void *);

static fer_gnnp_node_t *ferGNNPNodeNew(fer_gnnp_t *nn, const fer_vec_t *w);
static void ferGNNPNodeDel(fer_gnnp_t *nn, fer_gnnp_node_t *node);
/** Removes the longest edges incidenting with the node */
static void ferGNNPNodeRemoveLongestEdge(fer_gnnp_t *nn, fer_gnnp_node_t *node);
/** Moves the node towards input signal: w = w + e * (is - w) */
static void ferGNNPNodeMoveTowards(fer_gnnp_t *nn, fer_gnnp_node_t *node,
                                   const fer_vec_t *is, fer_real_t r);
_fer_inline void ferGNNPNodeSetFree(fer_gnnp_t *nn, fer_gnnp_node_t *n);
_fer_inline void ferGNNPNodeSetObst(fer_gnnp_t *nn, fer_gnnp_node_t *n);
static void ferGNNPNodeSetDepth(fer_gnnp_t *nn, fer_gnnp_node_t *n,
                                int depth, int fixed);

static void nearest(fer_gnnp_t *nn, const fer_vec_t *is,
                    fer_gnnp_node_t **n1,
                    fer_gnnp_node_t **n2);
static void learnDepth(fer_gnnp_t *nn, fer_gnnp_node_t *n);
static void hebbianLearning(fer_gnnp_t *nn, fer_gnnp_node_t *n1,
                                            fer_gnnp_node_t *n2);
static fer_gnnp_node_t *newNode(fer_gnnp_t *nn, fer_gnnp_node_t *wn,
                                const fer_vec_t *is);
static void move(fer_gnnp_t *nn, fer_gnnp_node_t *wn, const fer_vec_t *is);

static int findPath(fer_gnnp_t *nn, fer_list_t *path);
static int prunePath(fer_gnnp_t *nn, fer_list_t *path);

_fer_inline fer_gnnp_node_t *comp(fer_gnnp_node_t *n)
{
    fer_gnnp_node_t *c;
    c = n->comp;
    while (c && c->comp != c)
        c = c->comp;
    return c;
}

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
    p->tournament = 3;

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

    nn->rand = ferRandMTNewAuto();

    ferGNNPNodesInit(&nn->nodes, FER_GNNP_ARR_INIT_SIZE);
    nn->s = nn->g = NULL;

    ferGNNPNodesInit(&nn->nodes_set[0], FER_GNNP_ARR_INIT_SIZE);
    ferGNNPNodesInit(&nn->nodes_set[1], FER_GNNP_ARR_INIT_SIZE);
    nn->rand_set = 0;

    nn->depths_alloc = 100;
    nn->depths = FER_ALLOC_ARR(size_t, nn->depths_alloc);
    bzero(nn->depths, sizeof(size_t) * nn->depths_alloc);
    nn->max_depth = 0;

    nn->tmpv = ferVecNew(nn->params.dim);

    return nn;
}

void ferGNNPDel(fer_gnnp_t *nn)
{
    ferNetDel2(nn->net, netNodeDel, (void *)nn, netEdgeDel, (void *)nn);
    ferNNDel(nn->nn);

    ferGNNPNodesDestroy(&nn->nodes);
    ferGNNPNodesDestroy(&nn->nodes_set[0]);
    ferGNNPNodesDestroy(&nn->nodes_set[1]);

    ferRandMTDel(nn->rand);

    if (nn->depths)
        FER_FREE(nn->depths);

    ferVecDel(nn->tmpv);
    FER_FREE(nn);
}

int ferGNNPFindPath(fer_gnnp_t *nn,
                    const fer_vec_t *start, const fer_vec_t *goal,
                    fer_list_t *path)
{
    const fer_vec_t *is;
    fer_gnnp_node_t *n[2];
    fer_net_edge_t *e;
    fer_real_t dist;
    unsigned int cb = 0U;
    unsigned long c = 0UL;
    unsigned long nextprune = 1UL;

    // Init
    nn->s = ferGNNPNodeNew(nn, start);
    ferGNNPNodeSetFree(nn, nn->s);
    nn->g = ferGNNPNodeNew(nn, goal);
    ferGNNPNodeSetFree(nn, nn->g);

    e = ferNetEdgeNew();
    ferNetAddEdge(nn->net, e, &nn->s->net, &nn->g->net);

    ferListInit(path);
    ferListAppend(path, &nn->s->path);
    ferListAppend(path, &nn->g->path);
    if (!prunePath(nn, path))
        return 0;

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

        // learn graph depth
        //learnDepth(nn, n[0]);

        if (n[0]->fixed){
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

/*
void __dump(const fer_gnnp_t *nn, FILE *out, int fixed, int depth)
{
    size_t i;
    int num = 0;
    fer_gnnp_node_t *n;

    for (i = 0; i < nn->nodes.len; i++){
        n = ferGNNPNodesGet(&nn->nodes, i);
        if (n->set == fixed && n->depth == depth){
            num += 1;
        }
    }

    if (num == 0)
        return;

    fprintf(out, "--------\n");

    fprintf(out, "Name: %d %d\n", fixed, depth);
    if (fixed == 1)
        fprintf(out, "Point color: 0.1 0.1 0.8\n");
    if (fixed == 2)
        fprintf(out, "Point color: 0.8 0.1 0.1\n");
    fprintf(out, "Point size: %d\n", 1 + depth);
    fprintf(out, "Points:\n");
    for (i = 0; i < nn->nodes.len; i++){
        n = ferGNNPNodesGet(&nn->nodes, i);
        if (n->set == fixed && n->depth == depth){
            ferVecPrint(nn->params.dim, n->w, out);
            fprintf(out, "\n");
        }
    }
    fprintf(out, "--------\n");
}
*/

void __dump(const fer_gnnp_t *nn, FILE *out, int fixed)
{
    size_t i;
    int num = 0;
    fer_gnnp_node_t *n;

    for (i = 0; i < nn->nodes.len; i++){
        n = ferGNNPNodesGet(&nn->nodes, i);
        if (n->set == fixed){
            num += 1;
        }
    }

    if (num == 0)
        return;

    fprintf(out, "--------\n");

    fprintf(out, "Name: %d\n", fixed);
    if (fixed == 1)
        fprintf(out, "Point color: 0.1 0.1 0.8\n");
    if (fixed == 2)
        fprintf(out, "Point color: 0.8 0.1 0.1\n");
    fprintf(out, "Point size: 1\n");
    fprintf(out, "Points:\n");
    for (i = 0; i < nn->nodes.len; i++){
        n = ferGNNPNodesGet(&nn->nodes, i);
        if (n->set == fixed){
            ferVecPrint(nn->params.dim, n->w, out);
            fprintf(out, "\n");
        }
    }
    fprintf(out, "--------\n");
}

void __dumpRepr2(const fer_gnnp_t *nn, FILE *out, fer_gnnp_node_t *r,
                 fer_real_t b)
{
    size_t i;
    int num = 0;
    fer_gnnp_node_t *n;

    for (i = 0; i < nn->nodes.len; i++){
        n = ferGNNPNodesGet(&nn->nodes, i);
        if (comp(n) == r){
            num += 1;
        }
    }

    if (num == 0)
        return;

    fprintf(out, "--------\n");

    fprintf(out, "Name: Repr %f\n", b);
    fprintf(out, "Point color: 0.1 0.1 %f\n", b);
    fprintf(out, "Point size: 2\n");
    fprintf(out, "Points:\n");
    for (i = 0; i < nn->nodes.len; i++){
        n = ferGNNPNodesGet(&nn->nodes, i);
        if (comp(n) == r){
            ferVecPrint(nn->params.dim, n->w, out);
            fprintf(out, "\n");
        }
    }
}

void __dumpRepr(const fer_gnnp_t *nn, FILE *out)
{
    size_t i;
    fer_gnnp_node_t *n;
    fer_real_t b = 0.1;

    for (i = 0; i < nn->nodes.len; i++){
        n = ferGNNPNodesGet(&nn->nodes, i);
        if (n->comp == n){
            __dumpRepr2(nn, out, n, b);
            b += 0.1;
            if (b > 1.)
                b = 0.1;
        }
    }
}

void ferGNNPDumpSVT(const fer_gnnp_t *nn, FILE *out, const char *name)
{
    //fer_list_t *list, *item;
    //size_t i;
    /*
    fer_net_node_t *netn;
    fer_gnnp_node_t *n;
    fer_net_edge_t *e;
    size_t id1, id2;
    */

    if (nn->params.dim != 2 && nn->params.dim != 3)
        return;

    if (name)
        fprintf(out, "Name: %s\n", name);

    /*
    for (i = 0; i < 100; i++){
        __dump(nn, out, 1, i);
    }
    for (i = 0; i < 100; i++){
        __dump(nn, out, 2, i);
    }
    */
    __dump(nn, out, 1);
    __dump(nn, out, 2);
    __dumpRepr(nn, out);
    fprintf(out, "--------\n");

    /*
    if (name)
        fprintf(out, "Name: net\n");

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
    */
}

const fer_gnnp_node_t *ferGNNPRandNode(fer_gnnp_t *nn)
{
    int id, ndepth, mdepth;
    unsigned int i;
    fer_gnnp_node_t *n = NULL, *m;

    ndepth = -INT_MAX;
    for (i = 0; i < nn->params.tournament; i++){
        id  = ferRandMT(nn->rand, 0, nn->nodes_set[nn->rand_set].len);
        id  = (id < nn->nodes_set[nn->rand_set].len ? id : nn->nodes_set[nn->rand_set].len - 1);
        m   = ferGNNPNodesGet(&nn->nodes_set[nn->rand_set], id);
        mdepth = (nn->rand_set == 0 ? -m->depth : m->depth);
        if (mdepth > ndepth){
            ndepth = mdepth;
            n      = m;
        }
    }

    nn->rand_set ^= 1;

    return n;
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
    n->depth = -1;
    n->set = -1;
    n->arrid = -1;
    ferGNNPNodesAdd(&nn->nodes, n);

    n->w = ferVecClone(nn->params.dim, w);

    ferNetAddNode(nn->net, &n->net);

    ferNNElInit(nn->nn, &n->nn, n->w);
    ferNNAdd(nn->nn, &n->nn);

    n->comp = NULL;

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
}

_fer_inline void ferGNNPNodeSetFree(fer_gnnp_t *nn, fer_gnnp_node_t *n)
{
    n->fixed = 1;
    ferGNNPNodeSetDepth(nn, n, 0, 1);
}
_fer_inline void ferGNNPNodeSetObst(fer_gnnp_t *nn, fer_gnnp_node_t *n)
{
    n->fixed = 2;
    ferGNNPNodeSetDepth(nn, n, 0, 2);
}

static void ferGNNPNodeSetDepth(fer_gnnp_t *nn, fer_gnnp_node_t *n,
                                int depth, int fixed)
{
    size_t i;

    // remove from depth counter
    if (n->depth != -1 && n->set == 2){
        nn->depths[n->depth]--;

        if (n->depth == nn->max_depth
                && nn->depths[n->depth] == 0
                && n->depth != 0){
            for (i = n->depth - 1; i >= 0; i--){
                if (nn->depths[i] > 0){
                    nn->max_depth = i;
                    break;
                }
            }
        }
    }

    // remove from old array first
    if (n->arrid != -1){
        if ((n->set == 1 && fixed != 1)
                || (n->set == 2 && (fixed != 2 || depth <= 1))){
            ferGNNPNodesRemove(&nn->nodes_set[n->set - 1], n->arrid);
            ferGNNPNodesGet(&nn->nodes_set[n->set - 1], n->arrid)->arrid = n->arrid;
            n->arrid = -1;
        }
    }

    // assign values to the node
    n->depth = depth;
    n->set   = fixed;

    // put node into correct array
    if (n->arrid == -1 && (n->set == 1 || (n->set == 2 && n->depth > 1))){
        n->arrid = ferGNNPNodesAdd(&nn->nodes_set[n->set - 1], n);
    }

    // update depth counter
    if (n->set == 2){
        if (nn->depths_alloc <= n->depth){
            i = nn->depths_alloc;
            nn->depths_alloc *= 2;
            nn->depths = FER_REALLOC_ARR(nn->depths, size_t, nn->depths_alloc);
            for (; i < nn->depths_alloc; i++)
                nn->depths[i] = 0;
        }
        nn->depths[n->depth]++;
        if (n->depth > nn->max_depth)
            nn->max_depth = n->depth;
    }
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

static void learnDepth(fer_gnnp_t *nn, fer_gnnp_node_t *n)
{
    fer_list_t *list, *item;
    fer_net_edge_t *e;
    fer_net_node_t *netn;
    fer_gnnp_node_t *o;
    int min[2], num[2], set;

    if (n->fixed)
        return;

    min[0] = min[1] = INT_MAX - 1;
    num[0] = num[1] = 0;
    list = ferNetNodeEdges(&n->net);
    FER_LIST_FOR_EACH(list, item){
        e = ferNetEdgeFromNodeList(item);
        netn = ferNetEdgeOtherNode(e, &n->net);
        o    = fer_container_of(netn, fer_gnnp_node_t, net);

        num[o->set - 1]++;
        if (o->depth < min[o->set - 1])
            min[o->set - 1] = o->depth;
    }

    set = n->set;
    if (num[0] > num[1]){
        set = 1;
    }else if (num[1] > num[0]){
        set = 2;
    }

    ferGNNPNodeSetDepth(nn, n, min[set - 1] + 1, set);
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

    ferGNNPNodeSetDepth(nn, n, 1, wn->fixed);

    return n;
}

static void move(fer_gnnp_t *nn, fer_gnnp_node_t *wn, const fer_vec_t *is)
{
    fer_list_t *list, *item;
    fer_net_edge_t *e;
    fer_net_node_t *netn;
    fer_gnnp_node_t *n;

    // move winner node
    if (!wn->fixed)
        ferGNNPNodeMoveTowards(nn, wn, is, nn->params.ew);

    // move neighbor nodes
    list = ferNetNodeEdges(&wn->net);
    FER_LIST_FOR_EACH(list, item){
        e = ferNetEdgeFromNodeList(item);
        netn = ferNetEdgeOtherNode(e, &wn->net);
        n    = fer_container_of(netn, fer_gnnp_node_t, net);

        if (!n->fixed)
            ferGNNPNodeMoveTowards(nn, n, is, nn->params.en);
    }
}



struct sort_desc_t {
    fer_gnnp_node_t *root;
    fer_gnnp_t *nn;
};

static int sortDescendantsLT(fer_list_t *l1, fer_list_t *l2, void *data)
{
    fer_net_edge_t *e;
    fer_net_node_t *netn;
    struct sort_desc_t *d = (struct sort_desc_t *)data;
    fer_gnnp_node_t *n1, *n2;
    int v1, v2;

    e    = ferNetEdgeFromNodeList(l1);
    netn = ferNetEdgeOtherNode(e, &d->root->net);
    n1   = fer_container_of(netn, fer_gnnp_node_t, net);
    e    = ferNetEdgeFromNodeList(l2);
    netn = ferNetEdgeOtherNode(e, &d->root->net);
    n2   = fer_container_of(netn, fer_gnnp_node_t, net);

    if (!n1->prev && n2->prev)
        return 1;

    v1 = (n1->set == 2 ? n1->depth : d->nn->max_depth - n1->depth);
    v2 = (n2->set == 2 ? n2->depth : d->nn->max_depth - n2->depth);

    return v1 > v2 && !n1->prev;
}

static void sortDescendants(fer_gnnp_t *nn, fer_gnnp_node_t *n)
{
    struct sort_desc_t desc;
    desc.root = n;
    desc.nn   = nn;
    ferInsertSortList(&n->net.edges, sortDescendantsLT, (void *)&desc);
}

static int _findPathDFS(fer_gnnp_t *nn, fer_gnnp_node_t *root,
                        fer_gnnp_node_t *goal, int sort)
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

        if (!n->prev && (n->set == 1 || (n->set == 2 && n->depth > 0))){
            n->prev = root;
            if (sort)
                sortDescendants(nn, n);
            ret = _findPathDFS(nn, n, goal, sort);
            if (ret == 0)
                return 0;
        }
    }

    return -1;
}
static int findPathDFS(fer_gnnp_t *nn, fer_list_t *path, int sort)
{
    size_t i;
    int found;
    fer_gnnp_node_t *n;

    ferListInit(path);

    for (i = 0; i < nn->nodes.len; i++){
        nn->nodes.arr[i]->prev = NULL;
    }

    if (sort)
        sortDescendants(nn, nn->s);
    found = _findPathDFS(nn, nn->s, nn->g, sort);
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

        if (!ferDijNodeClosed(&n->dij) && (n->set == 1 || n->depth > 0)){
            dist = ferVecDist(nn->params.dim, root->w, n->w);
            ferDijNodeAdd(&n->dij, expand, dist);
        }
    }
}

static int findPathDij(fer_gnnp_t *nn, fer_list_t *path)
{
    fer_dij_ops_t ops;
    fer_dij_t *dij;
    size_t i;
    int found;
    fer_gnnp_node_t *n;
    fer_dij_node_t *dn;

    ferListInit(path);

    for (i = 0; i < nn->nodes.len; i++){
        ferDijNodeInit(&nn->nodes.arr[i]->dij);
        nn->nodes.arr[i]->prev = NULL;
    }

    // initialize operators
    ferDijOpsInit(&ops);
    ops.expand = findPathDijExpand;
    ops.data   = (void *)nn;

    // create dijkstra algorithm
    dij = ferDijNew(&ops);

    // run dijkstra
    found = ferDijRun(dij, &nn->s->dij, &nn->g->dij);
    if (found != 0)
        return -1;

    dn = &nn->g->dij;
    while (dn != &nn->s->dij){
        n = fer_container_of(dn, fer_gnnp_node_t, dij);
        ferListPrepend(path, &n->path);

        dn = dn->prev;
    }
    ferListPrepend(path, &nn->s->path);

    return 0;
}

static int findPath(fer_gnnp_t *nn, fer_list_t *path)
{
    return findPathDij(nn, path);
    return findPathDFS(nn, path, 1);
}

static int _pruneEval(fer_gnnp_t *nn, fer_gnnp_node_t *n)
{
    int eval;

    if (n->fixed == 1){
        return 0;
    }else if (n->fixed == 2){
        return 1;
    }

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

static void _pruneComp(fer_gnnp_t *nn, fer_list_t *path)
{
    fer_list_t *item;
    fer_gnnp_node_t *n, *p;

    n = NULL;
    FER_LIST_FOR_EACH(path, item){
        p = n;
        n = FER_LIST_ENTRY(item, fer_gnnp_node_t, path);

        if (n->fixed == 1){
            if (!p || p->fixed != 1){
                if (!n->comp){
                    n->comp = n;
                }else{
                    n->comp = comp(n->comp);
                }
            }else{
                if (n->comp){
                    comp(n)->comp = p->comp;
                }
                n->comp = p->comp;
            }
        }
    }
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
        _pruneComp(nn, path);
    }

    //return 0;
    return ret;
}
