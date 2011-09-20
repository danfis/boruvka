/***
 * fermat
 * -------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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

#include <stdio.h>
#include <fermat/alloc.h>
#include <fermat/vec.h>
#include <fermat/dbg.h>

fer_vec_t *ferVecNew(size_t size)
{
    fer_vec_t *v;
    v = FER_ALLOC_ARR(fer_vec_t, size);
    return v;
}

void ferVecDel(fer_vec_t *v)
{
    FER_FREE(v);
}
