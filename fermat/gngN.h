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

#ifndef __FER_GNG`N`_H__
#define __FER_GNG`N`_H__

#include <fermat/vec`N`.h>
#include <fermat/pc`N`.h>
#include <fermat/cubes`N`.h>
#include <fermat/gng.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Growing Neural Gas In `N`-D
 * ==========================
 */



/**
 * GNG `N`D Operations
 * --------------------
 *
 * This is similar to fer_gng_ops_t.
 *
 * See fer_gng`N`_ops_t.
 */

struct _fer_gng`N`_ops_t {
    fer_gng_terminate terminate; /*!< Termination condition */
    fer_gng_callback callback;
    unsigned long callback_period;

    void *data; /*!< Data for all callbacks if not specified otherwise */
    void *terminate_data;
    void *callback_data;
};
typedef struct _fer_gng`N`_ops_t fer_gng`N`_ops_t;

/**
 * Initialzes operations struct.
 */
void ferGNG`N`OpsInit(fer_gng`N`_ops_t *ops);



/**
 * GNG `N`-D Parameters
 * -------------------
 */
struct _fer_gng`N`_params_t {
    fer_gng_params_t gng;

    int use_cubes;    /*!< Set to true to enable using cubes`N` for nearest
                           neighbor search. [default true] */
    size_t num_cubes; /*!< Number of cubes`N` used for nearest neighbor
                           search. [default 10000] */
};
typedef struct _fer_gng`N`_params_t fer_gng`N`_params_t;

/**
 * Intializes parameters.
 */
void ferGNG`N`ParamsInit(fer_gng`N`_params_t *p);


/**
 * GNG `N`D Algorithm
 * -----------------
 */
struct _fer_gng`N`_t {
    fer_gng_t *gng;     /*!< Pointer to GNG structure */
    fer_pc`N`_t *pc;       /*!< Point cloud - input signals */
    fer_pc`N`_it_t pcit;   /*!< Iterator over .pc */
    fer_cubes`N`_t *cubes; /*!< Cubes`N` for nearest neighbor search */

    fer_gng`N`_ops_t ops;
    fer_gng`N`_params_t params;
};
typedef struct _fer_gng`N`_t fer_gng`N`_t;


/**
 * Creates new instance of GNG`N` algorithm.
 */
fer_gng`N`_t *ferGNG`N`New(const fer_gng`N`_ops_t *ops,
                         const fer_gng`N`_params_t *params);

/**
 * Deletes GNG`N`.
 */
void ferGNG`N`Del(fer_gng`N`_t *gng);

/**
 * Runs GNG`N` algorithm.
 */
void ferGNG`N`Run(fer_gng`N`_t *gng);

/**
 * Adds one input signal.
 */
_fer_inline void ferGNG`N`AddInputSignal(fer_gng`N`_t *gng, const fer_vec`N`_t *is);

/**
 * Adds input signals from file.
 * Returns number of added input signals.
 */
_fer_inline size_t ferGNG`N`AddInputSignalsFromFile(fer_gng`N`_t *gng,
                                                   const char *fn);
/**
 * Returns point cloud.
 */
_fer_inline fer_pc`N`_t *ferGNG`N`PC(fer_gng`N`_t *gng);

/**
 * Returns GNG structure.
 */
_fer_inline fer_gng_t *ferGNG`N`GNG(fer_gng`N`_t *gng);


/**
 * Net Related API
 * ----------------
 */
struct _fer_gng`N`_node_t {
    fer_gng_node_t node;  /*!< GNG node */
    fer_vec`N`_t *w;         /*!< Weight vector */
    fer_cubes`N`_el_t cubes; /*!< Struct for searching in cubes`N` */

    int _id; /*!< Currently useful only for ferGNG`N`DumpSVT(). */
};
typedef struct _fer_gng`N`_node_t fer_gng`N`_node_t;

/**
 * Returns number of nodes in net.
 */
_fer_inline size_t ferGNG`N`NodesLen(const fer_gng`N`_t *gng);

/**
 * Cast from Net node to GNG`N` node.
 */
_fer_inline fer_gng`N`_node_t *ferGNG`N`NodeFromNet(fer_net_node_t *n);

/**
 * Cast from GNG node to GNG`N` node.
 */
_fer_inline fer_gng`N`_node_t *ferGNG`N`NodeFromGNG(fer_gng_node_t *n);


/**
 * Dumps `N`-D network in SVT format.
 */
void ferGNG`N`DumpSVT(fer_gng`N`_t *gng, FILE *out, const char *name);

/**** INLINES ****/
_fer_inline void ferGNG`N`AddInputSignal(fer_gng`N`_t *gng, const fer_vec`N`_t *is)
{
    ferPC`N`Add(gng->pc, is);
}

_fer_inline size_t ferGNG`N`AddInputSignalsFromFile(fer_gng`N`_t *gng,
                                                   const char *fn)
{
    return ferPC`N`AddFromFile(gng->pc, fn);
}

_fer_inline fer_pc`N`_t *ferGNG`N`PC(fer_gng`N`_t *gng)
{
    return gng->pc;
}

_fer_inline fer_gng_t *ferGNG`N`GNG(fer_gng`N`_t *gng)
{
    return gng->gng;
}

_fer_inline fer_net_t *ferGNG`N`Net(fer_gng`N`_t *gng)
{
    return ferGNGNet(gng->gng);
}

_fer_inline size_t ferGNG`N`NodesLen(const fer_gng`N`_t *gng)
{
    return ferGNGNodesLen(gng->gng);
}

_fer_inline fer_gng`N`_node_t *ferGNG`N`NodeFromNet(fer_net_node_t *n)
{
    fer_gng_node_t *gn;
    gn = ferGNGNodeFromNet(n);
    return ferGNG`N`NodeFromGNG(gn);
}

_fer_inline fer_gng`N`_node_t *ferGNG`N`NodeFromGNG(fer_gng_node_t *n)
{
    return fer_container_of(n, fer_gng`N`_node_t, node);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GNG`N`_H__ */

