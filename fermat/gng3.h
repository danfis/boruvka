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

#ifndef __FER_GNG3_H__
#define __FER_GNG3_H__

#include <fermat/vec3.h>
#include <fermat/pc3.h>
#include <fermat/cubes3.h>
#include <fermat/gng.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Growing Neural Gas In 3-D
 * ==========================
 */



/**
 * GNG 3D Operations
 * --------------------
 *
 * This is similar to fer_gng_ops_t.
 *
 * See fer_gng3_ops_t.
 */

struct _fer_gng3_ops_t {
    fer_gng_terminate terminate; /*!< Termination condition */
    fer_gng_callback callback;
    unsigned long callback_period;

    void *data; /*!< Data for all callbacks if not specified otherwise */
    void *terminate_data;
    void *callback_data;
};
typedef struct _fer_gng3_ops_t fer_gng3_ops_t;

/**
 * Initialzes operations struct.
 */
void ferGNG3OpsInit(fer_gng3_ops_t *ops);



/**
 * GNG 3-D Parameters
 * -------------------
 */
struct _fer_gng3_params_t {
    fer_gng_params_t gng;

    int use_cubes;    /*!< Set to true to enable using cubes3 for nearest
                           neighbor search. [default true] */
    size_t num_cubes; /*!< Number of cubes3 used for nearest neighbor
                           search. [default 10000] */
};
typedef struct _fer_gng3_params_t fer_gng3_params_t;

/**
 * Intializes parameters.
 */
void ferGNG3ParamsInit(fer_gng3_params_t *p);


/**
 * GNG 3D Algorithm
 * -----------------
 */
struct _fer_gng3_t {
    fer_gng_t *gng;     /*!< Pointer to GNG structure */
    fer_pc3_t *pc;       /*!< Point cloud - input signals */
    fer_pc3_it_t pcit;   /*!< Iterator over .pc */
    fer_cubes3_t *cubes; /*!< Cubes3 for nearest neighbor search */

    fer_gng3_ops_t ops;
    fer_gng3_params_t params;
};
typedef struct _fer_gng3_t fer_gng3_t;


/**
 * Creates new instance of GNG3 algorithm.
 */
fer_gng3_t *ferGNG3New(const fer_gng3_ops_t *ops,
                         const fer_gng3_params_t *params);

/**
 * Deletes GNG3.
 */
void ferGNG3Del(fer_gng3_t *gng);

/**
 * Runs GNG3 algorithm.
 */
void ferGNG3Run(fer_gng3_t *gng);

/**
 * Adds one input signal.
 */
_fer_inline void ferGNG3AddInputSignal(fer_gng3_t *gng, const fer_vec3_t *is);

/**
 * Adds input signals from file.
 * Returns number of added input signals.
 */
_fer_inline size_t ferGNG3AddInputSignalsFromFile(fer_gng3_t *gng,
                                                   const char *fn);
/**
 * Returns point cloud.
 */
_fer_inline fer_pc3_t *ferGNG3PC(fer_gng3_t *gng);

/**
 * Returns GNG structure.
 */
_fer_inline fer_gng_t *ferGNG3GNG(fer_gng3_t *gng);


/**
 * Net Related API
 * ----------------
 */
struct _fer_gng3_node_t {
    fer_gng_node_t node;  /*!< GNG node */
    fer_vec3_t *w;         /*!< Weight vector */
    fer_cubes3_el_t cubes; /*!< Struct for searching in cubes3 */

    int _id; /*!< Currently useful only for ferGNG3DumpSVT(). */
};
typedef struct _fer_gng3_node_t fer_gng3_node_t;

/**
 * Returns number of nodes in net.
 */
_fer_inline size_t ferGNG3NodesLen(const fer_gng3_t *gng);

/**
 * Cast from Net node to GNG3 node.
 */
_fer_inline fer_gng3_node_t *ferGNG3NodeFromNet(fer_net_node_t *n);

/**
 * Cast from GNG node to GNG3 node.
 */
_fer_inline fer_gng3_node_t *ferGNG3NodeFromGNG(fer_gng_node_t *n);


/**
 * Dumps 3-D network in SVT format.
 */
void ferGNG3DumpSVT(fer_gng3_t *gng, FILE *out, const char *name);

/**** INLINES ****/
_fer_inline void ferGNG3AddInputSignal(fer_gng3_t *gng, const fer_vec3_t *is)
{
    ferPC3Add(gng->pc, is);
}

_fer_inline size_t ferGNG3AddInputSignalsFromFile(fer_gng3_t *gng,
                                                   const char *fn)
{
    return ferPC3AddFromFile(gng->pc, fn);
}

_fer_inline fer_pc3_t *ferGNG3PC(fer_gng3_t *gng)
{
    return gng->pc;
}

_fer_inline fer_gng_t *ferGNG3GNG(fer_gng3_t *gng)
{
    return gng->gng;
}

_fer_inline fer_net_t *ferGNG3Net(fer_gng3_t *gng)
{
    return ferGNGNet(gng->gng);
}

_fer_inline size_t ferGNG3NodesLen(const fer_gng3_t *gng)
{
    return ferGNGNodesLen(gng->gng);
}

_fer_inline fer_gng3_node_t *ferGNG3NodeFromNet(fer_net_node_t *n)
{
    fer_gng_node_t *gn;
    gn = ferGNGNodeFromNet(n);
    return ferGNG3NodeFromGNG(gn);
}

_fer_inline fer_gng3_node_t *ferGNG3NodeFromGNG(fer_gng_node_t *n)
{
    return fer_container_of(n, fer_gng3_node_t, node);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GNG3_H__ */

