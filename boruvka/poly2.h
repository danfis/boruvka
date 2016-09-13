/***
 * Boruvka
 * --------
 * Copyright (c)2016 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_POLY2_H__
#define __BOR_POLY2_H__

#include <boruvka/vec2.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Poly2 - Polygon
 * =================
 *
 * .. c:type:: bor_poly2_t
 */

/** vvvvv */
struct _bor_poly2_t {
    int size;
    bor_real_t *px;
    bor_real_t *py;
    bor_real_t *constant;
    bor_real_t *multiple;
};
typedef struct _bor_poly2_t bor_poly2_t;


/**
 * Functions
 * ----------
 */


/**
 * Allocate and initialize a new polygon.
 */
bor_poly2_t *borPoly2New(const bor_vec2_t *corners, int size);
bor_poly2_t *borPoly2New2(const bor_vec2_t *corners, int *idx, int size);

/**
 * Delete polygon.
 */
void borPoly2Del(bor_poly2_t *);

/**
 * Initialize a new polygon.
 */
void borPoly2Init(bor_poly2_t *p, const bor_vec2_t *corners, int size);
void borPoly2Init2(bor_poly2_t *p, const bor_vec2_t *corners,
                   int *idx, int size);

/**
 * Free polygon structure.
 */
void borPoly2Free(bor_poly2_t *);

/**
 * Returns true if the point v is inside the polygon.
 */
int borPoly2PointIn(const bor_poly2_t *poly, const bor_vec2_t *v);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_POLY2_H__ */
