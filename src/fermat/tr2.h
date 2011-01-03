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

#ifndef __FER_TR2_H__
#define __FER_TR2_H__

/**
 * Tr2 structure is implementation of transform matrix which can be used to
 * transform vectors, points, polylines or whole maps.
 */

#include <fermat/vec2.h>


/**
 * Object using which is possible to tranform vectors and others.
 * Mainly it contains transform matrix, of course.
 */
struct _fer_tr2_t {
    fer_real_t m[9]; /*! Transform matrix (3x3) */
    fer_real_t *tmpm, *tmpmc; /*! Temporary matrices */
    fer_real_t *tmpv, *tmpv2; /*! Temporary 3D vectors. */
};
typedef struct _fer_tr2_t fer_tr2_t;


/**
 * Constructor and destructor.
 */
fer_tr2_t *ferTr2New(void);
void ferTr2Del(fer_tr2_t *);

_fer_inline fer_tr2_t *ferTr2Clone(const fer_tr2_t *tr);

/**
 * Copies transformation from s into d.
 */
_fer_inline void ferTr2Copy(fer_tr2_t *d, const fer_tr2_t *s);


_fer_inline fer_real_t ferTr2Get(const fer_tr2_t *tr, int row, int col);
_fer_inline void ferTr2Set(fer_tr2_t *tr, int row, int col, fer_real_t val);


/**
 * Set up fer_tr2_t to identity matrix
 */
_fer_inline void ferTr2Identity(fer_tr2_t *tr);

/**
 * Apply rotation about origin onto transform matrix
 */
void ferTr2Rotate(fer_tr2_t *, fer_real_t angle);

/**
 * Apply rotation about variable center onto transform matrix.
 */
void ferTr2RotateCenter(fer_tr2_t *, fer_real_t angle, const fer_vec2_t *center);

/**
 * Apply translation onto transform matrix.
 */
void ferTr2TranslateCoords(fer_tr2_t *, fer_real_t dx, fer_real_t dy);
_fer_inline void ferTr2Translate(fer_tr2_t *tr, const fer_vec2_t *v);

/**
 * Scaling in x and y coordinates.
 */
void ferTr2Scale(fer_tr2_t *tr, fer_real_t sx, fer_real_t sy);

/**
 * Do composition of transform matrices, where matrix comp is applied onto
 * matrix tr:
 *      tr = comp * tr
 */
void ferTr2Compose(fer_tr2_t *tr, const fer_tr2_t *comp);


/**
 * Apply transform matrix onto given vector.
 */
void ferTr2(const fer_tr2_t *tr, fer_vec2_t *v);


/***** INLINES *****/
_fer_inline void ferTr2Copy(fer_tr2_t *d, const fer_tr2_t *s)
{
    size_t i;
    for (i = 0; i < 9; i++)
        d->m[i] = s->m[i];
}

_fer_inline fer_tr2_t *ferTr2Clone(const fer_tr2_t *tr)
{
    fer_tr2_t *t;
    t = ferTr2New();
    ferTr2Copy(t, tr);
    return t;
}

_fer_inline fer_real_t ferTr2Get(const fer_tr2_t *tr, int row, int col)
{
    return tr->m[3 * row + col];
}
_fer_inline void ferTr2Set(fer_tr2_t *tr, int row, int col, fer_real_t val)
{
    tr->m[3 * row + col] = val;
}

_fer_inline void ferTr2Identity(fer_tr2_t *tr)
{
    tr->m[0] = tr->m[4] = tr->m[8] = FER_ONE;
    tr->m[1] = tr->m[2] = tr->m[3] = tr->m[5] = tr->m[6] = tr->m[7] = FER_ZERO;
}

_fer_inline void ferTr2Translate(fer_tr2_t *tr, const fer_vec2_t *v)
{
    ferTr2TranslateCoords(tr, ferVec2X(v), ferVec2Y(v));
}


#endif /* __FER_TR2_H__ */
