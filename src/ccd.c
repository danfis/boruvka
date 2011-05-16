/***
 * fermat
 * -------
 * Copyright (c)2010,2011 Daniel Fiser <danfis@danfis.cz>
 *
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

#include <fermat/ccd.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>


void ferCCDFirstDirDefault(const void *o1, const void *o2, fer_vec3_t *dir)
{
    ferVec3Set(dir, FER_ONE, FER_ZERO, FER_ZERO);
}

void ferCCDInit(fer_ccd_t *ccd)
{
    ccd->first_dir = ferCCDFirstDirDefault;
    ccd->support1 = NULL;
    ccd->support2 = NULL;
    ccd->center1  = NULL;
    ccd->center2  = NULL;

    ccd->max_iterations = (unsigned long)-1;
    ccd->epa_tolerance = FER_REAL(0.0001);
    ccd->mpr_tolerance = FER_REAL(0.0001);
}
