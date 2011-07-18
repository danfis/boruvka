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

#ifndef __FER_GNG_PLAN2_H__
#define __FER_GNG_PLAN2_H__

#include <fermat/gng.h>
#include <fermat/vec.h>
#include <fermat/nncells.h>
#include <fermat/dij.h>
#include <fermat/pairheap.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Growing Neural Gas for Planning
 * ================================
 */

struct _fer_gng_plan_node_t {
    fer_gng_node_t node;
    fer_nncells_el_t cells; /*!< Connection into cells */
    fer_vec_t *w;           /*!< Weight vector */

    fer_list_t obst; /*!< Connection into list of obstacle nodes */

    fer_dij_node_t dij; /*!< Connection for dijkstra algorithm */
    fer_list_t path;    /*!< Connection into list representing path */

    int _id;
};
typedef struct _fer_gng_plan_node_t fer_gng_plan_node_t;


/**
 * GNG Operations
 * ---------------
 *
 * See fer_gng_plan_ops_t.
 */

/** vvvv */

#define FER_GNG_PLAN_FREE 0
#define FER_GNG_PLAN_OBST 1

/**
 * TODO
 */
typedef int (*fer_gng_plan_eval)(const fer_vec_t *w, void *);

/** ^^^^ */

struct _fer_gng_plan_ops_t {
    fer_gng_input_signal input_signal;
    fer_gng_terminate    terminate;
    fer_gng_plan_eval    eval;

    fer_gng_callback callback;
    unsigned long callback_period;

    void *data; /*!< Data pointer that will be provided to all callbacks if
                     not specified otherwise. */

    void *input_signal_data;
    void *terminate_data;
    void *eval_data;
    void *callback_data;
};
typedef struct _fer_gng_plan_ops_t fer_gng_plan_ops_t;


/**
 * Initializes ops struct to NULL values.
 */
void ferGNGPlanOpsInit(fer_gng_plan_ops_t *ops);




/**
 * GNG Parameters
 * ---------------
 */
struct _fer_gng_plan_params_t {
    int dim;              /*!< Dimensionality of space. Default: 2 */
    fer_real_t max_dist2; /*!< Maximal cube distance between two nodes in path.
                               Default: 0.1 */
    int min_nodes;        /*!< Minimal number of nodes to start path search.
                               Default: 100 */

    fer_gng_params_t gng;
    fer_nncells_params_t cells;
};
typedef struct _fer_gng_plan_params_t fer_gng_plan_params_t;

/**
 * Initializes params struct to default values.
 */
void ferGNGPlanParamsInit(fer_gng_plan_params_t *params);


/**
 * GNG Algorithm
 * --------------
 *
 * See fer_gng_plan_t.
 */

struct _fer_gng_plan_t {
    int dim;
    fer_real_t max_dist2;
    int min_nodes;

    fer_gng_plan_ops_t ops; /*!< Callbacks */

    fer_gng_t *gng;
    fer_nncells_t *cells;

    fer_list_t obst;           /*!< List of obstacle nodes */
    fer_nncells_t *obst_cells; /*!< NNCells for obstacle nodes */

    fer_vec_t *start, *goal; /*!< Start and goal positions */

    fer_vec_t *tmpv;
};
typedef struct _fer_gng_plan_t fer_gng_plan_t;


/**
 * Creates new instance of GNGP algorithm.
 */
fer_gng_plan_t *ferGNGPlanNew(const fer_gng_plan_ops_t *ops,
                              const fer_gng_plan_params_t *params);

/**
 * Deletes GNGP.
 */
void ferGNGPlanDel(fer_gng_plan_t *gng);

/**
 * Runs GNG plan algorithm.
 *
 * This runs whole algorithm in loop until operation terminate() returns
 * true.
 */
void ferGNGPlanRun(fer_gng_plan_t *gng);

/**
 * Dumps net.
 */
void ferGNGPlanDumpSVT(fer_gng_plan_t *gng, FILE *out, const char *name);

_fer_inline fer_gng_t *ferGNGPlanGNG(fer_gng_plan_t *gngp);
_fer_inline fer_net_t *ferGNGPlanNet(fer_gng_plan_t *gngp);

#if 0
/**
 * Tries to find path in net from start to goal.
 * If path was found 0 is returned and argument list is filled by nodes
 * representing path. Nodes are connected into this list by member .path.
 * If path wasn't found -1 is returned.
 */
int ferGNGPFindPath(fer_gng_plan_t *gng,
                     const fer_vec_t *start, const fer_vec_t *goal,
                     fer_list_t *list);

/**
 * Dumps net in SVT format.
 */
void ferGNGPDumpSVT(fer_gng_plan_t *gng, FILE *out, const char *name);

void ferGNGPDumpNodes(fer_gng_plan_t *gng, FILE *out);
#endif


/**** INLINES ****/
_fer_inline fer_gng_t *ferGNGPlanGNG(fer_gng_plan_t *gngp)
{
    return gngp->gng;
}

_fer_inline fer_net_t *ferGNGPlanNet(fer_gng_plan_t *gngp)
{
    return ferGNGNet(ferGNGPlanGNG(gngp));
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GNG_PLAN2_H__ */



