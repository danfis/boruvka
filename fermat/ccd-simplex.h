/***
 * libccd
 * ---------------------------------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *
 *  This file is part of libccd.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __FER_SIMPLEX_H__
#define __FER_SIMPLEX_H__

#include <fermat/ccd-support.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_simplex_t {
    fer_support_t ps[4];
    int last; //!< index of last added point
};
typedef struct _fer_simplex_t fer_simplex_t;


_fer_inline void ferSimplexInit(fer_simplex_t *s);
_fer_inline int ferSimplexSize(const fer_simplex_t *s);
_fer_inline const fer_support_t *ferSimplexLast(const fer_simplex_t *s);
_fer_inline const fer_support_t *ferSimplexPoint(const fer_simplex_t *s, int idx);
_fer_inline fer_support_t *ferSimplexPointW(fer_simplex_t *s, int idx);

_fer_inline void ferSimplexAdd(fer_simplex_t *s, const fer_support_t *v);
_fer_inline void ferSimplexSet(fer_simplex_t *s, size_t pos, const fer_support_t *a);
_fer_inline void ferSimplexSetSize(fer_simplex_t *s, int size);
_fer_inline void ferSimplexSwap(fer_simplex_t *s, size_t pos1, size_t pos2);


/**** INLINES ****/

_fer_inline void ferSimplexInit(fer_simplex_t *s)
{
    s->last = -1;
}

_fer_inline int ferSimplexSize(const fer_simplex_t *s)
{
    return s->last + 1;
}

_fer_inline const fer_support_t *ferSimplexLast(const fer_simplex_t *s)
{
    return ferSimplexPoint(s, s->last);
}

_fer_inline const fer_support_t *ferSimplexPoint(const fer_simplex_t *s, int idx)
{
    // here is no check on boundaries
    return &s->ps[idx];
}
_fer_inline fer_support_t *ferSimplexPointW(fer_simplex_t *s, int idx)
{
    return &s->ps[idx];
}

_fer_inline void ferSimplexAdd(fer_simplex_t *s, const fer_support_t *v)
{
    // here is no check on boundaries in sake of speed
    ++s->last;
    ferSupportCopy(s->ps + s->last, v);
}

_fer_inline void ferSimplexSet(fer_simplex_t *s, size_t pos, const fer_support_t *a)
{
    ferSupportCopy(s->ps + pos, a);
}

_fer_inline void ferSimplexSetSize(fer_simplex_t *s, int size)
{
    s->last = size - 1;
}

_fer_inline void ferSimplexSwap(fer_simplex_t *s, size_t pos1, size_t pos2)
{
    fer_support_t supp;

    ferSupportCopy(&supp, &s->ps[pos1]);
    ferSupportCopy(&s->ps[pos1], &s->ps[pos2]);
    ferSupportCopy(&s->ps[pos2], &supp);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_SIMPLEX_H__ */
