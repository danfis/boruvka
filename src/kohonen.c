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

#include <fermat/kohonen.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Returns nearest node to the input signal */
static fer_kohonen_node_t *nearest(fer_kohonen_t *k, const fer_vec_t *is);
/** Updates weights of nodes in the network */
static void updateWeights(fer_kohonen_t *k, const fer_vec_t *is,
                                            fer_kohonen_node_t *wn);
/** n->w = n->w + rate * (w - n->w) */
static void ferKohonenNodeMoveTowards(fer_kohonen_t *k,
                                      fer_kohonen_node_t *n,
                                      const fer_vec_t *w,
                                      fer_real_t rate);

static void netNodeDel(fer_net_node_t *n, void *);
static void netEdgeDel(fer_net_edge_t *e, void *);

#define OPS_DATA(name) \
    if (!k->ops.name ## _data) \
        k->ops.name ## _data = k->ops.data;

#define OPS_CHECK(name) \
    if (!k->ops.name){ \
        fprintf(stderr, "Fermat :: Kohonen :: No " #name " callback set.\n"); \
        exit(-1); \
    }

#define OPS_CHECK_DATA(name) \
    OPS_DATA(name) \
    OPS_CHECK(name)


void ferKohonenOpsInit(fer_kohonen_ops_t *ops)
{
    bzero(ops, sizeof(*ops));
    ops->callback_period = 100;
}


void ferKohonenParamsInit(fer_kohonen_params_t *p)
{
    p->dim = 2;
    p->learn_rate = 0.1;
    ferNNParamsInit(&p->nn);
}

fer_kohonen_t *ferKohonenNew(const fer_kohonen_ops_t *ops,
                             const fer_kohonen_params_t *params)
{
    fer_kohonen_t *k;

    k = FER_ALLOC(fer_kohonen_t);

    k->ops = *ops;
    OPS_CHECK_DATA(input_signal)
    OPS_CHECK_DATA(neighborhood)
    OPS_CHECK_DATA(terminate)
    OPS_DATA(callback)

    k->params = *params;

    k->net = ferNetNew();
    k->nn  = ferNNNew(&k->params.nn);

    k->tmpv = ferVecNew(k->params.dim);

    return k;
}

void ferKohonenDel(fer_kohonen_t *k)
{
    ferNetDel2(k->net, netNodeDel, (void *)k, netEdgeDel, (void *)k);
    ferNNDel(k->nn);
    ferVecDel(k->tmpv);
    FER_FREE(k);
}

static void ferKohonenReset(fer_kohonen_t *k)
{
    fer_list_t *list, *item;
    fer_net_node_t *nn;
    fer_kohonen_node_t *n;

    k->update = 1;

    list = ferNetNodes(k->net);
    FER_LIST_FOR_EACH(list, item){
        nn = FER_LIST_ENTRY(item, fer_net_node_t, list);
        n  = fer_container_of(nn, fer_kohonen_node_t, net);
        n->update = 0;
    }
}

void ferKohonenRun(fer_kohonen_t *k)
{
    unsigned long counter;
    const fer_vec_t *is;
    fer_kohonen_node_t *win;

    if (ferNetNodesLen(k->net) == 0){
        fprintf(stderr, "Fermat :: Kohonen :: No nodes in the map!\n");
        return;
    }

    ferKohonenReset(k);

    counter = 0UL;
    while (!k->ops.terminate(k, k->ops.terminate_data)){
        // get input signal
        is = k->ops.input_signal(k, k->ops.input_signal_data);

        // determine winning node
        win = nearest(k, is);

        // update weights
        updateWeights(k, is, win);

        counter += 1UL;
        if (k->ops.callback && counter == k->ops.callback_period){
            k->ops.callback(k, k->ops.callback_data);
            counter = 0UL;
        }
    }
}

void ferKohonenDumpSVT(const fer_kohonen_t *k, FILE *out, const char *name)
{
    fer_list_t *list, *item;
    fer_net_edge_t *e;
    fer_net_node_t *netn;
    fer_kohonen_node_t *n;
    int i;

    fprintf(out, "-----\n");

    if (name){
        fprintf(out, "Name: %s\n", name);
    }

    fprintf(out, "Points:\n");
    list = ferNetNodes(k->net);
    i = 0;
    FER_LIST_FOR_EACH(list, item){
        netn = FER_LIST_ENTRY(item, fer_net_node_t, list);
        n    = fer_container_of(netn, fer_kohonen_node_t, net);

        n->_id = i;
        ferVecPrint(k->params.dim, n->w, out);
        fprintf(out, "\n");

        i++;
    }

    fprintf(out, "Edges:\n");
    list = ferNetEdges(k->net);
    FER_LIST_FOR_EACH(list, item){
        e = FER_LIST_ENTRY(item, fer_net_edge_t, list);

        netn = ferNetEdgeNode(e, 0);
        n    = fer_container_of(netn, fer_kohonen_node_t, net);
        fprintf(out, "%d ", n->_id);

        netn = ferNetEdgeNode(e, 1);
        n    = fer_container_of(netn, fer_kohonen_node_t, net);
        fprintf(out, "%d\n", n->_id);
    }

    fprintf(out, "-----\n");
}

fer_kohonen_node_t *ferKohonenNodeNew(fer_kohonen_t *k, const fer_vec_t *init)
{
    fer_kohonen_node_t *n;

    n = FER_ALLOC(fer_kohonen_node_t);
    n->w = ferVecNew(k->params.dim);
    if (init)
        ferVecCopy(k->params.dim, n->w, init);

    ferNetAddNode(k->net, &n->net);
    ferNNElInit(k->nn, &n->nn, n->w);
    ferNNAdd(k->nn, &n->nn);
    return n;
}

void ferKohonenNodeDel(fer_kohonen_t *k, fer_kohonen_node_t *n)
{
    FER_FREE(n->w);
    ferNetRemoveNode(k->net, &n->net);
    ferNNRemove(k->nn, &n->nn);
    FER_FREE(n);
}

void ferKohonenNodeConnect(fer_kohonen_t *k,
                           fer_kohonen_node_t *n1,
                           fer_kohonen_node_t *n2)
{
    fer_net_edge_t *e;
    e = ferNetEdgeNew();
    ferNetAddEdge(k->net, e, &n1->net, &n2->net);
}


static fer_kohonen_node_t *nearest(fer_kohonen_t *k, const fer_vec_t *is)
{
    fer_nn_el_t *el;
    fer_kohonen_node_t *n;

    ferNNNearest(k->nn, is, 1, &el);
    n = fer_container_of(el, fer_kohonen_node_t, nn);
    return n;
}

static void updateWeightsUpdateFifo(fer_kohonen_node_t *n,
                                    fer_list_t *fifo,
                                    unsigned int update,
                                    unsigned int depth)
{
    fer_list_t *list, *item;
    fer_net_edge_t *e;
    fer_net_node_t *no;
    fer_kohonen_node_t *o;

    list = ferNetNodeEdges(&n->net);
    FER_LIST_FOR_EACH(list, item){
        e = ferNetEdgeFromNodeList(item);
        no = ferNetEdgeOtherNode(e, &n->net);
        o  = fer_container_of(no, fer_kohonen_node_t, net);

        if (o->update != update){
            ferListAppend(fifo, &o->fifo);
            o->update = update;
            o->depth  = depth;
        }
    }
}

static void updateWeights(fer_kohonen_t *k, const fer_vec_t *is,
                                            fer_kohonen_node_t *wn)
{
    fer_list_t fifo, *item;
    fer_kohonen_node_t *n;
    fer_real_t rate;
    int neigh;

    // update winner's weight
    ferKohonenNodeMoveTowards(k, wn, is, k->params.learn_rate);

    // initialize fifo list
    ferListInit(&fifo);

    wn->update = k->update;
    wn->depth  = 0;
    updateWeightsUpdateFifo(wn, &fifo, k->update, wn->depth);
    while (!ferListEmpty(&fifo)){
        item = ferListNext(&fifo);
        ferListDel(item);
        n = FER_LIST_ENTRY(item, fer_kohonen_node_t, fifo);

        neigh = k->ops.neighborhood(k, wn, n, n->depth, &rate,
                                    k->ops.neighborhood_data);

        if (neigh){
            // move node towards input signal
            ferKohonenNodeMoveTowards(k, n, is, k->params.learn_rate * rate);

            // add n's children to fifo
            updateWeightsUpdateFifo(n, &fifo, k->update, n->depth + 1);
        }
    }

    k->update = k->update + 1u;
}

static void ferKohonenNodeMoveTowards(fer_kohonen_t *k,
                                      fer_kohonen_node_t *n,
                                      const fer_vec_t *w,
                                      fer_real_t rate)
{
    if (!ferKohonenNodeFixed(n)){
        ferVecSub2(k->params.dim, k->tmpv, w, n->w);
        ferVecScale(k->params.dim, k->tmpv, rate);
        ferVecAdd(k->params.dim, n->w, k->tmpv);
        ferNNUpdate(k->nn, &n->nn);
    }
}


static void netNodeDel(fer_net_node_t *n, void *_k)
{
    fer_kohonen_t *k = (fer_kohonen_t *)_k;
    fer_kohonen_node_t *node = fer_container_of(n, fer_kohonen_node_t, net);
    ferKohonenNodeDel(k, node);
}

static void netEdgeDel(fer_net_edge_t *e, void *_k)
{
    ferNetEdgeDel(e);
}
