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
 * TODO
 */
#define FER_CD_TOP_DOWN 0

/**
 * TODO
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


#endif /* __FER_CD_CONST_H__ */

