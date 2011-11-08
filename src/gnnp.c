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

static void nearest(fer_gnnp_t *nn, const fer_vec_t *is,
                    fer_gnnp_node_t **n1,
                    fer_gnnp_node_t **n2);
static void hebbianLearning(fer_gnnp_t *nn, fer_gnnp_node_t *n1,
                                            fer_gnnp_node_t *n2);
static fer_gnnp_node_t *newNode(fer_gnnp_t *nn, fer_gnnp_node_t *wn,
                                const fer_vec_t *is);
static void move(fer_gnnp_t *nn, fer_gnnp_node_t *wn, const fer_vec_t *is);


#define OPS_DATA(name) \
    if (!nn->ops.name ## _data) \
        nn->ops.name ## _data = nn->ops.data;

#define OPS_CHECK(name) \
    if (!nn->ops.name){ \
        fprintf(stderr, "Fermat :: GNNP :: No " #name "callback set.\n"); \
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
    OPS_DATA(callback)

    nn->net = ferNetNew();
    nn->nn  = ferNNNew(&nn->params.nn);

    nn->nodes_alloc = 100000;
    nn->nodes_len   = 0;
    nn->nodes       = FER_ALLOC_ARR(fer_gnnp_node_t, nn->nodes_alloc);
    nn->s = nn->g = NULL;

    nn->tmpv = ferVecNew(nn->params.dim);

    return nn;
}

void ferGNNPDel(fer_gnnp_t *nn)
{
    ferNetDel2(nn->net, netNodeDel, (void *)nn, netEdgeDel, (void *)nn);
    ferNNDel(nn->nn);
    FER_FREE(nn->nodes);
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
    unsigned int counter = 0U;

    // Init
    nn->s = ferGNNPNodeNew(nn, start);
    nn->s->state = FER_GNNP_STATE_FREE;
    nn->g = ferGNNPNodeNew(nn, goal);
    nn->g->state = FER_GNNP_STATE_FREE;

    while (!nn->ops.terminate(nn, nn->ops.terminate_data)){
        counter += 1U;

        // get input signal
        is = nn->ops.input_signal(nn, nn->ops.input_signal_data);

        // find two nearest nodes
        nearest(nn, is, &n[0], &n[1]);

        // competitive hebbian learning with removing edges
        hebbianLearning(nn, n[0], n[1]);

        if (n[0]->state == FER_GNNP_STATE_FREE
                || n[0]->state == FER_GNNP_STATE_OBST){
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

        if (nn->ops.callback && nn->ops.callback_period == counter){
            nn->ops.callback(nn, nn->ops.callback_data);
            counter = 0L;
        }
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

    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Points:\n");
    for (i = 0; i < nn->nodes_len; i++){
        nn->nodes[i]._id = i;
        ferVecPrint(nn->params.dim, nn->nodes[i].w, out);
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

    if (nn->nodes_len == nn->nodes_alloc){
        nn->nodes_alloc *= 2;
        nn->nodes = FER_REALLOC_ARR(nn->nodes, fer_gnnp_node_t, nn->nodes_alloc);
    }

    n = &nn->nodes[nn->nodes_len];
    nn->nodes_len++;

    n->w = ferVecClone(nn->params.dim, w);
    n->state = FER_GNNP_STATE_UNFIX;

    ferNetAddNode(nn->net, &n->net);

    ferNNElInit(nn->nn, &n->nn, n->w);
    ferNNAdd(nn->nn, &n->nn);

    return n;
}

static void ferGNNPNodeDel(fer_gnnp_t *nn, fer_gnnp_node_t *node)
{
    size_t i;

    ferNetRemoveNode(nn->net, &node->net);
    ferNNRemove(nn->nn, &node->nn);
    ferVecDel(node->w);

    i = (unsigned long)node - (unsigned long)nn->nodes;
    i /= sizeof(fer_gnnp_node_t);
    nn->nodes[i] = nn->nodes[nn->nodes_len - 1];
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

        if (n->state == FER_GNNP_STATE_UNFIX)
            ferGNNPNodeMoveTowards(nn, n, is, nn->params.en);
    }
}
