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

#ifndef __FER_GNG_EU_H__
#define __FER_GNG_EU_H__

#include <fermat/vec.h>
#include <fermat/pc.h>
#include <fermat/nncells.h>
#include <fermat/gng.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Growing Neural Gas In Euclidean Metric
 * =======================================
 */



/**
 * GNG-Eu Operations
 * --------------------
 *
 */

/**
 * Initialzes operations struct.
 * All operations except terminate and callback are set.
 */
void ferGNGEuOpsInit(fer_gng_ops_t *ops);



/**
 * GNG-Eu Parameters
 * -------------------
 */
struct _fer_gng_eu_params_t {
    int dim; /*!< Number of dimensions. Default: 2 */

    fer_gng_params_t gng;

    int use_cells; /*!< True if cells should be used for nearest neighbor
                        search. Default: true */
    fer_nncells_params_t cells;
};
typedef struct _fer_gng_eu_params_t fer_gng_eu_params_t;

/**
 * Intializes parameters.
 */
void ferGNGEuParamsInit(fer_gng_eu_params_t *p);


/**
 * GNG-Eu Algorithm
 * -----------------
 */
struct _fer_gng_eu_t {
    int dim;

    fer_gng_t *gng;   /*!< Pointer to GNG structure */
    fer_pc_t *pc;     /*!< Point cloud of input signals */
    fer_pc_it_t pcit; /*!< Iterator over .pc */

    fer_nncells_t *cells; /*!< Cells for nearest neighbor search */
    int use_cells;
    fer_nncells_params_t cells_params;

    fer_vec_t *tmpv;
};
typedef struct _fer_gng_eu_t fer_gng_eu_t;


/**
 * Creates new instance of GNGEu algorithm.
 */
fer_gng_eu_t *ferGNGEuNew(const fer_gng_ops_t *ops,
                          const fer_gng_eu_params_t *params);

/**
 * Deletes GNGEu.
 */
void ferGNGEuDel(fer_gng_eu_t *gng);

/**
 * Runs GNGEu algorithm.
 */
void ferGNGEuRun(fer_gng_eu_t *gng);

/**
 * Adds one input signal.
 */
_fer_inline void ferGNGEuAddInputSignal(fer_gng_eu_t *gng, const fer_vec_t *is);

/**
 * Adds input signals from file.
 * Returns number of added input signals.
 */
_fer_inline size_t ferGNGEuAddInputSignalsFromFile(fer_gng_eu_t *gng,
                                                   const char *fn);

/**
 * Returns point cloud.
 */
_fer_inline fer_pc_t *ferGNGEuPC(fer_gng_eu_t *gng);

/**
 * Returns GNG structure.
 */
_fer_inline fer_gng_t *ferGNGEuGNG(fer_gng_eu_t *gng);


/**
 * Net Related API
 * ----------------
 */
struct _fer_gng_eu_node_t {
    fer_gng_node_t node;    /*!< GNG node */
    fer_vec_t *w;           /*!< Weight vector */
    fer_nncells_el_t cells; /*!< Struct for searching in cubes2 */

    int _id; /*!< Currently useful only for ferGNGEuDumpSVT(). */
};
typedef struct _fer_gng_eu_node_t fer_gng_eu_node_t;

/**
 * Returns number of nodes in net.
 */
_fer_inline size_t ferGNGEuNodesLen(const fer_gng_eu_t *gng);

/**
 * Cast from Net node to GNGEu node.
 */
_fer_inline fer_gng_eu_node_t *ferGNGEuNodeFromNet(fer_net_node_t *n);

/**
 * Cast from GNG node to GNGEu node.
 */
_fer_inline fer_gng_eu_node_t *ferGNGEuNodeFromGNG(fer_gng_node_t *n);


/**
 * Dumps 2-D network in SVT format.
 */
void ferGNGEuDumpSVT(fer_gng_eu_t *gng, FILE *out, const char *name);

/**** INLINES ****/
_fer_inline void ferGNGEuAddInputSignal(fer_gng_eu_t *gng, const fer_vec_t *is)
{
    ferPCAdd(gng->pc, is);
}

_fer_inline size_t ferGNGEuAddInputSignalsFromFile(fer_gng_eu_t *gng,
                                                   const char *fn)
{
    return ferPCAddFromFile(gng->pc, fn);
}

_fer_inline fer_pc_t *ferGNGEuPC(fer_gng_eu_t *gng)
{
    return gng->pc;
}

_fer_inline fer_gng_t *ferGNGEuGNG(fer_gng_eu_t *gng)
{
    return gng->gng;
}

_fer_inline fer_net_t *ferGNGEuNet(fer_gng_eu_t *gng)
{
    return ferGNGNet(gng->gng);
}

_fer_inline size_t ferGNGEuNodesLen(const fer_gng_eu_t *gng)
{
    return ferGNGNodesLen(gng->gng);
}

_fer_inline fer_gng_eu_node_t *ferGNGEuNodeFromNet(fer_net_node_t *n)
{
    fer_gng_node_t *gn;
    gn = ferGNGNodeFromNet(n);
    return ferGNGEuNodeFromGNG(gn);
}

_fer_inline fer_gng_eu_node_t *ferGNGEuNodeFromGNG(fer_gng_node_t *n)
{
    return fer_container_of(n, fer_gng_eu_node_t, node);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GNG_EU_H__ */


