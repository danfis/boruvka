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

#ifndef __FER_CD_H__
#define __FER_CD_H__

#include <fermat/core.h>
#include <fermat/vec3.h>
#include <fermat/mat3.h>
#include <fermat/quat.h>
#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Internal structure - base class for shapes */
struct _fer_cd_shape_t {
    char type;
};
typedef struct _fer_cd_shape_t fer_cd_shape_t;

/** Shape types */
#define FER_CD_SHAPE_TRIMESH 1
#define FER_CD_SHAPE_TRIMESH_TRI 2

#ifdef __cplusplus
}
#endif /* __cplusplus */


#include <fermat/cd-trimesh.h>
#include <fermat/cd-obb.h>
#include <fermat/cd-geom.h>

#endif /* __FER_CD_H__ */
