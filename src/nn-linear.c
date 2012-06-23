/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <boruvka/nn-linear.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>
#include <boruvka/nn.h>



static bor_real_t distL2Norm(int d, const bor_vec_t *v1,
                                    const bor_vec_t *v2, void *data);

static void bubbleUp(bor_nn_linear_el_t **els, size_t len);

void borNNLinearParamsInit(bor_nn_linear_params_t *p)
{
    p->dim = 2;
    p->dist = distL2Norm;
    p->dist_data = NULL;
}




bor_nn_linear_t *borNNLinearNew(const bor_nn_linear_params_t *params)
{
    bor_nn_linear_t *nn;

    nn = BOR_ALLOC(bor_nn_linear_t);
    nn->type = BOR_NN_LINEAR;
    borListInit(&nn->list);
    nn->params = *params;

    return nn;
}

void borNNLinearDel(bor_nn_linear_t *nn)
{
    borListInit(&nn->list);
    BOR_FREE(nn);
}

size_t borNNLinearNearest(const bor_nn_linear_t *nn, const bor_vec_t *p, size_t num,
                          bor_nn_linear_el_t **els)
{
    bor_list_t *item;
    bor_real_t *dists, dist;
    bor_nn_linear_el_t *el;
    size_t len;

    if (num == 0)
        return 0;

    dists = BOR_ALLOC_ARR(bor_real_t, num);
    len = 0;

    BOR_LIST_FOR_EACH(&nn->list, item){
        el = BOR_LIST_ENTRY(item, bor_nn_linear_el_t, list);
        dist = nn->params.dist(nn->params.dim, p, el->p, nn->params.dist_data);

        if (len < num){
            el->dist = dist;
            els[len] = el;
            len++;

            bubbleUp(els, len);
        }else if (dist < els[len - 1]->dist){
            el->dist = dist;
            els[len - 1]   = el;

            bubbleUp(els, len);
        }
    }

    BOR_FREE(dists);
    return len;
}



static bor_real_t distL2Norm(int d, const bor_vec_t *v1,
                                    const bor_vec_t *v2, void *data)
{
    return borVecDist2(d, v1, v2);
}

static void bubbleUp(bor_nn_linear_el_t **els, size_t len)
{
    size_t i;
    bor_nn_linear_el_t *tmpn;

    // don't worry, len can never be zero
    for (i = len - 1; i > 0; i--){
        if (els[i]->dist < els[i - 1]->dist){
            BOR_SWAP(els[i], els[i - 1], tmpn);
        }else{
            break;
        }
    }
}
