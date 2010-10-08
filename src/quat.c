/***
 * mg
 * ---
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of mg.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <mg/quat.h>
#include <mg/alloc.h>

mg_quat_t *mgQuatNew(mg_real_t x, mg_real_t y, mg_real_t z, mg_real_t w)
{
    mg_quat_t *q;

    q = MG_ALLOC(mg_quat_t);
    mgQuatSet(q, x, y, z, w);

    return q;
}

void mgQuatDel(mg_quat_t *q)
{
    free(q);
}
