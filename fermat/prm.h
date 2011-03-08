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

#ifndef __FER_PRM_H__
#define __FER_PRM_H__

#include <fermat/vec.h>
#include <fermat/net.h>
#include <fermat/nncells.h>
#include <fermat/dij.h>

#define FER_PRM_FREE 1
#define FER_PRM_OBST 2


/**
 * PRM - Probabilistic Roadmaps for Path Planning
 * ===============================================
 */

/**
 * PRM Operations
 * ---------------
 *
 * See fer_prm_ops_t.
 */

/** vvvv */

/**
 * Returns random configuration.
 */
typedef const fer_vec_t *(*fer_prm_conf)(void *);

/**
 * Returns true if algorithm should terminate.
 */
typedef int (*fer_prm_terminate)(void *);

/**
 * Evaluate configuration.
 *
 * Returns FER_PRM_FREE if configuration is in free space and FER_PRM_OBST
 * if it is obstacle configuration.
 */
typedef int (*fer_prm_eval)(const fer_vec_t *c, void *);

/**
 * Returns true if there exists path from start configuration to goal
 * configuration.
 */
typedef int (*fer_prm_find_path)(const fer_vec_t *start,
                                 const fer_vec_t *goal, void *);

/**
 * Callback that is peridically called from PRM.
 *
 * It is called every .callback_period'th added node.
 */
typedef void (*fer_prm_callback)(void *);

/** ^^^^ */

struct _fer_prm_ops_t {
    fer_prm_conf      conf;
    fer_prm_terminate terminate;
    fer_prm_eval      eval;
    fer_prm_find_path find_path;

    fer_prm_callback callback;
    unsigned long callback_period;

    void *data; /*!< Data pointer that will be provided to all callbacks if
                     not specified otherwise. */

    void *conf_data;
    void *terminate_data;
    void *eval_data;
    void *find_path_data;
    void *callback_data;
};
typedef struct _fer_prm_ops_t fer_prm_ops_t;

/**
 * Initializes ops struct to NULL values.
 */
void ferPRMOpsInit(fer_prm_ops_t *ops);



/**
 * PRM Parameters
 * ---------------
 */
struct _fer_prm_params_t {
    int d; /*!< Dimension of problem */

    fer_real_t max_dist;      /*!< Maximal distance between nodes that the
                                   local planner may try to connect */
    fer_real_t max_neighbors; /*!< Maximum number of calls of the local
                                   planner per node */

    fer_nncells_params_t cells;
};
typedef struct _fer_prm_params_t fer_prm_params_t;

/**
 * Initializes params struct to default values.
 */
void ferPRMParamsInit(fer_prm_params_t *params);



/**
 * PRM Algorithm
 * --------------
 */
struct _fer_prm_t {
    fer_net_t *net;       /*!< Holds roadmap */
    fer_nncells_t *cells; /*!< NN search */

    fer_prm_ops_t ops;
    fer_prm_params_t params;

    fer_list_t components;
};
typedef struct _fer_prm_t fer_prm_t;

struct _fer_prm_component_t {
    struct _fer_prm_component_t *parent;
    fer_list_t list;
};
typedef struct _fer_prm_component_t fer_prm_component_t;

struct _fer_prm_node_t {
    fer_vec_t *conf;
    fer_prm_component_t *comp;
    fer_net_node_t node;
    fer_nncells_el_t cells;

    fer_dij_node_t dij; /*!< Connection for dijkstra algorithm */
    fer_list_t path;

    int _id;
};
typedef struct _fer_prm_node_t fer_prm_node_t;


/**
 * Creates new instance of PRM algorithm.
 */
fer_prm_t *ferPRMNew(const fer_prm_ops_t *ops,
                     const fer_prm_params_t *params);

/**
 * Deletes PRM instance.
 */
void ferPRMDel(fer_prm_t *prm);

/**
 * Runs algorithm
 */
void ferPRMRun(fer_prm_t *prm);

/**
 * Tries to find path in net from start to goal.
 * If path was found 0 is returned and argument list is filled by nodes
 * representing path. Nodes are connected into this list by member .path.
 * If path wasn't found -1 is returned.
 */
int ferPRMFindPath(fer_prm_t *prm,
                   const fer_vec_t *start, const fer_vec_t *goal,
                   fer_list_t *list);

/**
 * Returns number of nodes in roadmap.
 */
_fer_inline size_t ferPRMNodesLen(const fer_prm_t *prm);

/**
 * Dumps net in SVT format.
 */
void ferPRMDumpSVT(fer_prm_t *prm, FILE *out, const char *name);

/**** INLINES ****/
_fer_inline size_t ferPRMNodesLen(const fer_prm_t *prm)
{
    return ferNetNodesLen(prm->net);
}


#endif /* __FER_PRM_H__ */

