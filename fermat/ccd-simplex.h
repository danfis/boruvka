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

#ifndef __FER_CCD_SIMPLEX_H__
#define __FER_CCD_SIMPLEX_H__

#include <fermat/ccd-support.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_ccd_simplex_t {
    fer_ccd_support_t ps[4];
    int last; /*!< index of last added point */
};
typedef struct _fer_ccd_simplex_t fer_ccd_simplex_t;


_fer_inline void ferCCDSimplexInit(fer_ccd_simplex_t *s);
_fer_inline int ferCCDSimplexSize(const fer_ccd_simplex_t *s);
_fer_inline const fer_ccd_support_t *ferCCDSimplexLast(const fer_ccd_simplex_t *s);
_fer_inline const fer_ccd_support_t *ferCCDSimplexPoint(const fer_ccd_simplex_t *s, int idx);
_fer_inline fer_ccd_support_t *ferCCDSimplexPointW(fer_ccd_simplex_t *s, int idx);

_fer_inline void ferCCDSimplexAdd(fer_ccd_simplex_t *s, const fer_ccd_support_t *v);
_fer_inline void ferCCDSimplexSet(fer_ccd_simplex_t *s, size_t pos, const fer_ccd_support_t *a);
_fer_inline void ferCCDSimplexSetSize(fer_ccd_simplex_t *s, int size);
_fer_inline void ferCCDSimplexSwap(fer_ccd_simplex_t *s, size_t pos1, size_t pos2);


/**** INLINES ****/

_fer_inline void ferCCDSimplexInit(fer_ccd_simplex_t *s)
{
    s->last = -1;
}

_fer_inline int ferCCDSimplexSize(const fer_ccd_simplex_t *s)
{
    return s->last + 1;
}

_fer_inline const fer_ccd_support_t *ferCCDSimplexLast(const fer_ccd_simplex_t *s)
{
    return ferCCDSimplexPoint(s, s->last);
}

_fer_inline const fer_ccd_support_t *ferCCDSimplexPoint(const fer_ccd_simplex_t *s, int idx)
{
    /* here is no check on boundaries */
    return &s->ps[idx];
}
_fer_inline fer_ccd_support_t *ferCCDSimplexPointW(fer_ccd_simplex_t *s, int idx)
{
    return &s->ps[idx];
}

_fer_inline void ferCCDSimplexAdd(fer_ccd_simplex_t *s, const fer_ccd_support_t *v)
{
    /* here is no check on boundaries in sake of speed */
    ++s->last;
    ferCCDSupportCopy(s->ps + s->last, v);
}

_fer_inline void ferCCDSimplexSet(fer_ccd_simplex_t *s, size_t pos, const fer_ccd_support_t *a)
{
    ferCCDSupportCopy(s->ps + pos, a);
}

_fer_inline void ferCCDSimplexSetSize(fer_ccd_simplex_t *s, int size)
{
    s->last = size - 1;
}

_fer_inline void ferCCDSimplexSwap(fer_ccd_simplex_t *s, size_t pos1, size_t pos2)
{
    fer_ccd_support_t supp;

    ferCCDSupportCopy(&supp, &s->ps[pos1]);
    ferCCDSupportCopy(&s->ps[pos1], &s->ps[pos2]);
    ferCCDSupportCopy(&s->ps[pos2], &supp);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_CCD_SIMPLEX_H__ */
