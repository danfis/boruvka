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

#ifndef __FER_CD_ODE_H__
#define __FER_CD_ODE_H__

#include <fermat/cd.h>
#include <ode/ode.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * The simple wrappers for easier integration with ODE were made.
 *
 * It requires to include additional *fermat/cd-ode.h* header file:
 * ~~~~
 * #include <fermat/cd-ode.h>
 * ~~~~
 */

/**
 * Register geom to ODE's body.
 * It uses dBody's data pointer and 'moved' callback and geom's data2 pointer.
 */
_fer_inline void ferCDGeomSetBody(fer_cd_t *cd, fer_cd_geom_t *g, dBodyID body);

/**
 * Transforms ODE's rotation matrix into fermat's 3x3 matrix
 */
_fer_inline void ferMat3FromODERot(fer_mat3_t *m, const dReal *rot);

/** BodyMoved callback */
static void __ferCDGeomBodyMoved(dBodyID body);

/**** INLINES ****/
_fer_inline void ferCDGeomSetBody(fer_cd_t *cd, fer_cd_geom_t *g, dBodyID body)
{
    ferCDGeomSetData2(g, (void *)cd);
    dBodySetData(body, (void *)g);
    dBodySetMovedCallback(body, __ferCDGeomBodyMoved);
    __ferCDGeomBodyMoved(body);
}

_fer_inline void ferMat3FromODERot(fer_mat3_t *m, const dReal *rot)
{
    ferMat3Set(m, rot[0], rot[1], rot[2],
                  rot[4], rot[5], rot[6],
                  rot[8], rot[9], rot[10]);
}

static void __ferCDGeomBodyMoved(dBodyID body)
{
    fer_cd_geom_t *g = (fer_cd_geom_t *)dBodyGetData(body);
    fer_cd_t *cd = (fer_cd_t *)ferCDGeomData2(g);
    fer_mat3_t rot;
    const dReal *pos;

    pos = dBodyGetPosition(body);
    ferMat3FromODERot(&rot, dBodyGetRotation(body));

    ferCDGeomSetTr3(cd, g, pos[0], pos[1], pos[2]);
    ferCDGeomSetRot(cd, g, &rot);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_ODE_H__ */
