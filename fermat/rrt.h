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

#ifndef __FER_RRT_H__
#define __FER_RRT_H__

#include <fermat/vec.h>
#include <fermat/net.h>
#include <fermat/nncells.h>
#include <fermat/dij.h>

#define FER_RRT_FREE 1
#define FER_RRT_OBST 2

struct _fer_rrt_node_t;

/**
 * RRT - Rapidly-Exploring Random Trees
 * =====================================
 */

/**
 * RRT Operations
 * ---------------
 *
 * See fer_rrt_ops_t.
 */

/** vvvv */

/**
 * Returns random configuration.
 */
typedef const fer_vec_t *(*fer_rrt_conf)(void *);

/**
 * Returns true if algorithm should terminate.
 */
typedef int (*fer_rrt_terminate)(void *);

/**
 * Returns next configuration where should tree grow from xnear.
 *
 * xrand is randomly generated configuration from ops.conf() and xnear is
 * nearest configuration from tree.
 */
typedef const fer_vec_t *(*fer_rrt_new_conf)(const fer_vec_t *xnear,
                                             const fer_vec_t *xrand, void *);


/**
 * Callback that is peridically called from PRM.
 *
 * It is called every .callback_period'th added node.
 */
typedef void (*fer_rrt_callback)(void *);

/** ^^^^ */

struct _fer_rrt_ops_t {
    fer_rrt_conf      conf;
    fer_rrt_terminate terminate;
    fer_rrt_new_conf new_conf;

    fer_rrt_callback callback;
    unsigned long callback_period;

    void *data; /*!< Data pointer that will be provided to all callbacks if
                     not specified otherwise. */

    void *conf_data;
    void *terminate_data;
    void *new_conf_data;
    void *callback_data;
};
typedef struct _fer_rrt_ops_t fer_rrt_ops_t;

/**
 * Initializes ops struct to NULL values.
 */
void ferRRTOpsInit(fer_rrt_ops_t *ops);


/**
 * RRT Parameters
 * ---------------
 */
struct _fer_rrt_params_t {
    int d; /*!< Dimension of problem */

    fer_nncells_params_t cells;
};
typedef struct _fer_rrt_params_t fer_rrt_params_t;

/**
 * Initializes params struct to default values.
 */
void ferRRTParamsInit(fer_rrt_params_t *params);


/**
 * RRT Algorithm
 * --------------
 *
 * See fer_rrt_t.
 */

struct _fer_rrt_node_t {
    fer_vec_t *conf;
    fer_net_node_t node;
    fer_nncells_el_t cells;

    fer_dij_node_t dij;
    fer_list_t path;

    int _id;
};
typedef struct _fer_rrt_node_t fer_rrt_node_t;

struct _fer_rrt_t {
    fer_rrt_ops_t ops;
    fer_rrt_params_t params;

    fer_net_t *net;
    fer_nncells_t *cells;

    fer_rrt_node_t *node_init; /*!< Initial node */
    fer_rrt_node_t *node_last; /*!< Last generated node */
};
typedef struct _fer_rrt_t fer_rrt_t;

/**
 * Creates new instance of algorithm
 */
fer_rrt_t *ferRRTNew(const fer_rrt_ops_t *ops,
                     const fer_rrt_params_t *params);

/**
 * Deletes RRT instance.
 */
void ferRRTDel(fer_rrt_t *rrt);

/**
 * Runs algorithm.
 */
void ferRRTRun(fer_rrt_t *rrt, const fer_vec_t *init);

/**
 * Dumps net in SVT format.
 */
void ferRRTDumpSVT(fer_rrt_t *rrt, FILE *out, const char *name);

/**
 * Returns number of nodes in roadmap.
 */
_fer_inline size_t ferRRTNodesLen(const fer_rrt_t *rrt);

/**
 * Returns initial node.
 *
 * The one with configuration passed to *Run() function.
 */
_fer_inline const fer_rrt_node_t *ferRRTNodeInitial(const fer_rrt_t *rrt);

/**
 * Returns last newly created node.
 */
_fer_inline const fer_rrt_node_t *ferRRTNodeLast(const fer_rrt_t *rrt);

/**
 * Creates new node in tree with configuration {conf} and this node will be
 * connected with node {n}. Node {n} must be already in RRT's net.
 */
const fer_rrt_node_t *ferRRTNodeNew(fer_rrt_t *rrt, const fer_vec_t *conf,
                                    const fer_rrt_node_t *n);


/**
 * Tries to find path in net from init to goal.
 * If path was found 0 is returned and argument list is filled by nodes
 * representing path. Nodes are connected into this list by member .path.
 * If path wasn't found -1 is returned.
 */
int ferRRTFindPath(fer_rrt_t *rrt,
                   const fer_rrt_node_t *init, const fer_rrt_node_t *goal,
                   fer_list_t *list);

/**
 * Node Functions
 * ---------------
 *
 * See fer_rrt_node_t.
 */

/**
 * Returns configuration (state) of node.
 */
_fer_inline const fer_vec_t *ferRRTNodeConf(const fer_rrt_node_t *n);


/**** INLINES ****/
_fer_inline size_t ferRRTNodesLen(const fer_rrt_t *rrt)
{
    return ferNetNodesLen(rrt->net);
}

_fer_inline const fer_rrt_node_t *ferRRTNodeInitial(const fer_rrt_t *rrt)
{
    return rrt->node_init;
}

_fer_inline const fer_rrt_node_t *ferRRTNodeLast(const fer_rrt_t *rrt)
{
    return rrt->node_last;
}


_fer_inline const fer_vec_t *ferRRTNodeConf(const fer_rrt_node_t *n)
{
    return n->conf;
}

#endif /* __FER_RRT_H__ */
