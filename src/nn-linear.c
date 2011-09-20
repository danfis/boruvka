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

#include <fermat/nn-linear.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>
#include <fermat/nn.h>



static fer_real_t distL2Norm(int d, const fer_vec_t *v1,
                                    const fer_vec_t *v2, void *data);

static void bubbleUp(fer_nn_linear_el_t **els, size_t len);

void ferNNLinearParamsInit(fer_nn_linear_params_t *p)
{
    p->dim = 2;
    p->dist = distL2Norm;
    p->dist_data = NULL;
}




fer_nn_linear_t *ferNNLinearNew(const fer_nn_linear_params_t *params)
{
    fer_nn_linear_t *nn;

    nn = FER_ALLOC(fer_nn_linear_t);
    nn->type = FER_NN_LINEAR;
    ferListInit(&nn->list);
    nn->params = *params;

    return nn;
}

void ferNNLinearDel(fer_nn_linear_t *nn)
{
    ferListInit(&nn->list);
    FER_FREE(nn);
}

size_t ferNNLinearNearest(const fer_nn_linear_t *nn, const fer_vec_t *p, size_t num,
                          fer_nn_linear_el_t **els)
{
    fer_list_t *item;
    fer_real_t *dists, dist;
    fer_nn_linear_el_t *el;
    size_t len;

    if (num == 0)
        return 0;

    dists = FER_ALLOC_ARR(fer_real_t, num);
    len = 0;

    FER_LIST_FOR_EACH(&nn->list, item){
        el = FER_LIST_ENTRY(item, fer_nn_linear_el_t, list);
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

    FER_FREE(dists);
    return len;
}



static fer_real_t distL2Norm(int d, const fer_vec_t *v1,
                                    const fer_vec_t *v2, void *data)
{
    return ferVecDist2(d, v1, v2);
}

static void bubbleUp(fer_nn_linear_el_t **els, size_t len)
{
    size_t i;
    fer_nn_linear_el_t *tmpn;

    // don't worry, len can never be zero
    for (i = len - 1; i > 0; i--){
        if (els[i]->dist < els[i - 1]->dist){
            FER_SWAP(els[i], els[i - 1], tmpn);
        }else{
            break;
        }
    }
}
