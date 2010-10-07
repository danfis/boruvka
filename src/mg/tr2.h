#ifndef __MG_TR2_H__
#define __MG_TR2_H__

/**
 * Tr2 structure is implementation of transform matrix which can be used to
 * transform vectors, points, polylines or whole maps.
 */

#include <mg/core.h>
#include <mg/vec2.h>


/**
 * Object using which is possible to tranform vectors and others.
 * Mainly it contains transform matrix, of course.
 */
struct _mg_tr2_t {
    mg_real_t m[9]; /*! Transform matrix (3x3) */
    mg_real_t *tmpm, *tmpmc; /*! Temporary matrices */
    mg_real_t *tmpv, *tmpv2; /*! Temporary 3D vectors. */
};
typedef struct _mg_tr2_t mg_tr2_t;


/**
 * Constructor and destructor.
 */
mg_tr2_t *mgTr2New(void);
void mgTr2Del(mg_tr2_t *);

_mg_inline mg_tr2_t *mgTr2Clone(const mg_tr2_t *tr);

/**
 * Copies transformation from s into d.
 */
_mg_inline void mgTr2Copy(mg_tr2_t *d, const mg_tr2_t *s);


_mg_inline mg_real_t mgTr2Get(const mg_tr2_t *tr, int row, int col);
_mg_inline void mgTr2Set(mg_tr2_t *tr, int row, int col, mg_real_t val);


/**
 * Set up mg_tr2_t to identity matrix
 */
_mg_inline void mgTr2Identity(mg_tr2_t *tr);

/**
 * Apply rotation about origin onto transform matrix
 */
void mgTr2Rotate(mg_tr2_t *, mg_real_t angle);

/**
 * Apply rotation about variable center onto transform matrix.
 */
void mgTr2RotateCenter(mg_tr2_t *, mg_real_t angle, const mg_vec2_t *center);

/**
 * Apply translation onto transform matrix.
 */
void mgTr2TranslateCoords(mg_tr2_t *, mg_real_t dx, mg_real_t dy);
_mg_inline void mgTr2Translate(mg_tr2_t *tr, const mg_vec2_t *v);

/**
 * Scaling in x and y coordinates.
 */
void mgTr2Scale(mg_tr2_t *tr, mg_real_t sx, mg_real_t sy);

/**
 * Do composition of transform matrices, where matrix comp is applied onto
 * matrix tr:
 *      tr = comp * tr
 */
void mgTr2Compose(mg_tr2_t *tr, const mg_tr2_t *comp);


/**
 * Apply transform matrix onto given vector.
 */
void mgTr2(const mg_tr2_t *tr, mg_vec2_t *v);


/***** INLINES *****/
_mg_inline void mgTr2Copy(mg_tr2_t *d, const mg_tr2_t *s)
{
    size_t i;
    for (i = 0; i < 9; i++)
        d->m[i] = s->m[i];
}

_mg_inline mg_tr2_t *mgTr2Clone(const mg_tr2_t *tr)
{
    mg_tr2_t *t;
    t = mgTr2New();
    mgTr2Copy(t, tr);
    return t;
}

_mg_inline mg_real_t mgTr2Get(const mg_tr2_t *tr, int row, int col)
{
    return tr->m[3 * row + col];
}
_mg_inline void mgTr2Set(mg_tr2_t *tr, int row, int col, mg_real_t val)
{
    tr->m[3 * row + col] = val;
}

_mg_inline void mgTr2Identity(mg_tr2_t *tr)
{
    tr->m[0] = tr->m[4] = tr->m[8] = MG_ONE;
    tr->m[1] = tr->m[2] = tr->m[3] = tr->m[5] = tr->m[6] = tr->m[7] = MG_ZERO;
}

_mg_inline void mgTr2Translate(mg_tr2_t *tr, const mg_vec2_t *v)
{
    mgTr2TranslateCoords(tr, mgVec2X(v), mgVec2Y(v));
}


#endif /* __MG_TR2_H__ */
