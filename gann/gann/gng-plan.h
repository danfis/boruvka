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

#ifndef __FER_GANN_GNG_PLAN_H__
#define __FER_GANN_GNG_PLAN_H__

#include <gann/net.h>
#include <gann/gng.h>
#include <fermat/vec2.h>
#include <fermat/cubes2.h>

/**
 * Growing Neural Gas for Planning
 * ================================
 */

#define GANN_GNGP_FREE 0
#define GANN_GNGP_OBST 1

struct _gann_gngp_node_t {
    gann_net_node_t node;  /*!< Connection into net */
    fer_cubes2_el_t cubes; /*!< Connection into cubes */
    int set;               /*!< Specifies into which set node belongs to */
    fer_vec2_t w;          /*!< Weight vector */

    fer_list_t fifo; /*!< Connection into fifo queue - used internally */
    int evaled;      /*!< Marks nodes that were already evalueated - used
                          internally */

    fer_real_t err_local; /*!< Local error */
    fer_real_t err;       /*!< Overall error */
};
typedef struct _gann_gngp_node_t gann_gngp_node_t;


struct _gann_gngp_edge_t {
    gann_net_edge_t edge;

    int age;
};
typedef struct _gann_gngp_edge_t gann_gngp_edge_t;

/**
 * GNG Operations
 * ---------------
 *
 * See gann_gngp_ops_t.
 */

/** vvvv */

/**
 * Returns random input signal.
 */
typedef const fer_vec2_t *(*gann_gngp_input_signal)(void *);

/**
 * Returns true if algorithm should terminate.
 */
typedef int (*gann_gngp_terminate)(void *);

/**
 * TODO
 */
typedef int (*gann_gngp_eval)(const fer_vec2_t *w, void *);

/** ^^^^ */

struct _gann_gngp_ops_t {
    gann_gngp_input_signal     input_signal;
    gann_gngp_terminate        terminate;
    gann_gngp_eval             eval;

    void *data; /*!< Data pointer that will be provided to all callbacks if
                     not specified otherwise. */

    void *input_signal_data;
    void *terminate_data;
    void *eval_data;
};
typedef struct _gann_gngp_ops_t gann_gngp_ops_t;


/**
 * Initializes ops struct to NULL values.
 */
void gannGNGPOpsInit(gann_gngp_ops_t *ops);




/**
 * GNG Parameters
 * ---------------
 */
struct _gann_gngp_params_t {
    size_t lambda;    /*!< Number of steps between adding nodes */
    fer_real_t eb;    /*!< Winner node learning rate */
    fer_real_t en;    /*!< Winners' neighbors learning rate */
    fer_real_t alpha; /*!< Decrease error counter rate */
    fer_real_t beta;  /*!< Decrease error counter rate for all nodes */
    int age_max;      /*!< Maximal age of edge */

    size_t num_cubes;
    fer_real_t aabb[4];
};
typedef struct _gann_gngp_params_t gann_gngp_params_t;

/**
 * Initializes params struct to default values.
 */
void gannGNGPParamsInit(gann_gngp_params_t *params);


/**
 * GNG Algorithm
 * --------------
 *
 * See gann_gngp_t.
 */

struct _gann_gngp_t {
    gann_net_t *net;
    fer_cubes2_t *cubes;
    gann_gngp_ops_t ops;
    gann_gngp_params_t params;

    fer_real_t *beta_n; /*!< Precomputed beta^n for n = 1, ..., lambda */
};
typedef struct _gann_gngp_t gann_gngp_t;


/**
 * Creates new instance of GNGP algorithm.
 */
gann_gngp_t *gannGNGPNew(const gann_gngp_ops_t *ops,
                         const gann_gngp_params_t *params);

/**
 * Deletes GNGP.
 */
void gannGNGPDel(gann_gngp_t *gng);

/**
 * Runs GNGP algorithm.
 *
 * This runs whole algorithm in loop until operation terminate() returns
 * true.
 */
void gannGNGPRun(gann_gngp_t *gng);

/**
 * Returns number of nodes in net.
 */
_fer_inline size_t gannGNGPNodesLen(const gann_gngp_t *gng);


/**** INLINES ****/
_fer_inline size_t gannGNGPNodesLen(const gann_gngp_t *gng)
{
    return gannNetNodesLen(gng->net);
}

#endif /* __FER_GANN_GNG_PLAN_H__ */


