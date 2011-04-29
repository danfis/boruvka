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

#ifndef __FER_CD_BOX_H__
#define __FER_CD_BOX_H__

#include <fermat/cd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Box shape.
 *
 * Box shape is special case because OBB equals to a box and thus we only
 * need to know that the shape is box-shape.
 */
typedef fer_cd_shape_t fer_cd_box_t;


/**
 * Creates sphere shape
 */
fer_cd_box_t *ferCDBoxNew(void);

/**
 * Deletes sphere.
 */
void ferCDBoxDel(fer_cd_box_t *b);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_BOX_H__ */

