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

#include "fermat/mat.h"
#include "fermat/alloc.h"

fer_mat_t *ferMatNew(size_t rows, size_t cols)
{
    fer_mat_t *m;

    m = FER_ALLOC(fer_mat_t);
    m->m = fer_gsl_matrix_alloc(rows, cols);

    return m;
}

void ferMatDel(fer_mat_t *m)
{
    fer_gsl_matrix_free(m->m);
    free(m);
}
