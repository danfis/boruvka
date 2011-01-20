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

/** Clones matrix to gsl_matrix */
static gsl_matrix *ferMatCloneToGSL(const fer_mat_t *m);
#ifdef FER_SINGLE
/** Copies data from gsl_matrix to fer_mat_t matrix */
static void ferMatCopyFromGSL(fer_mat_t *d, gsl_matrix *s);
#endif /* FER_SINGLE */


fer_mat_t *ferMatNew(size_t rows, size_t cols)
{
    fer_mat_t *m;

    m = FER_ALLOC(fer_mat_t);
    ferMatInit(m, rows, cols);

    return m;
}

void ferMatDel(fer_mat_t *m)
{
    ferMatDestroy(m);
    free(m);
}

int ferMatMul(fer_mat_t *d, const fer_mat_t *b)
{
    fer_mat_t *a;
    int ret;

    a = ferMatClone(d);
    ret = ferMatMul2(d, a, b);
    ferMatDel(a);
    return ret;
}


int ferMatMulLeft(fer_mat_t *d, const fer_mat_t *b)
{
    fer_mat_t *a;
    int ret;

    a = ferMatClone(d);
    ret = ferMatMul2(d, b, a);
    ferMatDel(a);
    return ret;
}

fer_real_t ferMatDet(const fer_mat_t *m)
{
    size_t n;
    fer_real_t det;
    int s;
    gsl_matrix *decomp;
    gsl_permutation *perm;

    n = ferMatRows(m);

    // for matrix 1 x 1
    if (n == 1){
        det = ferMatGet(m, 0, 0);
    // for matrix 2 x 2
    }else if (n == 2){
        det  = ferMatGet(m, 0, 0) * ferMatGet(m, 1, 1);
        det -= ferMatGet(m, 0, 1) * ferMatGet(m, 1, 0);
    // TODO for n == 3 and n == 4
    }else{
        decomp = ferMatCloneToGSL(m);
        perm = gsl_permutation_alloc(n);

        gsl_linalg_LU_decomp(decomp, perm, &s);
        det = gsl_linalg_LU_det(decomp, s);

        gsl_matrix_free(decomp);
        gsl_permutation_free(perm);
    }

    return det;
}

int ferMatInv(fer_mat_t *m)
{
    return ferMatInv2(m, m);
}

int ferMatInv2(fer_mat_t *m, const fer_mat_t *a)
{
    size_t rows;
    int s;
    gsl_matrix *decomp;
    gsl_permutation *perm;
#ifdef FER_SINGLE
    gsl_matrix *inv;
#endif /* FER_SINGLE */

    __FER_CHECKDIM(m, a);

    rows = ferMatRows(m);
    if (rows != ferMatCols(m))
        return -1;

    decomp = ferMatCloneToGSL(a);
    perm = gsl_permutation_alloc(rows);

    gsl_linalg_LU_decomp(decomp, perm, &s);

#ifdef FER_SINGLE
    inv = gsl_matrix_alloc(rows, rows);
    gsl_linalg_LU_invert(decomp, perm, inv);
    ferMatCopyFromGSL(m, inv);
    gsl_matrix_free(inv);
#else /* FER_SINGLE */
    gsl_linalg_LU_invert(decomp, perm, m->m);
#endif /* FER_SINGLE */

    gsl_permutation_free(perm);
    gsl_matrix_free(decomp);

    return 0;
}



static gsl_matrix *ferMatCloneToGSL(const fer_mat_t *m)
{
    gsl_matrix *g;

#ifdef FER_SINGLE
    size_t i, j, rows, cols;

    g = gsl_matrix_alloc(ferMatRows(m), ferMatCols(m));
    rows = ferMatRows(m);
    cols = ferMatCols(m);
    for (i = 0; i < rows; i++){
        for (j = 0; j < cols; j++){
            gsl_matrix_set(g, i, j, ferMatGet(m, i, j));
        }
    }

#else /* FER_SINGLE */
    g = gsl_matrix_alloc(ferMatRows(m), ferMatCols(m));
    gsl_matrix_memcpy(g, m->m);
#endif /* FER_SINGLE */

    return g;
}

#ifdef FER_SINGLE
static void ferMatCopyFromGSL(fer_mat_t *d, gsl_matrix *s)
{
    size_t i, j, rows, cols;

    rows = ferMatRows(d);
    cols = ferMatCols(d);
    for (i = 0; i < rows; i++){
        for (j = 0; j < cols; j++){
            ferMatSet(d, i, j, gsl_matrix_get(s, i, j));
        }
    }
}
#endif /* FER_SINGLE */
