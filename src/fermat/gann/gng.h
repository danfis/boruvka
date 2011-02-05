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

#ifndef __FER_GANN_GNG_H__
#define __FER_GANN_GNG_H__

#include <fermat/gann/net.h>

struct _gann_gng_node_t {
    gann_net_node_t node;

    fer_real_t err_counter;
};
typedef struct _gann_gng_node_t gann_gng_node_t;


struct _gann_gng_edge_t {
    gann_net_edge_t edge;

    int age;
};
typedef struct _gann_gng_edge_t gann_gng_edge_t;



/**
 * GNG Operations
 * ---------------
 * TODO
 */

/**
 * Create new node initialized weight vector to input_signal.
 */
typedef gann_gng_node_t *(*gann_gng_new_node)(const void *input_signal, void *);

/**
 * Create new node in between n1 and n2.
 */
typedef gann_gng_node_t *(*gann_gng_new_node_between)(const gann_gng_node_t *n1,
                                                      const gann_gng_node_t *n2,
                                                      void *);

/**
 * Deletes given node.
 */
typedef void (*gann_gng_del_node)(gann_gng_node_t *n, void *);

/**
 * Returns random input signal.
 */
typedef const void *(*gann_gng_input_signal)(void *);

/**
 * Returns (via n1 and n2) first and second nearest node to input signal.
 */
typedef void (*gann_gng_nearest)(const void *input_signal,
                                 gann_gng_node_t **n1,
                                 gann_gng_node_t **n2,
                                 void *);

/**
 * Returns squared distance between input_signal and node.
 */
typedef fer_real_t (*gann_gng_dist2)(const void *input_signal,
                                     const gann_gng_node_t *node, void *);

/**
 * Move given node towards input_signal by given fraction.
 *
 * If position on node is w and position of input_signal is v, then:
 * w = w + ((v - w) * fraction)
 */
typedef void (*gann_gng_move_towards)(gann_gng_node_t *node,
                                      const void *input_signal,
                                      fer_real_t fraction,
                                      void *);

/**
 * Returns true if algorithm should terminate.
 */
typedef int (*gann_gng_terminate)(void *);

struct _gann_gng_ops_t {
    gann_gng_new_node         new_node;
    gann_gng_new_node_between new_node_between;
    gann_gng_del_node         del_node;
    gann_gng_input_signal     input_signal;
    gann_gng_nearest          nearest;
    gann_gng_dist2            dist2;
    gann_gng_move_towards     move_towards;
    gann_gng_terminate        terminate;
};
typedef struct _gann_gng_ops_t gann_gng_ops_t;


/**
 * Initializes ops struct to NULL values.
 */
void gannGNGOpsInit(gann_gng_ops_t *ops);



/**
 * GNG Parameters
 * ---------------
 * TODO
 */
struct _gann_gng_params_t {
    size_t lambda;    /*!< Number of steps between adding nodes */
    fer_real_t eb;    /*!< Winner node learning rate */
    fer_real_t en;    /*!< Winners' neighbors learning rate */
    fer_real_t alpha; /*!< Decrease error counter rate */
    fer_real_t beta;  /*!< Decrease error counter rate for all nodes */
    int age_max;      /*!< Maximal age of edge */
};
typedef struct _gann_gng_params_t gann_gng_params_t;

/**
 * Initializes params struct to default values.
 */
void gannGNGParamsInit(gann_gng_params_t *params);



struct _gann_gng_t {
    gann_net_t *net;
    gann_gng_ops_t ops;
    gann_gng_params_t params;
    void *data;
};
typedef struct _gann_gng_t gann_gng_t;


/**
 * Creates new instance of GNG algorithm.
 */
gann_gng_t *gannGNGNew(const gann_gng_ops_t *ops,
                       const gann_gng_params_t *params,
                       void *data);

/**
 * Deletes GNG.
 */
void gannGNGDel(gann_gng_t *gng);

/**
 * Runs GNG algorithm.
 */
void gannGNGRun(gann_gng_t *gng);

/**
 * Returns error counter of node.
 *
 * Always use this function instead of direct access to struct!
 */
fer_real_t gannGNGNodeErrCounter(const gann_gng_t *gng, const gann_gng_node_t *n);

/**
 * Returns age of edge.
 */
int gannGNGEdgeAge(const gann_gng_t *gng, const gann_gng_edge_t *edge);

#endif /* __FER_GANN_GNG_H__ */

