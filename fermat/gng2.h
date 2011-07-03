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

#ifndef __FER_GNG2_H__
#define __FER_GNG2_H__

#include <fermat/vec2.h>
#include <fermat/pc2.h>
#include <fermat/cubes2.h>
#include <fermat/gng.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Growing Neural Gas In 2-D
 * ==========================
 */



/**
 * GNG 2D Operations
 * --------------------
 *
 */

/**
 * Initialzes operations struct.
 * All operations except terminate and callback are set.
 */
void ferGNG2OpsInit(fer_gng_ops_t *ops);



/**
 * GNG 2-D Parameters
 * -------------------
 */
struct _fer_gng2_params_t {
    fer_gng_params_t gng;

    int use_cubes;    /*!< Set to true to enable using cubes2 for nearest
                           neighbor search. [default true] */
    size_t num_cubes; /*!< Number of cubes2 used for nearest neighbor
                           search. [default 10000] */
};
typedef struct _fer_gng2_params_t fer_gng2_params_t;

/**
 * Intializes parameters.
 */
void ferGNG2ParamsInit(fer_gng2_params_t *p);


/**
 * GNG 2D Algorithm
 * -----------------
 */
struct _fer_gng2_t {
    fer_gng_t *gng;      /*!< Pointer to GNG structure */
    fer_pc2_t *pc;       /*!< Point cloud - input signals */
    fer_pc2_it_t pcit;   /*!< Iterator over .pc */

    fer_cubes2_t *cubes; /*!< Cubes2 for nearest neighbor search */
    size_t num_cubes;
};
typedef struct _fer_gng2_t fer_gng2_t;


/**
 * Creates new instance of GNG2 algorithm.
 */
fer_gng2_t *ferGNG2New(const fer_gng_ops_t *ops,
                       const fer_gng2_params_t *params);

/**
 * Deletes GNG2.
 */
void ferGNG2Del(fer_gng2_t *gng);

/**
 * Runs GNG2 algorithm.
 */
void ferGNG2Run(fer_gng2_t *gng);

/**
 * Adds one input signal.
 */
_fer_inline void ferGNG2AddInputSignal(fer_gng2_t *gng, const fer_vec2_t *is);

/**
 * Adds input signals from file.
 * Returns number of added input signals.
 */
_fer_inline size_t ferGNG2AddInputSignalsFromFile(fer_gng2_t *gng,
                                                  const char *fn);

/**
 * Returns point cloud.
 */
_fer_inline fer_pc2_t *ferGNG2PC(fer_gng2_t *gng);

/**
 * Returns GNG structure.
 */
_fer_inline fer_gng_t *ferGNG2GNG(fer_gng2_t *gng);


/**
 * Net Related API
 * ----------------
 */
struct _fer_gng2_node_t {
    fer_gng_node_t node;  /*!< GNG node */
    fer_vec2_t *w;         /*!< Weight vector */
    fer_cubes2_el_t cubes; /*!< Struct for searching in cubes2 */

    int _id; /*!< Currently useful only for ferGNG2DumpSVT(). */
};
typedef struct _fer_gng2_node_t fer_gng2_node_t;

/**
 * Returns number of nodes in net.
 */
_fer_inline size_t ferGNG2NodesLen(const fer_gng2_t *gng);

/**
 * Cast from Net node to GNG2 node.
 */
_fer_inline fer_gng2_node_t *ferGNG2NodeFromNet(fer_net_node_t *n);

/**
 * Cast from GNG node to GNG2 node.
 */
_fer_inline fer_gng2_node_t *ferGNG2NodeFromGNG(fer_gng_node_t *n);


/**
 * Dumps 2-D network in SVT format.
 */
void ferGNG2DumpSVT(fer_gng2_t *gng, FILE *out, const char *name);

/**** INLINES ****/
_fer_inline void ferGNG2AddInputSignal(fer_gng2_t *gng, const fer_vec2_t *is)
{
    ferPC2Add(gng->pc, is);
}

_fer_inline size_t ferGNG2AddInputSignalsFromFile(fer_gng2_t *gng,
                                                   const char *fn)
{
    return ferPC2AddFromFile(gng->pc, fn);
}

_fer_inline fer_pc2_t *ferGNG2PC(fer_gng2_t *gng)
{
    return gng->pc;
}

_fer_inline fer_gng_t *ferGNG2GNG(fer_gng2_t *gng)
{
    return gng->gng;
}

_fer_inline fer_net_t *ferGNG2Net(fer_gng2_t *gng)
{
    return ferGNGNet(gng->gng);
}

_fer_inline size_t ferGNG2NodesLen(const fer_gng2_t *gng)
{
    return ferGNGNodesLen(gng->gng);
}

_fer_inline fer_gng2_node_t *ferGNG2NodeFromNet(fer_net_node_t *n)
{
    fer_gng_node_t *gn;
    gn = ferGNGNodeFromNet(n);
    return ferGNG2NodeFromGNG(gn);
}

_fer_inline fer_gng2_node_t *ferGNG2NodeFromGNG(fer_gng_node_t *n)
{
    return fer_container_of(n, fer_gng2_node_t, node);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GNG2_H__ */

