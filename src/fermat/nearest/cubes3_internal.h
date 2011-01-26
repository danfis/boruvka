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

#ifndef __FER_CUBES3_INTERNAL_H__
#define __FER_CUBES3_INTERNAL_H__

#include <fermat/core.h>
#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Struct representing one single cube.
 * A size of this cube is stored in fer_cubes3_t, functions related to this
 * struct do just single cube related searching, i.e. handle somehow list
 * stored in there.
 */
struct _fer_cubes3_cube_t {
    fer_list_t list; /*! List of elements. */
};
typedef struct _fer_cubes3_cube_t fer_cubes3_cube_t;

/**
 * Initializes cube.
 * It basicaly initializes a list.
 */
_fer_inline void ferCubes3CubeInit(fer_cubes3_cube_t *cube);

/**
 * Destroys cube, i.e. free list (detach all elements).
 */
_fer_inline void ferCubes3CubeDestroy(fer_cubes3_cube_t *cube);

/**
 * Returns pointer to list of elements stored in cube.
 */
_fer_inline fer_list_t *ferCubes3CubeList(fer_cubes3_cube_t *c);

/**
 * Returns true if given element is already registered in cube.
 */
_fer_inline int ferCubes3CubeHas(const fer_cubes3_cube_t *c, fer_list_t *el);

/**
 * Add given element to cube.
 * No check if given element is stored in other is performed!
 */
_fer_inline void ferCubes3CubeAdd(fer_cubes3_cube_t *c, fer_list_t *el);

/**
 * Removes element from cube.
 * No checks are performed - this is basically wrapper around ferListDel().
 */
_fer_inline void ferCubes3CubeRemove(fer_list_t *el);


/**** INLINES ****/
_fer_inline void ferCubes3CubeInit(fer_cubes3_cube_t *cube)
{
    ferListInit(&cube->list);
}

_fer_inline void ferCubes3CubeDestroy(fer_cubes3_cube_t *cube)
{
    fer_list_t *el;

    while (!ferListEmpty(&cube->list)){
        el = ferListNext(&cube->list);
        ferListDel(el);
    }
}

_fer_inline fer_list_t *ferCubes3CubeList(fer_cubes3_cube_t *c)
{
    return &c->list;
}

_fer_inline int ferCubes3CubeHas(const fer_cubes3_cube_t *c, fer_list_t *el)
{
    fer_list_t *i;
    ferListForEach(&c->list, i){
        if (i == el)
            return 1;
    }
    return 0;
}

_fer_inline void ferCubes3CubeAdd(fer_cubes3_cube_t *c, fer_list_t *el)
{
    ferListAppend(&c->list, el);
}

_fer_inline void ferCubes3CubeRemove(fer_list_t *el)
{
    ferListDel(el);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_CUBES3_INTERNAL_H__ */


