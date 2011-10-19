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
    if (!k->ops.input_signal_data)
        k->ops.input_signal_data = k->ops.data;
    if (!k->ops.neighborhood_data)
        k->ops.neighborhood_data = k->ops.data;
    if (!k->ops.terminate_data)
        k->ops.terminate_data = k->ops.data;
    if (!k->ops.callback_data)
        k->ops.callback_data = k->ops.data;
    if (!k->ops.input_signal){
        fprintf(stderr, "Fermat :: Kohonen :: No input_signal operator defined!\n");
        exit(-1);
    }
    if (!k->ops.neighborhood){
        fprintf(stderr, "Fermat :: Kohonen :: No neighborhood operator defined!\n");
        exit(-1);
    }

    k->params = *params;

    k->net = ferNetNew();
    k->nn  = ferNNNew(&k->params.nn);

    return k;
}

void ferKohonenDel(fer_kohonen_t *k)
{
    // TODO
    ferNNDel(k->nn);
    FER_FREE(k);
}

void ferKohonenRun(fer_kohonen_t *k)
{
    unsigned long counter;
    const fer_vec_t *is;
    fer_kohonen_node_t *win;

    counter = 0UL;
    while (!k->ops.terminate(k->ops.terminate_data)){
        // get input signal
        is = k->ops.input_signal(k->ops.input_signal_data);

        // determine winning node
        // TODO

        // update weights
        // TODO

        counter += 1UL;
        if (k->ops.callback && counter == k->ops.callback_period){
            k->ops.callback(k->ops.callback_data);
        }
    }
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
