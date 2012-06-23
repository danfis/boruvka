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

#include <boruvka/nnbp.h>
#include <boruvka/rand-mt.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

/** Sigmoid function: 1/(1 + exp(-lambda * x)) */
static bor_real_t sigmoid(bor_real_t x, bor_real_t lambda);
/** Sigmoid function with output <0, 1> */
static bor_real_t sigmoid01(bor_real_t x, bor_real_t lambda);
/** Computes new weights in network */
static void newWeights(bor_nnbp_t *nn, const bor_vec_t *out);

void ferNNBPParamsInit(bor_nnbp_params_t *params)
{
    params->alpha  = BOR_REAL(0.7);
    params->eta    = BOR_REAL(0.3);
    params->lambda = BOR_REAL(1.);

    params->func = BOR_NNBP_SIGMOID;
}

bor_nnbp_t *ferNNBPNew(const bor_nnbp_params_t *params)
{
    bor_nnbp_t *nn;
    bor_rand_mt_t *rnd;
    size_t i, j, k, max;

    nn = BOR_ALLOC(bor_nnbp_t);

    nn->layers_num = params->layers_num;
    nn->eta    = params->eta;
    nn->alpha  = params->alpha;
    nn->lambda = params->lambda;
    nn->func   = params->func;

    //rnd = ferRandMTNewAuto();
    rnd = ferRandMTNew(1111);

    max = 0;
    nn->layers = BOR_ALLOC_ARR(bor_nnbp_layer_t, nn->layers_num);
    for (i = 0; i < nn->layers_num; i++){
        max = BOR_MAX(params->layer_size[i], max);

        nn->layers[i].size = params->layer_size[i];
        nn->layers[i].x    = ferVecNew(nn->layers[i].size + 1);
        nn->layers[i].w    = NULL;

        ferVecSet(nn->layers[i].x, 0, -1.);

        if (i > 0){
            nn->layers[i].w = BOR_ALLOC_ARR(bor_vec_t *, nn->layers[i].size);
            nn->layers[i].prevw = BOR_ALLOC_ARR(bor_vec_t *, nn->layers[i].size);
            for (j = 0; j < nn->layers[i].size; j++){
                nn->layers[i].w[j] = ferVecNew(nn->layers[i - 1].size + 1);
                nn->layers[i].prevw[j] = ferVecNew(nn->layers[i - 1].size + 1);

                for (k = 0; k < nn->layers[i - 1].size + 1; k++){
                    // TODO: parameter eps
                    ferVecSet(nn->layers[i].w[j], k, ferRandMT(rnd, -0.3, 0.3));
                }
                ferVecSetZero(nn->layers[i - 1].size + 1, nn->layers[i].prevw[j]);
            }
        }
    }

    ferRandMTDel(rnd);


    nn->delta[0] = ferVecNew(max);
    nn->delta[1] = ferVecNew(max);
    nn->tmp      = ferVecNew(max + 1);

    return nn;
}

void ferNNBPDel(bor_nnbp_t *nn)
{
    size_t i, j;

    for (i = 0; i < nn->layers_num; i++){
        ferVecDel(nn->layers[i].x);
        if (i > 0){
            for (j = 0; j < nn->layers[i].size; j++){
                ferVecDel(nn->layers[i].w[j]);
                ferVecDel(nn->layers[i].prevw[j]);
            }
            BOR_FREE(nn->layers[i].w);
            BOR_FREE(nn->layers[i].prevw);
        }
    }
    BOR_FREE(nn->layers);

    ferVecDel(nn->delta[0]);
    ferVecDel(nn->delta[1]);
    ferVecDel(nn->tmp);

    BOR_FREE(nn);
}

const bor_vec_t *ferNNBPFeed(bor_nnbp_t *nn, const bor_vec_t *in)
{
    bor_real_t sum, val;
    size_t i, n;

    // copy input layer
    for (i = 0; i < nn->layers[0].size; i++){
        ferVecSet(nn->layers[0].x, i + 1, ferVecGet(in, i));
    }

    // for each neuron in each layer
    for (i = 1; i < nn->layers_num; i++){
        for (n = 0; n < nn->layers[i].size; n++){
            sum = ferVecDot(nn->layers[i - 1].size + 1,
                            nn->layers[i].w[n], nn->layers[i - 1].x);
            val = BOR_ZERO;
            if (nn->func == BOR_NNBP_SIGMOID){
                val = sigmoid(sum, nn->lambda);
            }else if (nn->func == BOR_NNBP_SIGMOID01){
                val = sigmoid01(sum, nn->lambda);
            }
            ferVecSet(nn->layers[i].x, n + 1, val);
        }
    }

    return ferVecOff(nn->layers[nn->layers_num - 1].x, 1);
}

bor_real_t ferNNBPErr(const bor_nnbp_t *nn, const bor_vec_t *out)
{
    bor_nnbp_layer_t *lr = &nn->layers[nn->layers_num - 1];
    ferVecSub2(lr->size, nn->tmp, ferVecOff(lr->x, 1), out);
    return ferVecLen2(lr->size, nn->tmp);
}

void ferNNBPLearn(bor_nnbp_t *nn, const bor_vec_t *in, const bor_vec_t *out)
{
    // feed neural network with the input
    ferNNBPFeed(nn, in);
    // compute new weights
    newWeights(nn, out);
}

static void makeDelta(bor_nnbp_t *nn, size_t layer,
                      const bor_vec_t *out,
                      const bor_vec_t *delta,
                      bor_vec_t *delta_out)
{
    bor_nnbp_layer_t *lr, *lr2;
    bor_real_t err;
    size_t i, j;

    lr = &nn->layers[layer];

    if (layer == nn->layers_num - 1){
        // difference from desired output
        ferVecSub2(lr->size, nn->tmp, ferVecOff(lr->x, 1), out);
        //ferVecSub2(lr->size, nn->tmp, out, ferVecOff(lr->x, 1));

        // delta_out = x.*(1 - x).*err
        ferVecScale2(lr->size, delta_out, ferVecOff(lr->x, 1), -BOR_ONE);
        ferVecAddConst(lr->size, delta_out, BOR_ONE);
        ferVecMulComp(lr->size, delta_out, ferVecOff(lr->x, 1));
        ferVecMulComp(lr->size, delta_out, nn->tmp);

    }else{
        lr2 = &nn->layers[layer + 1];

        for (i = 0; i < lr->size; i++){
            err = BOR_ZERO;
            for (j = 0; j < lr2->size; j++){
                err += ferVecGet(delta, j) * ferVecGet(lr2->w[j], i + 1);
            }

            err *= ferVecGet(lr->x, i + 1);
            err *= (BOR_ONE - ferVecGet(lr->x, i + 1));
            ferVecSet(delta_out, i, err);
        }
    }
}

static void newWeights(bor_nnbp_t *nn, const bor_vec_t *out)
{
    bor_real_t wold, w;
    bor_nnbp_layer_t *lr, *lr2;
    int layer;
    size_t i, j;
    bor_vec_t *din, *delta;

    for (layer = (int)nn->layers_num - 1; layer >= 1; layer--){
        lr  = &nn->layers[layer];
        lr2 = &nn->layers[layer - 1];
        din = nn->delta[layer % 2];
        delta = nn->delta[(layer + 1) % 2];

        // delta of current layer
        makeDelta(nn, layer, out, din, delta);

        for (i = 0; i < lr->size; i++){
            for (j = 0; j < lr2->size + 1; j++){
                w = wold = ferVecGet(lr->w[i], j);
                w -= nn->eta * ferVecGet(delta, i) * ferVecGet(lr2->x, j);
                w += nn->alpha * (ferVecGet(lr->w[i], j) - ferVecGet(lr->prevw[i], j));
                ferVecSet(lr->w[i], j, w);
                ferVecSet(lr->prevw[i], j, wold);
            }
        }
    }
}



static bor_real_t sigmoid(bor_real_t x, bor_real_t lambda)
{
    bor_real_t val = sigmoid01(x, lambda);
    val = (val - BOR_REAL(0.5)) * BOR_REAL(2.);
    return val;
}

static bor_real_t sigmoid01(bor_real_t x, bor_real_t lambda)
{
    bor_real_t val;
    val = ferRecp(BOR_ONE + BOR_EXP(-lambda * x));
    return val;
}
