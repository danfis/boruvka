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

#ifndef __FER_NNBP_H__
#define __FER_NNBP_H__

#include <fermat/core.h>
#include <fermat/vec.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Neural Networks - Backpropagation
 * ==================================
 */

/** Transition Functions */
#define FER_NNBP_SIGMOID   0  /*!< sigmoid <-1, 1> */
#define FER_NNBP_SIGMOID01 1  /*!< sigmoid <0, 1> */

struct _fer_nnbp_layer_t {
    size_t size;   /*!< Size of layer */
    fer_vec_t *x;  /*!< Layer output.
                        x[i] = output of (i - 1)'th neuron
                        x[0] = -1 */
    fer_vec_t **w; /*!< Layer weights.
                        w[i] = weight of i'th neuron (~x[i + 1]) */
    fer_vec_t **prevw;
};
typedef struct _fer_nnbp_layer_t fer_nnbp_layer_t;

struct _fer_nnbp_params_t {
    size_t layers_num;  /*!< Number of layers */
    size_t *layer_size; /*!< Number of neurons in layer (from input to
                             output). Size of array must be {.layers} */
    fer_real_t alpha;   /*!< */
    fer_real_t eta;     /*!< */

    fer_real_t lambda;  /*!< Sigmoid slope */
    int func;           /*!< Transition function of single neuron.
                             Default: FER_NNBP_SIGMOID */
};
typedef struct _fer_nnbp_params_t fer_nnbp_params_t;

/**
 * Initialize parameters
 */
void ferNNBPParamsInit(fer_nnbp_params_t *params);

struct _fer_nnbp_t {
    size_t layers_num;  /*!< Number of layers */
    fer_real_t alpha, eta, lambda;

    fer_nnbp_layer_t *layers; /*!< Array of layers */
    fer_vec_t *delta[2];
    fer_vec_t *tmp;

    int func;
};
typedef struct _fer_nnbp_t fer_nnbp_t;

/**
 * Create new network
 */
fer_nnbp_t *ferNNBPNew(const fer_nnbp_params_t *params);

/**
 * Delete network
 */
void ferNNBPDel(fer_nnbp_t *nn);

/**
 * Feeds network with an input.
 * An output of the network is returned.
 */
const fer_vec_t *ferNNBPFeed(fer_nnbp_t *nn, const fer_vec_t *in);

/**
 * Returns error of feeded network (sum of sqares)
 */
fer_real_t ferNNBPErr(const fer_nnbp_t *nn, const fer_vec_t *out);

/**
 * Show one pattern for learning.
 * Returns an error.
 * A length of {in} must equal to {params.layer_size[0]} and
 * length of {out} must equal to number of neurons in output layer.
 */
void ferNNBPLearn(fer_nnbp_t *nn, const fer_vec_t *in, const fer_vec_t *out);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_NNBP_H__ */

