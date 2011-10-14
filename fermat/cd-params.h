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

#ifndef __FER_CD_PARAMS_H__
#define __FER_CD_PARAMS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Each instance of FermatCD must be configured by set of parameters.
 * {fer_cd_params_t} is the structure that serves for this purpose, each
 * parameter is described below.
 *
 * Build flags are described in more detail below in `Build Flags`_ section.
 */
struct _fer_cd_params_t {
    uint32_t build_flags; /*!< Flags that alter building methods.
                               Available flags are:
                                 - FER_CD_TOP_DOWN, FER_CD_BOTTOM_UP
                                 - FER_CD_FIT_COVARIANCE,
                                   FER_CD_FIT_CALIPERS,
                                   FER_CD_FIT_CALIPERS_NUM_ROT(),
                                   FER_CD_FIT_POLYHEDRAL_MASS,
                                   FER_CD_FIT_NAIVE,
                                   FER_CD_FIT_NAIVE_NUM_ROT(),
                                   FER_CD_FIT_COVARIANCE_FAST
                                 - FER_CD_BUILD_PARALLEL()
                               Default: FER_CD_TOP_DOWN
                                            | FER_CD_FIT_COVARIANCE */

    int use_sap; /*!< True if SAP should be used. Default: true */
    int sap_gpu; /*!< True if SAP should run on GPU. Default: false */
    size_t sap_hashsize; /*!< Size of SAP's hash table.
                              Set it to reasonable high number (consider
                              prime number). Default: 1023 */

    size_t max_contacts; /*!< Maximal number of contacts.
                              Default: 20 */
    size_t num_threads; /*!< Number of threads used in parallel versions of
                             some functions. If set >1 then parallelization
                             usign threads will be used whenever possible
                             (if not documented otherwise).
                             Default: 1 */

    unsigned long mpr_max_iterations; /*!< Maximal number of iterations MPR
                                           algorithm can perform.
                                           Default: ULONG_MAX */
    fer_real_t mpr_tolerance;         /*!< Boundary tolerance of MPR
                                           algorithm.
                                           Default: 1E-4 */


    int use_cp;         /*!< If set to true, contact persistence is used.
                             Note that contact persistence is used only for
                             those geoms that were explicitly enabled by
                             [see ferCDGeomContactPersistence() function].
                             Default: true */
    size_t cp_hashsize; /*!< Size of hash table used as register for
                             contact persistence.  If set to 0,
                             {.sap_hashsize} is used instead.
                             Default: 0 */
    fer_real_t cp_max_dist; /*!< Maximal squared distance a contact point
                                 can move to be considered "still" */
};
typedef struct _fer_cd_params_t fer_cd_params_t;

/**
 * Initializes parameters to default values.
 */
void ferCDParamsInit(fer_cd_params_t *params);


/**
 * Build Flags
 * ^^^^^^^^^^^^
 */

/**
 * Uses top-down approach for building OBB tree.
 */
#define FER_CD_TOP_DOWN 0

/**
 * Uses bottom-up approach for building OBB tree.
 */
#define FER_CD_BOTTOM_UP 1

/**
 * Use covariance matrix for fitting OBB to its content.
 */
#define FER_CD_FIT_COVARIANCE (0 << 1)

/**
 * Use "rotation calipers" for fitting OBB. This method is slower and more
 * accurate than {FER_CD_FIT_COVARIANCE}.
 */
#define FER_CD_FIT_CALIPERS (1 << 1)

/**
 * It specifies number of rotation that will be tried for fitting OBB.
 * The higher value is, the more accurate (and slower) method is used.
 * Reasonable values are {10, 20, ..., 50}. This flag is active only if
 * {FER_CD_FIT_CALIPERS} flag is set.
 * Default value is {5}.
 */
#define FER_CD_FIT_CALIPERS_NUM_ROT(rot) ((rot & 0xFF) << 8)

/**
 * Uses "Polyhedral Mass Properties" method for fittin OBB.
 *
 * See Eberly, David. "Polyhedral Mass Properties (Revisited)", Technical
 * Report Magic Software, January 25, 2003
 * http://www.geometrictools.com/Documentation/PolyhedralMassProperties.pdf
 */
#define FER_CD_FIT_POLYHEDRAL_MASS (2 << 1)

/**
 * Uses naive approach for fitting, i.e., creates AABB box and then rotate
 * it about x, y, z axis and choose best fitting box.
 * Use macro {FER_CD_FIT_NAIVE_NUM_ROT()} for setting number of rotations.
 * Note that real number of rotations is {num_rot^3}.
 * Default number of rotations is {3}.
 */
#define FER_CD_FIT_NAIVE (3 << 1)

/**
 * Specifies number of rotations for "naive" fitting algorithm.
 */
#define FER_CD_FIT_NAIVE_NUM_ROT(num_rot) ((num_rot & 0xFF) << 8)

/**
 * Fast fitting algorithm using covariance matrix
 */
#define FER_CD_FIT_COVARIANCE_FAST (4 << 1)


/**
 * Use this flag for building OBB trees in parallel using threads.
 * Default {0}.
 */
#define FER_CD_BUILD_PARALLEL(num_threads) ((num_threads & 0xFF) << 16)


/**** INTERNAL ****/
/*** These macros return true if flag is set ***/
#define __FER_CD_TOP_DOWN(flags)  ((flags & 0x1) == FER_CD_TOP_DOWN)
#define __FER_CD_BOTTOM_UP(flags) ((flags & 0x1) == FER_CD_BOTTOM_UP)

#define __FER_CD_FIT_COVARIANCE(flags) ((flags & 0xE) == FER_CD_FIT_COVARIANCE)
#define __FER_CD_FIT_CALIPERS(flags) ((flags & 0xE) == FER_CD_FIT_CALIPERS)
#define __FER_CD_FIT_POLYHEDRAL_MASS(flags) \
    ((flags & 0xE) == FER_CD_FIT_POLYHEDRAL_MASS)
#define __FER_CD_FIT_NAIVE(flags) ((flags & 0xE) == FER_CD_FIT_NAIVE)
#define __FER_CD_FIT_COVARIANCE_FAST(flags) \
    ((flags & 0xE) == FER_CD_FIT_COVARIANCE_FAST)


/*** Expands to number of rotations ***/
#define __FER_CD_NUM_ROT(flags) ((flags >> 8) & 0xFF)


/** Expands to number of threads that ought to be used */
#define __FER_CD_BUILD_PARALLEL(flags) ((flags >> 16) & 0xFF)



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_CD_H__ */

