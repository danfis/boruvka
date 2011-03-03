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
#include <fermat/vec2.h>
#include <fermat/nncells.h>
#include <fermat/dij.h>

/**
 * Growing Neural Gas for Planning
 * ================================
 */

#define FER_GNGP_NONE 0
#define FER_GNGP_FREE 1
#define FER_GNGP_OBST 2

struct _fer_gngp_node_t {
    fer_net_node_t node;   /*!< Connection into net */
    fer_nncells_el_t cells; /*!< Connection into cells */
    int set;                /*!< Specifies into which set node belongs to */
    fer_vec2_t w;           /*!< Weight vector */

    fer_list_t fifo; /*!< Connection into fifo queue - used internally */
    int evaled;      /*!< Marks nodes that were already evalueated - used
                          internally */

    fer_real_t err_local; /*!< Local error */
    fer_real_t err;       /*!< Overall error */

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
typedef const fer_vec2_t *(*fer_gngp_input_signal)(void *);

/**
 * Returns true if algorithm should terminate.
 */
typedef int (*fer_gngp_terminate)(void *);

/**
 * TODO
 */
typedef int (*fer_gngp_eval)(const fer_vec2_t *w, void *);

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
    size_t lambda;    /*!< Number of steps between adding nodes */
    fer_real_t eb;    /*!< Winner node learning rate */
    fer_real_t en;    /*!< Winners' neighbors learning rate */
    fer_real_t alpha; /*!< Decrease error counter rate */
    fer_real_t beta;  /*!< Decrease error counter rate for all nodes */
    int age_max;      /*!< Maximal age of edge */

    size_t warm_start;

    size_t num_cells;
    fer_real_t aabb[4];
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
    fer_nncells_t *cells;
    fer_gngp_ops_t ops;
    fer_gngp_params_t params;

    fer_real_t *beta_n; /*!< Precomputed beta^n for n = 1, ..., lambda */

    size_t set_size[3]; /*!< Holds size of sets NONE, FREE, OBST */
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
                     const fer_vec2_t *start, const fer_vec2_t *goal,
                     fer_list_t *list);

/**
 * Dumps net in SVT format.
 */
void ferGNGPDumpSVT(fer_gngp_t *gng, FILE *out, const char *name);


/**** INLINES ****/
_fer_inline size_t ferGNGPNodesLen(const fer_gngp_t *gng)
{
    return ferNetNodesLen(gng->net);
}

#endif /* __FER_GNG_PLAN_H__ */


