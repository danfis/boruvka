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

#ifndef __FER_GNNP_H__
#define __FER_GNNP_H__

#include <fermat/core.h>
#include <fermat/net.h>
#include <fermat/nn.h>
#include <fermat/dij.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_gnnp_t;

/**
 * Growing Neural Network for Planning
 * ====================================
 * See fer_gnnp_t.
 */

struct _fer_gnnp_node_t {
    fer_vec_t *w;  /*!< Weight vector */
    fer_net_node_t net;
    fer_nn_el_t nn;
    fer_nn_el_t nn_path;

    uint8_t fixed; /*!< True if node is fixed (1 - FREE, 2 - OBST) */

    fer_list_t path;
    struct _fer_gnnp_node_t *prev; /*!< Learned path from node towards
                                        init or goal node */
    uint8_t prev_type;             /*!< prev is: 1 -> init, 2 -> goal */

    int _id;
};
typedef struct _fer_gnnp_node_t fer_gnnp_node_t;


/**
 * Operators
 * ----------
 * See fer_gnnp_ops_t.
 */

/** vvvv */

/**
 * Returns random input signal.
 */
typedef const fer_vec_t *(*fer_gnnp_input_signal)(struct _fer_gnnp_t *nn, void *);

/**
 * Returns true if algorithm should terminate.
 */
typedef int (*fer_gnnp_terminate)(struct _fer_gnnp_t *nn, void *);

/**
 * Returns true if it {conf} is in free space
 */
typedef int (*fer_gnnp_eval)(struct _fer_gnnp_t *nn, const fer_vec_t *conf, void *);

/**
 * Callback that is peridically called from GNG.
 *
 * It is called every .callback_period'th added node.
 */
typedef void (*fer_gnnp_callback)(struct _fer_gnnp_t *nn, void *);

/** ^^^^ */

struct _fer_gnnp_ops_t {
    fer_gnnp_input_signal input_signal; /*!< Default: NULL */
    fer_gnnp_terminate    terminate;    /*!< Default: NULL */
    fer_gnnp_eval         eval;         /*!< Default: NULL */

    fer_gnnp_callback callback;
    unsigned int callback_period;

    void *data;
    void *input_signal_data;
    void *terminate_data;
    void *eval_data;
    void *callback_data;
};
typedef struct _fer_gnnp_ops_t fer_gnnp_ops_t;

/**
 * Initializes operators struct
 */
void ferGNNPOpsInit(fer_gnnp_ops_t *ops);


/**
 * Parameters
 * -----------
 */
struct _fer_gnnp_params_t {
    int dim;           /*!< Dimension. Default: 2 */
    fer_real_t ew;     /*!< Winner node's learning rate. Default: 0.05 */
    fer_real_t en;     /*!< Winner neighbor's learning rate. Default: 0.0005 */
    unsigned int rmax; /*!< Max rank of node. Default: 4 */
    fer_real_t h;      /*!< Resolution. Default: 0.1 */

    fer_nn_params_t nn; /*!< Params of nearest neighbor search */
};
typedef struct _fer_gnnp_params_t fer_gnnp_params_t;

/**
 * Initialize parameters
 */
void ferGNNPParamsInit(fer_gnnp_params_t *p);


/**
 * Functions
 * ----------
 */

struct _fer_gnnp_t {
    fer_gnnp_ops_t ops;
    fer_gnnp_params_t params;
    fer_net_t *net;
    fer_nn_t *nn;
    fer_nn_t *nn_path;

    fer_gnnp_node_t *init, *goal; /*!< Init and goal nodes */
    fer_vec_t *tmpv;
};
typedef struct _fer_gnnp_t fer_gnnp_t;

/**
 * Creates network
 */
fer_gnnp_t *ferGNNPNew(const fer_gnnp_ops_t *ops, const fer_gnnp_params_t *p);

/**
 * Deletes network
 */
void ferGNNPDel(fer_gnnp_t *nn);


/**
 * Finds path between start and goal.
 * If path was found, {path} argument is filled and 0 is returned.
 */
int ferGNNPFindPath(fer_gnnp_t *nn,
                    const fer_vec_t *start, const fer_vec_t *goal,
                    fer_list_t *path);

/**
 * Dumps net (if it is 2-D or 3-D) as SVT object
 */
void ferGNNPDumpSVT(const fer_gnnp_t *nn, FILE *out, const char *name);

/**
 * Returns number of nodes in network
 */
_fer_inline size_t ferGNNPNodesLen(const fer_gnnp_t *nn);

/**
 * Returns network
 */
_fer_inline fer_net_t *ferGNNPNet(fer_gnnp_t *nn);

/**** INLINES ****/
_fer_inline size_t ferGNNPNodesLen(const fer_gnnp_t *nn)
{
    return ferNetNodesLen(nn->net);
}

_fer_inline fer_net_t *ferGNNPNet(fer_gnnp_t *nn)
{
    return nn->net;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GNNP_H__ */
