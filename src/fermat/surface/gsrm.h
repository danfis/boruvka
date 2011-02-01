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

#ifndef __FER_GSRM_H__
#define __FER_GSRM_H__

#include <fermat/core.h>
#include <fermat/point_cloud.h>
#include <fermat/mesh/mesh3.h>
#include <fermat/nearest/cubes3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_gsrm_cache_t;

struct _fer_gsrm_param_t {
    size_t lambda;
    fer_real_t eb, en;
    fer_real_t alpha, beta;
    int age_max;
    size_t max_nodes;
    size_t num_cubes;

    fer_real_t min_dangle; /*! minimal dihedral angle between faces */
    fer_real_t max_angle; /*! max angle between nodes to form face */
    fer_real_t angle_merge_edges;

    // simplification:
    fer_real_t simpl_dist_treshold; /*! max distance of node from average
                                        plane to be removed */
    fer_real_t simpl_max_node_dec; /*! max. node decimation */
    fer_real_t simpl_max_face_dec; /*! max. face decimation */
};
typedef struct _fer_gsrm_param_t fer_gsrm_param_t;

/**
 * TODO
 */
struct _fer_gsrm_t {
    fer_gsrm_param_t param; /*!< Parameters of algorithm */

    fer_pc_t *is;        /*!< Input signals */
    fer_pc_it_t isit;    /*!< Iterator over is */
    fer_mesh3_t *mesh;   /*!< Reconstructed mesh */
    fer_cubes3_t *cubes; /*!< Search structure for nearest neighbor */

    struct _fer_gsrm_cache_t *c; /*!< Internal cache, don't touch it! */
};
typedef struct _fer_gsrm_t fer_gsrm_t;


/**
 * Allocates core struct and initializes to default values.
 */
fer_gsrm_t *ferGSRMNew(void);

/**
 * Deallocates struct.
 */
void ferGSRMDel(fer_gsrm_t *g);

/**
 * Adds input signals from given file.
 * Returns number of read points.
 */
size_t ferGSRMAddInputSignals(fer_gsrm_t *g, const char *fn);

/**
 * Runs GSRM algorithm.
 * Returns 0 on success.
 * Returns -1 if no there are no input signals.
 */
int ferGSRMRun(fer_gsrm_t *g);

/**
 * Performs postprocessing of mesh.
 * This function should be called _after_ ferGSRMRun().
 */
int ferGSRMPostprocess(fer_gsrm_t *g);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GSRM_H__ */

