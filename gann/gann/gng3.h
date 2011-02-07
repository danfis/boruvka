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

#ifndef __FER_GANN_GNG3_H__
#define __FER_GANN_GNG3_H__

#include <fermat/vec3.h>
#include <fermat/pc3.h>
#include <fermat/cubes3.h>
#include <gann/gng.h>

/**
 * Growing Neural Gas In 3-D
 * ==========================
 */



/**
 * GNG 3D Operations
 * ---------------
 *
 * This is similar to gann_gng_ops_t.
 *
 * See gann_gng3_ops_t.
 */

struct _gann_gng3_ops_t {
    gann_gng_terminate terminate; /*!< Termination condition */
    gann_gng_callback callback;
    unsigned long callback_period;

    void *data; /*!< Data for all callbacks if not specified otherwise */
    void *terminate_data;
    void *callback_data;
};
typedef struct _gann_gng3_ops_t gann_gng3_ops_t;

/**
 * Initialzes operations struct.
 */
void gannGNG3OpsInit(gann_gng3_ops_t *ops);



/**
 * GNG 3-D Parameters
 * -------------------
 */
struct _gann_gng3_params_t {
    gann_gng_params_t gng;

    int use_cubes;    /*!< Set to true to enable using cubes3 for nearest
                           neighbor search. [default true] */
    size_t num_cubes; /*!< Number of cubes3 used for nearest neighbor
                           search. [default 10000] */
};
typedef struct _gann_gng3_params_t gann_gng3_params_t;

/**
 * Intializes parameters.
 */
void gannGNG3ParamsInit(gann_gng3_params_t *p);


/**
 * GNG 3D Algorithm
 * -----------------
 */
struct _gann_gng3_t {
    gann_gng_t *gng;     /*!< Pointer to GNG structure */
    fer_pc3_t *pc;       /*!< Point cloud - input signals */
    fer_pc3_it_t pcit;   /*!< Iterator over .pc */
    fer_cubes3_t *cubes; /*!< Cubes3 for nearest neighbor search */

    gann_gng3_ops_t ops;
    gann_gng3_params_t params;
};
typedef struct _gann_gng3_t gann_gng3_t;


/**
 * Creates new instance of GNG3 algorithm.
 */
gann_gng3_t *gannGNG3New(const gann_gng3_ops_t *ops,
                         const gann_gng3_params_t *params);

/**
 * Deletes GNG3.
 */
void gannGNG3Del(gann_gng3_t *gng);

/**
 * Runs GNG3 algorithm.
 */
void gannGNG3Run(gann_gng3_t *gng);

/**
 * Adds one input signal.
 */
_fer_inline void gannGNG3AddInputSignal(gann_gng3_t *gng, const fer_vec3_t *is);

/**
 * Adds input signals from file.
 * Returns number of added input signals.
 */
_fer_inline size_t gannGNG3AddInputSignalsFromFile(gann_gng3_t *gng,
                                                   const char *fn);
/**
 * Returns point cloud.
 */
_fer_inline fer_pc3_t *gannGNG3PC(gann_gng3_t *gng);

/**
 * Returns GNG structure.
 */
_fer_inline gann_gng_t *gannGNG3GNG(gann_gng3_t *gng);


/**
 * Net Related API
 * ----------------
 */
struct _gann_gng3_node_t {
    gann_gng_node_t node;  /*!< GNG node */
    fer_vec3_t *w;         /*!< Weight vector */
    fer_cubes3_el_t cubes; /*!< Struct for searching in cubes3 */

    int _id; /*!< Currently useful only for gannGNG3DumpSVT(). */
};
typedef struct _gann_gng3_node_t gann_gng3_node_t;

/**
 * Returns number of nodes in net.
 */
_fer_inline size_t gannGNG3NodesLen(const gann_gng3_t *gng);

/**
 * Cast from Net node to GNG3 node.
 */
_fer_inline gann_gng3_node_t *gannGNG3NodeFromNet(gann_net_node_t *n);

/**
 * Cast from GNG node to GNG3 node.
 */
_fer_inline gann_gng3_node_t *gannGNG3NodeFromGNG(gann_gng_node_t *n);


/**
 * Dumps 3-D network in SVT format.
 */
void gannGNG3DumpSVT(gann_gng3_t *gng, FILE *out, const char *name);

/**** INLINES ****/
_fer_inline void gannGNG3AddInputSignal(gann_gng3_t *gng, const fer_vec3_t *is)
{
    ferPC3Add(gng->pc, is);
}

_fer_inline size_t gannGNG3AddInputSignalsFromFile(gann_gng3_t *gng,
                                                   const char *fn)
{
    return ferPC3AddFromFile(gng->pc, fn);
}

_fer_inline fer_pc3_t *gannGNG3PC(gann_gng3_t *gng)
{
    return gng->pc;
}

_fer_inline gann_gng_t *gannGNG3GNG(gann_gng3_t *gng)
{
    return gng->gng;
}

_fer_inline gann_net_t *gannGNG3Net(gann_gng3_t *gng)
{
    return gannGNGNet(gng->gng);
}

_fer_inline size_t gannGNG3NodesLen(const gann_gng3_t *gng)
{
    return gannGNGNodesLen(gng->gng);
}

_fer_inline gann_gng3_node_t *gannGNG3NodeFromNet(gann_net_node_t *n)
{
    gann_gng_node_t *gn;
    gn = gannGNGNodeFromNet(n);
    return gannGNG3NodeFromGNG(gn);
}

_fer_inline gann_gng3_node_t *gannGNG3NodeFromGNG(gann_gng_node_t *n)
{
    return fer_container_of(n, gann_gng3_node_t, node);
}
#endif /* __FER_GANN_GNG3_H__ */

