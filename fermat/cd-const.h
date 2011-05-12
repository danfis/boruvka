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

#ifndef __FER_CD_CONST_H__
#define __FER_CD_CONST_H__

/**
 * Constants and Macros
 * ---------------------
 */

/**
 * Uses top-down approach for building OBB tree.
 * This is default.
 */
#define FER_CD_TOP_DOWN 0

/**
 * Uses bottom-up approach for building OBB tree.
 */
#define FER_CD_BOTTOM_UP 1

/**
 * Use covariance matrix for fitting OBB to its content.
 * This is default.
 */
#define FER_CD_FIT_COVARIANCE (0 << 1)

/**
 * Use "rotation calipers" for fitting OBB. This method is slower and more
 * accurate than FER_CD_FIT_COVARIANCE.
 */
#define FER_CD_FIT_CALIPERS (1 << 1)

/**
 * It specifies number of rotation that will be tried for fitting OBB.
 * The higher value is, the more accurate (and slower) method is used.
 * Reasonable values are 10, 20, ..., 50. This flag is active only if
 * FER_CD_FIT_CALIPERS flag is set.
 * Default value is 5.
 */
#define FER_CD_FIT_CALIPERS_NUM_ROT(rot) ((rot & 0xFF) << 8)

/**
 * Uses "Polyhedral Mass Properties" method for fittin OBB.
 * See Eberly, David. "Polyhedral Mass Properties (Revisited)", Technical
 *     Report Magic Software, January 25, 2003
 *     http://www.geometrictools.com/Documentation/PolyhedralMassProperties.pdf
 */
#define FER_CD_FIT_POLYHEDRAL_MASS (2 << 1)

/**
 * Uses naive approach for fitting, i.e., creates AABB box and then rotate
 * it about x, y, z axis and choose best fitting box.
 * Use macro FER_CD_FIT_NAIVE_NUM_ROT() for setting number of rotations.
 * Note that real number of rotations is num_rot^3.
 * Default number of rotations is 3.
 */
#define FER_CD_FIT_NAIVE (3 << 1)

/**
 * Specifies number of rotations for "naive" fitting algorithm.
 */
#define FER_CD_FIT_NAIVE_NUM_ROT(num_rot) ((num_rot & 0xFF) << 8)


#define FER_CD_FIT_COVARIANCE_FAST (4 << 1)



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


#endif /* __FER_CD_CONST_H__ */

