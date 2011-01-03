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

#include <fermat/alloc.h>
#include <fermat/tr2.h>


#define NEED_TMPM(tr) \
    if ((tr)->tmpm == NULL) (tr)->tmpm = FER_ALLOC_ARR(fer_real_t, 9)
#define NEED_TMPMC(tr) \
    if ((tr)->tmpmc == NULL) (tr)->tmpmc = FER_ALLOC_ARR(fer_real_t, 9)
#define NEED_TMPV(tr) \
    if ((tr)->tmpv == NULL) (tr)->tmpv = FER_ALLOC_ARR(fer_real_t, 3)
#define NEED_TMPV2(tr) \
    if ((tr)->tmpv2 == NULL) (tr)->tmpv2 = FER_ALLOC_ARR(fer_real_t, 3)

_fer_inline void identity(fer_real_t *m)
{
    m[0] = m[4] = m[8] = FER_ONE;
    m[1] = m[2] = m[3] = m[5] = m[6] = m[7] = FER_ZERO;
}

_fer_inline void set(fer_real_t *m, int row, int col, fer_real_t val)
{
    m[3 * row + col] = val;
}

_fer_inline void copy(fer_real_t *d, const fer_real_t *s)
{
    size_t i;
    for (i = 0; i < 9; i++)
        d[i] = s[i];
}

/**
 * Composes matrix A with matrix B according to this equation:
 *      A = B * A
 * Temporary matrix tmp is used.
 */
_fer_inline void compose(fer_real_t *A, const fer_real_t *B, fer_real_t *tmp)
{
    copy(tmp, A);

    A[0] = B[0] * tmp[0] + B[1] * tmp[3] + B[2] * tmp[6];
    A[1] = B[0] * tmp[1] + B[1] * tmp[4] + B[2] * tmp[7];
    A[2] = B[0] * tmp[2] + B[1] * tmp[5] + B[2] * tmp[8];

    A[3] = B[3] * tmp[0] + B[4] * tmp[3] + B[5] * tmp[6];
    A[4] = B[3] * tmp[1] + B[4] * tmp[4] + B[5] * tmp[7];
    A[5] = B[3] * tmp[2] + B[4] * tmp[5] + B[5] * tmp[8];

    A[6] = B[6] * tmp[0] + B[7] * tmp[3] + B[8] * tmp[6];
    A[7] = B[6] * tmp[1] + B[7] * tmp[4] + B[8] * tmp[7];
    A[8] = B[6] * tmp[2] + B[7] * tmp[5] + B[8] * tmp[8];
}

fer_tr2_t *ferTr2New(void)
{
    fer_tr2_t *tr = FER_ALLOC(fer_tr2_t);

    tr->tmpm = NULL;
    tr->tmpmc = NULL;
    tr->tmpv = NULL;
    tr->tmpv2 = NULL;

    ferTr2Identity(tr);

    return tr;
}

void ferTr2Del(fer_tr2_t *tr)
{
    if (tr->tmpm != NULL)
        free(tr->tmpm);
    if (tr->tmpmc != NULL)
        free(tr->tmpmc);
    if (tr->tmpv != NULL)
        free(tr->tmpv);
    if (tr->tmpv2 != NULL)
        free(tr->tmpv2);

    free(tr);
}

void ferTr2Rotate(fer_tr2_t *tr, fer_real_t angle)
{
    /**
     * Basic rotation matrix:
     *      | cos(angle) -sin(angle) 0 |
     *      | sin(angle) cos(angle)  0 |
     *      |      0         0       1 |
     */
    NEED_TMPM(tr);
    NEED_TMPMC(tr);

    // set up rotation matrix
    identity(tr->tmpm);
    set(tr->tmpm, 0, 0, FER_COS(angle));
    set(tr->tmpm, 0, 1, FER_REAL(-1.) * FER_SIN(angle));
    set(tr->tmpm, 1, 0, FER_SIN(angle));
    set(tr->tmpm, 1, 1, FER_COS(angle));

    compose(tr->m, tr->tmpm, tr->tmpmc);
}

void ferTr2RotateCenter(fer_tr2_t *tr, fer_real_t angle, const fer_vec2_t *center)
{
    ferTr2TranslateCoords(tr, FER_REAL(-1.) * ferVec2X(center), FER_REAL(-1.) * ferVec2Y(center));
    ferTr2Rotate(tr, angle);
    ferTr2Translate(tr, center);
}

void ferTr2TranslateCoords(fer_tr2_t *tr, fer_real_t dx, fer_real_t dy)
{
    /**
     * Translate matrix using given vector
     * Basic translation matrix is:
     *      | 1 0 tx |
     *      | 0 1 ty |
     *      | 0 0 1  |
     */
    NEED_TMPM(tr);
    NEED_TMPMC(tr);

    identity(tr->tmpm);
    set(tr->tmpm, 0, 2, dx);
    set(tr->tmpm, 1, 2, dy);
    compose(tr->m, tr->tmpm, tr->tmpmc);
}

void ferTr2Scale(fer_tr2_t *tr, fer_real_t sx, fer_real_t sy)
{
    /**
     * | sx 0  0 |
     * | 0  sy 0 |
     * | 0  0  1. |
     */
    NEED_TMPM(tr);
    NEED_TMPMC(tr);

    identity(tr->tmpm);
    set(tr->tmpm, 0, 0, sx);
    set(tr->tmpm, 1, 1, sy);
    set(tr->tmpm, 2, 2, 1.);
    compose(tr->m, tr->tmpm, tr->tmpmc);
}

void ferTr2Compose(fer_tr2_t *tr, const fer_tr2_t *comp)
{
    NEED_TMPMC(tr);

    compose(tr->m, comp->m, tr->tmpmc);
}

void ferTr2(const fer_tr2_t *_tr, fer_vec2_t *v)
{
    fer_tr2_t *tr = (fer_tr2_t *)_tr;
    fer_real_t k;

    NEED_TMPV(tr);
    NEED_TMPV2(tr);

    tr->tmpv[0] = ferVec2X(v);
    tr->tmpv[1] = ferVec2Y(v);
    tr->tmpv[2] = FER_ONE;

    tr->tmpv2[0] = tr->m[0] * tr->tmpv[0] + tr->m[1] * tr->tmpv[1] + tr->m[2] * tr->tmpv[2];
    tr->tmpv2[1] = tr->m[3] * tr->tmpv[0] + tr->m[4] * tr->tmpv[1] + tr->m[5] * tr->tmpv[2];
    tr->tmpv2[2] = tr->m[6] * tr->tmpv[0] + tr->m[7] * tr->tmpv[1] + tr->m[8] * tr->tmpv[2];

    if (ferNEq(tr->tmpv2[2], FER_ZERO)){
        k = FER_REAL(1.) / tr->tmpv2[2];
        tr->tmpv2[0] *= k;
        tr->tmpv2[1] *= k;
        tr->tmpv2[2] *= k;
    }

    ferVec2Set(v, tr->tmpv2[0], tr->tmpv2[1]);
}

