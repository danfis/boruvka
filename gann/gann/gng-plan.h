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
#include <fermat/nncells.h>
#include <fermat/dij.h>

/**
 * Growing Neural Gas for Planning
 * ================================
 */

#define GANN_GNGP_NONE 0
#define GANN_GNGP_FREE 1
#define GANN_GNGP_OBST 2

struct _gann_gngp_node_t {
    gann_net_node_t node;   /*!< Connection into net */
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

/**
 * Callback that is peridically called from GNG.
 *
 * It is called every .callback_period'th added node.
 */
typedef void (*gann_gngp_callback)(void *);

/** ^^^^ */

struct _gann_gngp_ops_t {
    gann_gngp_input_signal     input_signal;
    gann_gngp_terminate        terminate;
    gann_gngp_eval             eval;

    gann_gng_callback callback;
    unsigned long callback_period;

    void *data; /*!< Data pointer that will be provided to all callbacks if
                     not specified otherwise. */

    void *input_signal_data;
    void *terminate_data;
    void *eval_data;
    void *callback_data;
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

    size_t warm_start;

    size_t num_cells;
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
    fer_nncells_t *cells;
    gann_gngp_ops_t ops;
    gann_gngp_params_t params;

    fer_real_t *beta_n; /*!< Precomputed beta^n for n = 1, ..., lambda */

    size_t set_size[3]; /*!< Holds size of sets NONE, FREE, OBST */
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

/**
 * Tries to find path in net from start to goal.
 * If path was found 0 is returned and argument list is filled by nodes
 * representing path. Nodes are connected into this list by member .path.
 * If path wasn't found -1 is returned.
 */
int gannGNGPFindPath(gann_gngp_t *gng,
                     const fer_vec2_t *start, const fer_vec2_t *goal,
                     fer_list_t *list);

/**
 * Dumps net in SVT format.
 */
void gannGNGPDumpSVT(gann_gngp_t *gng, FILE *out, const char *name);


/**** INLINES ****/
_fer_inline size_t gannGNGPNodesLen(const gann_gngp_t *gng)
{
    return gannNetNodesLen(gng->net);
}

#endif /* __FER_GANN_GNG_PLAN_H__ */


