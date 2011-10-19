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

#include <fermat/nnbp.h>
#include <fermat/rand-mt.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

fer_nnbp_t *ferNNBPNew(const fer_nnbp_params_t *params)
{
    fer_nnbp_t *nn;
    fer_rand_mt_t *rnd;
    size_t i, j, k;

    nn = FER_ALLOC(fer_nnbp_t);

    nn->layers_num = params->layers_num;
    nn->eta    = params->eta;
    nn->alpha  = params->alpha;
    nn->lambda = params->lambda;

    //rnd = ferRandMTNewAuto();
    rnd = ferRandMTNew(1111);

    nn->layers = FER_ALLOC_ARR(fer_nnbp_layer_t, nn->layers_num);
    for (i = 0; i < nn->layers_num; i++){
        nn->layers[i].size = params->layer_size[i];
        nn->layers[i].x    = ferVecNew(nn->layers[i].size + 1);
        nn->layers[i].w    = NULL;

        ferVecSet(nn->layers[i].x, 0, -1.);

        if (i > 0){
            nn->layers[i].w = FER_ALLOC_ARR(fer_vec_t *, nn->layers[i].size);
            for (j = 0; j < nn->layers[i].size; j++){
                nn->layers[i].w[j] = ferVecNew(nn->layers[i - 1].size + 1);

                for (k = 0; k < nn->layers[i - 1].size + 1; k++){
                    // TODO: parameter eps
                    ferVecSet(nn->layers[i].w[j], k, ferRandMT(rnd, -0.3, 0.3));
                }
            }
        }
    }

    ferRandMTDel(rnd);

    return nn;
}

void ferNNBPDel(fer_nnbp_t *nn)
{
    size_t i, j;

    for (i = 0; i < nn->layers_num; i++){
        ferVecDel(nn->layers[i].x);
        if (i > 0){
            for (j = 0; j < nn->layers[i].size; j++){
                ferVecDel(nn->layers[i].w[j]);
            }
            FER_FREE(nn->layers[i].w);
        }
    }
    FER_FREE(nn->layers);

    FER_FREE(nn);
}

static fer_real_t sigmoid(fer_real_t x, fer_real_t lambda)
{
    fer_real_t val;
    val = ferRecp(FER_ONE + FER_EXP(-lambda * x));
    return val;
}

/** Feed NN with input data */
static void ferNNBPFeed(fer_nnbp_t *nn, const fer_vec_t *in)
{
    fer_real_t sum, val;
    size_t i, n;

    // copy input layer
    DBG("0, %d, %f", -1, ferVecGet(nn->layers[0].x, 0));
    for (i = 0; i < nn->layers[0].size; i++){
        ferVecSet(nn->layers[0].x, i + 1, ferVecGet(in, i));
        DBG("0, %u, %f", i, ferVecGet(nn->layers[0].x, i + 1));
    }

    // for each neuron in each layer
    for (i = 1; i < nn->layers_num; i++){
        DBG("%u, %d, %f", i, -1, ferVecGet(nn->layers[i].x, 0));
        for (n = 0; n < nn->layers[i].size; n++){
            sum = ferVecDot(nn->layers[i - 1].size + 1,
                            nn->layers[i].w[n], nn->layers[i - 1].x);
            val = sigmoid(sum, nn->lambda);
            ferVecSet(nn->layers[i].x, n + 1, val);
            DBG("%u, %u, %f", i, n, ferVecGet(nn->layers[i].x, n + 1));
        }
    }
}

static fer_real_t ferNNBPErr(const fer_nnbp_t *nn,
                             const fer_vec_t *in)
{
    size_t i;
    fer_real_t err;

    err = FER_CUBE(ferVecGet(nn->layers[nn->layers_num - 1].x, 1)
                    - ferVecGet(in, 0));
    for (i = 1; i < nn->layers[nn->layers_num - 1].size; i++){
        err += FER_CUBE(ferVecGet(nn->layers[nn->layers_num - 1].x, i + 1)
                            - ferVecGet(in, i));
    }

    return err;
}

static void ferNNBPNewWeights(fer_nnbp_t *nn)
{
    // TODO
}

fer_real_t ferNNBPLearn(fer_nnbp_t *nn, const fer_vec_t *in)
{
    fer_real_t err;

    // feed neural network with the input
    ferNNBPFeed(nn, in);

    // compute error
    err = ferNNBPErr(nn, in);

    ferNNBPNewWeights(nn);

    return err;
}
