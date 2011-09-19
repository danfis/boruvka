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

#ifndef __FER_GNG_PLAN_H__
#define __FER_GNG_PLAN_H__

#include <fermat/net.h>
#include <fermat/vec.h>
#include <fermat/gug.h>
#include <fermat/dij.h>
#include <fermat/pairheap.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Growing Neural Gas for Planning
 * ================================
 */

#define FER_GNGP_NONE 0
#define FER_GNGP_FREE 1
#define FER_GNGP_OBST 2

struct _fer_gngp_node_t {
    fer_net_node_t node; /*!< Connection into net */
    fer_gug_el_t gug;    /*!< Connection into GUG */
    int set;             /*!< Specifies into which set node belongs to */
    fer_vec_t *w;        /*!< Weight vector */

    fer_list_t fifo;     /*!< Connection into fifo queue - used internally */
    unsigned int evaled; /*!< Marks nodes that were already evalueated - used
                              internally */

    fer_real_t err;               /*!< Error counter */
    unsigned long err_cycle;      /*!< Last cycle in which were .err changed */
    fer_pairheap_node_t err_heap; /*!< Connection into error heap */

    fer_dij_node_t dij; /*!< Connection for dijkstra algorithm */
    fer_list_t path;    /*!< Connection into list representing path */

    int _id;
};
typedef struct _fer_gngp_node_t fer_gngp_node_t;


struct _fer_gngp_edge_t {
    fer_net_edge_t edge;

    int age;
};
typedef struct _fer_gngp_edge_t fer_gngp_edge_t;

/**
 * GNG Operations
 * ---------------
 *
 * See fer_gngp_ops_t.
 */

/** vvvv */

/**
 * Returns random input signal.
 */
typedef const fer_vec_t *(*fer_gngp_input_signal)(void *);

/**
 * Returns true if algorithm should terminate.
 */
typedef int (*fer_gngp_terminate)(void *);

/**
 * TODO
 */
typedef int (*fer_gngp_eval)(const fer_vec_t *w, void *);

/**
 * Callback that is peridically called from GNG.
 *
 * It is called every .callback_period'th added node.
 */
typedef void (*fer_gngp_callback)(void *);

/** ^^^^ */

struct _fer_gngp_ops_t {
    fer_gngp_input_signal     input_signal;
    fer_gngp_terminate        terminate;
    fer_gngp_eval             eval;

    fer_gngp_callback callback;
    unsigned long callback_period;

    void *data; /*!< Data pointer that will be provided to all callbacks if
                     not specified otherwise. */

    void *input_signal_data;
    void *terminate_data;
    void *eval_data;
    void *callback_data;
};
typedef struct _fer_gngp_ops_t fer_gngp_ops_t;


/**
 * Initializes ops struct to NULL values.
 */
void ferGNGPOpsInit(fer_gngp_ops_t *ops);




/**
 * GNG Parameters
 * ---------------
 */
struct _fer_gngp_params_t {
    size_t d;         /*!< Dimension. Default: 2 */
    size_t lambda;    /*!< Number of steps between adding nodes */
    fer_real_t eb;    /*!< Winner node learning rate */
    fer_real_t en;    /*!< Winners' neighbors learning rate */
    fer_real_t alpha; /*!< Decrease error counter rate */
    fer_real_t beta;  /*!< Decrease error counter rate for all nodes */
    int age_max;      /*!< Maximal age of edge */

    size_t warm_start;

    fer_gug_params_t gug;
};
typedef struct _fer_gngp_params_t fer_gngp_params_t;

/**
 * Initializes params struct to default values.
 */
void ferGNGPParamsInit(fer_gngp_params_t *params);


/**
 * GNG Algorithm
 * --------------
 *
 * See fer_gngp_t.
 */

struct _fer_gngp_t {
    fer_net_t *net;
    fer_gug_t *gug;
    fer_gngp_ops_t ops;
    fer_gngp_params_t params;

    fer_real_t *beta_n; /*!< Precomputed beta^n for n = 1, ..., lambda */
    fer_real_t *beta_lambda_n; /*!< Precomputed beta^(n*lambda) for
                                    n = 1, ..., .beta_lambda_n_len */
    size_t beta_lambda_n_len;

    size_t set_size[3]; /*!< Holds size of sets NONE, FREE, OBST */

    size_t step;
    unsigned long cycle;          /*!< Number of cycle */

    fer_pairheap_t *err_heap;
};
typedef struct _fer_gngp_t fer_gngp_t;


/**
 * Creates new instance of GNGP algorithm.
 */
fer_gngp_t *ferGNGPNew(const fer_gngp_ops_t *ops,
                       const fer_gngp_params_t *params);

/**
 * Deletes GNGP.
 */
void ferGNGPDel(fer_gngp_t *gng);

/**
 * Runs GNGP algorithm.
 *
 * This runs whole algorithm in loop until operation terminate() returns
 * true.
 */
void ferGNGPRun(fer_gngp_t *gng);

/**
 * Returns number of nodes in net.
 */
_fer_inline size_t ferGNGPNodesLen(const fer_gngp_t *gng);

/**
 * Tries to find path in net from start to goal.
 * If path was found 0 is returned and argument list is filled by nodes
 * representing path. Nodes are connected into this list by member .path.
 * If path wasn't found -1 is returned.
 */
int ferGNGPFindPath(fer_gngp_t *gng,
                     const fer_vec_t *start, const fer_vec_t *goal,
                     fer_list_t *list);

/**
 * Dumps net in SVT format.
 */
void ferGNGPDumpSVT(fer_gngp_t *gng, FILE *out, const char *name);

/**
 * Dump nodes - coordinates and set
 */
void ferGNGPDumpNodes(fer_gngp_t *gng, FILE *out);


/**** INLINES ****/
_fer_inline size_t ferGNGPNodesLen(const fer_gngp_t *gng)
{
    return ferNetNodesLen(gng->net);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GNG_PLAN_H__ */


