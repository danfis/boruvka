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

#ifndef __FER_CUBES3_H__
#define __FER_CUBES3_H__

#include <fermat/core.h>
#include <fermat/vec3.h>
#include <fermat/nearest/cubes3_internal.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_cubes3_cache_t;

/**
 * Cubes structure for nearest neighbor search in 3D space.
 *
 * TODO: example
 */
struct _fer_cubes3_t {
    fer_real_t size;   /*!< Size of edge of one cube. */
    size_t dim[3];     /*!< How many cubes are along x, y or z axis
                            respectively. */
    fer_vec3_t *shift; /*!< Shifting of points stored in cubes.
                            For easiest navigation in cubes, cubes structure
                            is built from origin towards x, y, z axis.
                            This vector is always added to point coordinates
                            that are searched to move it into space covered by
                            cubes. */

    fer_cubes3_cube_t *cubes; /*! Array of all cubes - it has (.dim[0] *
                                  .dim[1] * .dim[2]) elements. */
    size_t cubes_len;         /*! Length of .cubes array. */

    size_t num_els; /*!< Number of elements stored in cubes */

    struct _fer_cubes3_cache_t *cache;
};
typedef struct _fer_cubes3_t fer_cubes3_t;


/**
 * Structure for connecting element into cubes structure.
 *
 * TODO: example
 */
struct _fer_cubes3_el_t {
    fer_list_t list; /*!< List struct which is used for connection to cubes */
    size_t cube_id;  /*!< Id of cube where is element currently registered,
                          i.e. .list is connected into fer_cubes3_t's
                          .cubes[.cube_id] cube. */

    const fer_vec3_t *coords; /*!< Coordinates of element. Assign pointer
                                   to your vector here. */
};
typedef struct _fer_cubes3_el_t fer_cubes3_el_t;


/**
 * Initializes fer_cubes3_el_t structure.
 * No "destroy" functions needs to be called.
 */
_fer_inline void ferCubes3ElInit(fer_cubes3_el_t *el, const fer_vec3_t *coords);


/**
 * Creates and initializes cubes structure.
 * First argument (bound) is bounding box (xmin, xmax, ymin, ymax, zmin,
 * zmax) that should be covered by cubes.
 * Second argument (num_cubes) is number of cubes that should be created,
 * note that this is only initial guess - finite number of cubes can be
 * little bit more.
 */
fer_cubes3_t *ferCubes3New(const fer_real_t *bound, size_t num_cubes);

/**
 * Deletes cubes structure.
 */
void ferCubes3Del(fer_cubes3_t *c);

/**
 * Returns number of cubes stored in structure.
 */
_fer_inline size_t ferCubes3Len(const fer_cubes3_t *c);

/**
 * Returns size (length) of edge of one cube.
 */
_fer_inline fer_real_t ferCubes3Size(const fer_cubes3_t *c);

/**
 * Returns number of cubes alogn x, y, z axis.
 */
_fer_inline const size_t *ferCubes3Dim(const fer_cubes3_t *c);

/**
 * Returns number of elements stored in cubes.
 */
_fer_inline size_t ferCubes3NumEls(const fer_cubes3_t *c);


/**
 * Adds element to cubes according to its coordinates in space.
 * This function should be called only once.
 *
 * First parameter is pointer to cubes structure.
 * Second parameter is cubes3 element structure - this structure is used
 * for connection into cubes structure.
 * Third parameter is coordinates of element.
 */
_fer_inline void ferCubes3Add(fer_cubes3_t *cs, fer_cubes3_el_t *el);

/**
 * Removes element from cubes.
 *
 * First parameter is point to cubes structure.
 * Second parameter is pointer to list struct using which were element
 * connected in cubes.
 */
_fer_inline void ferCubes3Remove(fer_cubes3_t *cs, fer_cubes3_el_t *el);

/**
 * Updates elements position in cubes.
 * Call this function anytime element is moved. If you don't do that
 * position of element in cubes could be incorrect.
 *
 * Parameters are same as in ferCubes3Add().
 */
_fer_inline void ferCubes3Update(fer_cubes3_t *cs, fer_cubes3_el_t *el);

/**
 * This function si similar to ferCubes3Update() but given element is
 * always removed from cubes and then added again. No checking if
 * re-registering is needed is performed.
 */
_fer_inline void ferCubes3UpdateForce(fer_cubes3_t *cs, fer_cubes3_el_t *el);


/**
 * Finds *num* nearest elements to given point *p*.
 *
 * Array of pointers els must be allocated and must have at least *num*
 * elements. This array is filled with pointers to elements that are
 * nearest to point *p*. Number of found elements is returned.
 */
size_t ferCubes3Nearest(fer_cubes3_t *cs, const fer_vec3_t *p, size_t num,
                        fer_cubes3_el_t **els);



/**
 * Returns cube id (position in .cubes array) where belongs point with
 * given coordinates.
 * This function _always_ returns correct ID.
 */
_fer_inline size_t __ferCubes3IDCoords(const fer_cubes3_t *cs,
                                       const fer_vec3_t *coords);

/**
 * Fills pos[3] with coordinates of cube where belongs point with given
 * coordinates.
 */
_fer_inline void __ferCubes3PosCoords(const fer_cubes3_t *cs,
                                      const fer_vec3_t *coords,
                                      size_t *pos);


/**** INLINES ****/
_fer_inline void ferCubes3ElInit(fer_cubes3_el_t *el, const fer_vec3_t *coords)
{
    ferListInit(&el->list);
    el->cube_id = (size_t)-1;
    el->coords = coords;
}

_fer_inline size_t ferCubes3Len(const fer_cubes3_t *c)
{
    return c->cubes_len;
}

_fer_inline fer_real_t ferCubes3Size(const fer_cubes3_t *c)
{
    return c->size;
}

_fer_inline const size_t *ferCubes3Dim(const fer_cubes3_t *c)
{
    return c->dim;
}

_fer_inline size_t ferCubes3NumEls(const fer_cubes3_t *c)
{
    return c->num_els;
}

_fer_inline void ferCubes3Add(fer_cubes3_t *cs, fer_cubes3_el_t *el)
{
    size_t id;

    id = __ferCubes3IDCoords(cs, el->coords);
    ferCubes3CubeAdd(&cs->cubes[id], &el->list);
    el->cube_id = id;
    cs->num_els++;
}

_fer_inline void ferCubes3Remove(fer_cubes3_t *cs, fer_cubes3_el_t *el)
{
    ferCubes3CubeRemove(&el->list);
    el->cube_id = (size_t)-1;
    cs->num_els--;
}

_fer_inline void ferCubes3Update(fer_cubes3_t *cs, fer_cubes3_el_t *el)
{
    size_t correct_id;

    correct_id = __ferCubes3IDCoords(cs, el->coords);
    if (correct_id != el->cube_id){
        ferCubes3Remove(cs, el);
        ferCubes3Add(cs, el);
    }
}

_fer_inline void ferCubes3UpdateForce(fer_cubes3_t *cs, fer_cubes3_el_t *el)
{
    ferCubes3Remove(cs, el);
    ferCubes3Add(cs, el);
}

_fer_inline size_t __ferCubes3IDCoords(const fer_cubes3_t *cs,
                                       const fer_vec3_t *coords)
{
    size_t cube_id, cube_pos[3];

    __ferCubes3PosCoords(cs, coords, cube_pos);

    // now we have coordinates of cube we are looking for, lets compute
    // actual id
    cube_id = cube_pos[0]
                + cube_pos[1] * cs->dim[0]
                + cube_pos[2] * cs->dim[0] * cs->dim[1];

    return cube_id;
}

_fer_inline void __ferCubes3PosCoords(const fer_cubes3_t *cs,
                                      const fer_vec3_t *coords,
                                      size_t *cube_pos)
{
    fer_vec3_t pos; // position in cubes space (shifted position and
                    // aligned with space covered by cubes)

    // compute shifted position
    ferVec3Add2(&pos, coords, cs->shift);

    // Align position with cubes boundaries.
    // Border cubes hold vectors which are out of mapped space.
    // To do this shifted coordinates can't run out before 0
    ferVec3Set(&pos, FER_FMAX(ferVec3X(&pos), FER_ZERO),
                     FER_FMAX(ferVec3Y(&pos), FER_ZERO),
                     FER_FMAX(ferVec3Z(&pos), FER_ZERO));

    // and if it runs above higher bound of space, last coordinate of
    // cube is picked
    ferVec3Scale(&pos, ferRecp((fer_real_t)cs->size));
    cube_pos[0] = (size_t)ferVec3X(&pos);
    cube_pos[1] = (size_t)ferVec3Y(&pos);
    cube_pos[2] = (size_t)ferVec3Z(&pos);
    cube_pos[0] = FER_MIN(cube_pos[0], cs->dim[0] - 1);
    cube_pos[1] = FER_MIN(cube_pos[1], cs->dim[1] - 1);
    cube_pos[2] = FER_MIN(cube_pos[2], cs->dim[2] - 1);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_CUBES3_H__ */

