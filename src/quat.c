/***
 * Boruvka
 * --------
 * Copyright (c)2012 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <boruvka/quat.h>

void ferQuatSetEuler(bor_quat_t *q, bor_real_t yaw, bor_real_t pitch, bor_real_t roll)
{
    bor_real_t c1, c2, c3, s1, s2, s3;
    bor_real_t w, x, y, z;

    c1 = FER_COS(yaw / FER_REAL(2.));
    c2 = FER_COS(pitch / FER_REAL(2.));
    c3 = FER_COS(roll / FER_REAL(2.));
    s1 = FER_SIN(yaw / FER_REAL(2.));
    s2 = FER_SIN(pitch / FER_REAL(2.));
    s3 = FER_SIN(roll / FER_REAL(2.));

    w = c1 * c2 * c3 - s1 * s2 * s3;
    x = s1 * s2 * c3 + c1 * c2 * s3;
    y = s1 * c2 * c3 + c1 * s2 * s3;
    z = c1 * s2 * c3 - s1 * c2 * s3;

    ferQuatSet(q, x, y, z, w);
}
