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

#ifndef __FER_GANN_GNG`N`_H__
#define __FER_GANN_GNG`N`_H__

#include <fermat/vec`N`.h>
#include <fermat/pc`N`.h>
#include <fermat/cubes`N`.h>
#include <gann/gng.h>

/**
 * Growing Neural Gas In `N`-D
 * ==========================
 */



/**
 * GNG `N`D Operations
 * ---------------
 *
 * This is similar to gann_gng_ops_t.
 *
 * See gann_gng`N`_ops_t.
 */

struct _gann_gng`N`_ops_t {
    gann_gng_terminate terminate; /*!< Termination condition */
    gann_gng_callback callback;
    unsigned long callback_period;

    void *data; /*!< Data for all callbacks if not specified otherwise */
    void *terminate_data;
    void *callback_data;
};
typedef struct _gann_gng`N`_ops_t gann_gng`N`_ops_t;

/**
 * Initialzes operations struct.
 */
void gannGNG`N`OpsInit(gann_gng`N`_ops_t *ops);



/**
 * GNG `N`-D Parameters
 * -------------------
 */
struct _gann_gng`N`_params_t {
    gann_gng_params_t gng;

    int use_cubes;    /*!< Set to true to enable using cubes`N` for nearest
                           neighbor search. [default true] */
    size_t num_cubes; /*!< Number of cubes`N` used for nearest neighbor
                           search. [default 10000] */
};
typedef struct _gann_gng`N`_params_t gann_gng`N`_params_t;

/**
 * Intializes parameters.
 */
void gannGNG`N`ParamsInit(gann_gng`N`_params_t *p);


/**
 * GNG `N`D Algorithm
 * -----------------
 */
struct _gann_gng`N`_t {
    gann_gng_t *gng;     /*!< Pointer to GNG structure */
    fer_pc`N`_t *pc;       /*!< Point cloud - input signals */
    fer_pc`N`_it_t pcit;   /*!< Iterator over .pc */
    fer_cubes`N`_t *cubes; /*!< Cubes`N` for nearest neighbor search */

    gann_gng`N`_ops_t ops;
    gann_gng`N`_params_t params;
};
typedef struct _gann_gng`N`_t gann_gng`N`_t;


/**
 * Creates new instance of GNG`N` algorithm.
 */
gann_gng`N`_t *gannGNG`N`New(const gann_gng`N`_ops_t *ops,
                         const gann_gng`N`_params_t *params);

/**
 * Deletes GNG`N`.
 */
void gannGNG`N`Del(gann_gng`N`_t *gng);

/**
 * Runs GNG`N` algorithm.
 */
void gannGNG`N`Run(gann_gng`N`_t *gng);

/**
 * Adds one input signal.
 */
_fer_inline void gannGNG`N`AddInputSignal(gann_gng`N`_t *gng, const fer_vec`N`_t *is);

/**
 * Adds input signals from file.
 * Returns number of added input signals.
 */
_fer_inline size_t gannGNG`N`AddInputSignalsFromFile(gann_gng`N`_t *gng,
                                                   const char *fn);
/**
 * Returns point cloud.
 */
_fer_inline fer_pc`N`_t *gannGNG`N`PC(gann_gng`N`_t *gng);

/**
 * Returns GNG structure.
 */
_fer_inline gann_gng_t *gannGNG`N`GNG(gann_gng`N`_t *gng);


/**
 * Net Related API
 * ----------------
 */
struct _gann_gng`N`_node_t {
    gann_gng_node_t node;  /*!< GNG node */
    fer_vec`N`_t *w;         /*!< Weight vector */
    fer_cubes`N`_el_t cubes; /*!< Struct for searching in cubes`N` */

    int _id; /*!< Currently useful only for gannGNG`N`DumpSVT(). */
};
typedef struct _gann_gng`N`_node_t gann_gng`N`_node_t;

/**
 * Returns number of nodes in net.
 */
_fer_inline size_t gannGNG`N`NodesLen(const gann_gng`N`_t *gng);

/**
 * Cast from Net node to GNG`N` node.
 */
_fer_inline gann_gng`N`_node_t *gannGNG`N`NodeFromNet(gann_net_node_t *n);

/**
 * Cast from GNG node to GNG`N` node.
 */
_fer_inline gann_gng`N`_node_t *gannGNG`N`NodeFromGNG(gann_gng_node_t *n);


/**
 * Dumps `N`-D network in SVT format.
 */
void gannGNG`N`DumpSVT(gann_gng`N`_t *gng, FILE *out, const char *name);

/**** INLINES ****/
_fer_inline void gannGNG`N`AddInputSignal(gann_gng`N`_t *gng, const fer_vec`N`_t *is)
{
    ferPC`N`Add(gng->pc, is);
}

_fer_inline size_t gannGNG`N`AddInputSignalsFromFile(gann_gng`N`_t *gng,
                                                   const char *fn)
{
    return ferPC`N`AddFromFile(gng->pc, fn);
}

_fer_inline fer_pc`N`_t *gannGNG`N`PC(gann_gng`N`_t *gng)
{
    return gng->pc;
}

_fer_inline gann_gng_t *gannGNG`N`GNG(gann_gng`N`_t *gng)
{
    return gng->gng;
}

_fer_inline gann_net_t *gannGNG`N`Net(gann_gng`N`_t *gng)
{
    return gannGNGNet(gng->gng);
}

_fer_inline size_t gannGNG`N`NodesLen(const gann_gng`N`_t *gng)
{
    return gannGNGNodesLen(gng->gng);
}

_fer_inline gann_gng`N`_node_t *gannGNG`N`NodeFromNet(gann_net_node_t *n)
{
    gann_gng_node_t *gn;
    gn = gannGNGNodeFromNet(n);
    return gannGNG`N`NodeFromGNG(gn);
}

_fer_inline gann_gng`N`_node_t *gannGNG`N`NodeFromGNG(gann_gng_node_t *n)
{
    return fer_container_of(n, gann_gng`N`_node_t, node);
}
#endif /* __FER_GANN_GNG`N`_H__ */

