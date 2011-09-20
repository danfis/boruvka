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

#include <fermat/cubes`N`.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

struct _fer_cubes`N`_cache_t {
    fer_cubes`N`_el_t **els; /*!< Elements that we are searching for */
    fer_real_t *dist;      /*!< Distances of elements in .els */
    size_t dist_alloc;     /*!< Amount of allocated memory for .dist */
    size_t len;            /*!< Number of so far found elements */
    size_t max_len;        /*!< Maximal number of elements we want to find */

    const fer_vec`N`_t *p;
};
typedef struct _fer_cubes`N`_cache_t fer_cubes`N`_cache_t;

/** Initializes and destroys .cache member of fer_cubes`N`_t */
static void ferCubesCacheInit(fer_cubes`N`_t *cs, fer_cubes`N`_el_t **els,
                              size_t max_len, const fer_vec`N`_t *p);
static void ferCubesCacheDestroy(fer_cubes`N`_t *cs);

/** Searches for nearest points in radius from center. */
static void ferCubesNearestInRadius(fer_cubes`N`_t *cs, size_t *center,
                                     size_t radius);

/** Searches specified area of cubes. Area is specified in cube coordinates
 *  (xmin, xmax, ymin, ymax, zmin, zmax) */
static void ferCubesNearestInArea(fer_cubes`N`_t *cs, size_t *r);

/** Searches specified only specified cube */
static void ferCubesNearestInCube(fer_cubes`N`_t *cs, size_t id);

/** Checks if given element isn't closer than the ones already stored in
 *  cache. */
static void ferCubesNearestCheck(fer_cubes`N`_cache_t *c, fer_cubes`N`_el_t *el);

/** Bubble sort. Takes the last element in .els and bubble it towards
 *  smaller ones (according to .dist[] value). */
static void ferCubesNearestBubbleUp(fer_cubes`N`_cache_t *c);

/** Fills sides[6] with 0 or 1 depending on whether this side of cube area
 *  has to be searched, i.e. 0 means that on that side we are out of
 *  covered area (and we searched that in previous step).
 *  It also fills ranges[6] with range of coordinates (x, y, z) which
 *  covers search area in given radius from center. */
static void ferCubesNearestRangesSides(fer_cubes`N`_t *cs, size_t *center,
                                       size_t radius,
                                       size_t *ranges, unsigned int *sides);

/** Fills pos[`N`] with coordinates of cube where belongs point with given
 *  coordinates. */
_fer_inline void __ferCubesPosCoords(const fer_cubes`N`_t *cs,
                                     const fer_vec`N`_t *coords,
                                     size_t *pos);

/** Converts cube position in grid to cube's ID */
_fer_inline size_t __ferCubesPosToID(const fer_cubes`N`_t *cs, const size_t *pos);


fer_cubes`N`_t *ferCubes`N`New(const fer_real_t *bound, size_t num_cubes)
{
    fer_real_t edge[`N`]; // length of edge along of each axis
    fer_real_t volume; // volume of mapped space
    fer_real_t cvol; // volume of one cube
    size_t len; // length of cs->cubes array
    size_t i;
    fer_cubes`N`_t *cs;

    cs = FER_ALLOC(fer_cubes`N`_t);

    // compute shifting
    cs->shift = ferVec`N`Clone(fer_vec`N`_origin);
    for (i = 0; i < `N`; i++){
        ferVec`N`SetCoord(cs->shift, i, bound[2 * i]);
    }
    ferVec`N`Scale(cs->shift, -FER_ONE);
    //DBG("Cubes: shift[0]: %f", cs->shift[0]);
    //DBG("Cubes: shift[1]: %f", cs->shift[1]);
    //DBG("Cubes: shift[2]: %f", cs->shift[2]);

    // compute dimension of mapped space
    for (i = 0; i < `N`; i++){
        edge[i] = FER_FABS(bound[2 * i + 1] - bound[2 * i]);
    }

    // Compute size of cube:

    // volume of mapped space
    volume = edge[0];
    for (i = 1; i < `N`; i++){
        volume *= edge[i];
    }
    //DBG("Cubes: volume: %f", volume);
    // estimate volume of one cube
    cvol = volume * ferRecp(num_cubes);
    //DBG("Cubes: cvol: %f", cvol);
    // size of cube - third root of volume (for 3-D)
    cs->size = FER_POW(cvol, ferRecp(FER_REAL(`N`.)));
    //DBG("Cubes: size: %f", cs->size);

    // and finally cumpute number of cubes in each direction (x, y, z)
    // there is addition of 1 to be sure that whole space will be mapped to
    // cubes
    for (i = 0; i < `N`; i++){
        cs->dim[i] = (size_t)(edge[i] / cs->size) + (size_t)1;
        //DBG("Cubes: dim[%d]: %d", i, cs->dim[i]);
    }

    // length of array
    len = cs->dim[0];
    for (i = 1; i < `N`; i++){
        len *= cs->dim[i];
    }
    //DBG("Cubes: len: %d", len);

    // allocate and initialize cubes
    cs->cubes = FER_ALLOC_ARR(fer_cubes_cube_t, len);
    cs->cubes_len = len;
    for (i=0; i < len; i++){
        ferCubesCubeInit(&cs->cubes[i]);
    }

    cs->num_els = 0;
    cs->cache = NULL;

    return cs;
}

void ferCubes`N`Del(fer_cubes`N`_t *cs)
{
    size_t i;

    for (i = 0; i < cs->cubes_len; i++){
        ferCubesCubeDestroy(&cs->cubes[i]);
    }

    ferVec`N`Del(cs->shift);

    if (cs->cache)
        ferCubesCacheDestroy(cs);

    FER_FREE(cs->cubes);
    FER_FREE(cs);
}


size_t ferCubes`N`Nearest(fer_cubes`N`_t *cs, const fer_vec`N`_t *p, size_t num,
                          fer_cubes`N`_el_t **els)
{
    size_t center[`N`];
    fer_real_t border;
    int radius;

    // initialize cache
    ferCubesCacheInit(cs, els, FER_MIN(num, cs->num_els), p);

    if (cs->cache->max_len == 0)
        return 0;

    // compute center cube from point p - this is where we start searching
    __ferCubesPosCoords(cs, p, center);

    radius = 0;
    while (1){
        ferCubesNearestInRadius(cs, center, radius);

        border = (fer_real_t)(radius) * cs->size;
        // border must be squared because internaly are distances managed
        // as squared distances
        border *= border;

        // End searching if we have all points we wanted and the furthest
        // one from them is before border, i.e. we are sure there is no
        // nearest point in other cubes.
        if (cs->cache->len == cs->cache->max_len
                && cs->cache->dist[cs->cache->len - 1] < border){
            break;
        }

        radius++;
    }

    return cs->cache->len;
}


static void ferCubesCacheInit(fer_cubes`N`_t *cs, fer_cubes`N`_el_t **els,
                                 size_t max_len, const fer_vec`N`_t *p)
{
    if (cs->cache == NULL){
        cs->cache = FER_ALLOC(fer_cubes`N`_cache_t);
        cs->cache->dist = NULL;
        cs->cache->dist_alloc = 0;
    }

    if (cs->cache->dist_alloc < max_len){
        cs->cache->dist = FER_REALLOC_ARR(cs->cache->dist, fer_real_t, max_len);
        cs->cache->dist_alloc = max_len;
    }

    cs->cache->els     = els;
    cs->cache->len     = 0;
    cs->cache->max_len = max_len;
    cs->cache->p       = p;
}

static void ferCubesCacheDestroy(fer_cubes`N`_t *cs)
{
    if (cs->cache){
        if (cs->cache->dist)
            FER_FREE(cs->cache->dist);
        FER_FREE(cs->cache);
    }
    cs->cache = NULL;
}

static void ferCubesNearestInRadius(fer_cubes`N`_t *cs, size_t *center,
                                     size_t radius)
{
    size_t ranges[2 * `N`];
    unsigned int sides[2 * `N`];
    size_t i, j, p;
    size_t r[2 * `N`];

    // fill ranges and sides
    ferCubesNearestRangesSides(cs, center, radius, ranges, sides);

    for (i = 0; i < 2 * `N`; i++){
        if (!sides[i])
            continue;


        for (j=0; j < 2 * `N`; j++)
            r[j] = ranges[j];

        p = i / 2;

        // shrink current dimension to 1
        r[2 * p] = r[2 * p + 1] = ranges[i];

        if (p >= 1){
            // clip cubes we searched in previous steps
            if (sides[0])
                r[0] += 1;
            if (sides[1])
                r[1] -= 1;
        }

        if (p >= 2){
            // clip cubes we searched in previous steps
            if (sides[2])
                r[2] += 1;
            if (sides[`N`])
                r[`N`] -= 1;
        }

        ferCubesNearestInArea(cs, r);
    }
}

#define FOR_DIM(dim) \
    for (pos[dim] = r[2 * dim]; pos[dim] <= r[2 * dim + 1]; pos[dim]++)

static void ferCubesNearestInArea(fer_cubes`N`_t *cs, size_t *r)
{
    size_t pos[`N`], id;

    // search whole area
    // note that in one axis is dimension always 1
    FOR_DIM(0){
        FOR_DIM(1){
#if `N` >= 3
            FOR_DIM(2){
#endif
                //DBG("(%d %d %d)", pos[0], pos[1], pos[2]);
                // TODO: move this to separate function?
                id = __ferCubesPosToID(cs, pos);
                ferCubesNearestInCube(cs, id);
#if `N` >= 3
            }
#endif
        }
    }
}

static void ferCubesNearestInCube(fer_cubes`N`_t *cs, size_t id)
{
    fer_list_t *list, *item;
    fer_cubes`N`_el_t *el;

    list = ferCubesCubeList(&cs->cubes[id]);
    FER_LIST_FOR_EACH(list, item){
        el = FER_LIST_ENTRY(item, fer_cubes`N`_el_t, list);
        ferCubesNearestCheck(cs->cache, el);
    }
}

static void ferCubesNearestCheck(fer_cubes`N`_cache_t *c, fer_cubes`N`_el_t *el)
{
    fer_real_t dist;

    dist = ferVec`N`Dist2(c->p, el->coords);
    if (c->len < c->max_len){
        c->els[c->len]  = el;
        c->dist[c->len] = dist;
        c->len++;

        ferCubesNearestBubbleUp(c);
    }else if (dist < c->dist[c->len - 1]){
        c->els[c->len - 1]  = el;
        c->dist[c->len - 1] = dist;

        ferCubesNearestBubbleUp(c);
    }
}

static void ferCubesNearestBubbleUp(fer_cubes`N`_cache_t *c)
{
    size_t i;
    fer_real_t tmpd;
    fer_cubes`N`_el_t *tmpn;

    // don't worry, c->len can never be zero
    for (i = c->len - 1; i > 0; i--){
        if (c->dist[i] < c->dist[i - 1]){
            FER_SWAP(c->dist[i], c->dist[i - 1], tmpd);
            FER_SWAP(c->els[i], c->els[i - 1], tmpn);
        }else{
            break;
        }
    }
}

static void ferCubesNearestRangesSides(fer_cubes`N`_t *cs, size_t *center,
                                          size_t radius,
                                          size_t *ranges, unsigned int *sides)
{
    size_t i;
    int coord;

    // set ranges and sides to browse just center
    if (radius == 0){
        for (i=0; i < 2 * `N`; i++){
            sides[i] = 0;
            ranges[i] = center[i / 2];
        }
        sides[0] = 1;

        return;
    }

    // set all sides as enabled
    for (i=0; i < 2 * `N`; i++)
        sides[i] = 1;

    for (i=0; i < `N`; i++){
        coord = (int)center[i] - (int)radius;
        ranges[i * 2] = FER_MAX(coord, 0);
        if (coord < 0)
            sides[i * 2] = 0;

        coord = center[i] + radius;
        ranges[(i * 2) + 1] = FER_MIN(coord, cs->dim[i] - 1);
        if (coord > cs->dim[i] - 1)
            sides[(i * 2) + 1] = 0;
    }
}



size_t __ferCubes`N`IDCoords(const fer_cubes`N`_t *cs, const fer_vec`N`_t *coords)
{
    size_t cube_id, cube_pos[`N`];

    __ferCubesPosCoords(cs, coords, cube_pos);

    // now we have coordinates of cube we are looking for, lets compute
    // actual id
    cube_id = __ferCubesPosToID(cs, cube_pos);

    return cube_id;
}

_fer_inline void __ferCubesPosCoords(const fer_cubes`N`_t *cs,
                                     const fer_vec`N`_t *coords,
                                     size_t *cube_pos)
{
    size_t i;
    fer_vec`N`_t pos; // position in cubes space (shifted position and
                      // aligned with space covered by cubes)

    // compute shifted position
    ferVec`N`Add2(&pos, coords, cs->shift);

    // Align position with cubes boundaries.
    // Border cubes hold vectors which are out of mapped space.
    // To do this shifted coordinates can't run out before 0
#if `N` == 2
    ferVec2Set(&pos, FER_FMAX(ferVec2X(&pos), FER_ZERO),
                     FER_FMAX(ferVec2Y(&pos), FER_ZERO));
#endif
#if `N` == 3
    ferVec3Set(&pos, FER_FMAX(ferVec3X(&pos), FER_ZERO),
                     FER_FMAX(ferVec3Y(&pos), FER_ZERO),
                     FER_FMAX(ferVec3Z(&pos), FER_ZERO));
#endif

    // and if it runs above higher bound of space, last coordinate of
    // cube is picked
    ferVec`N`Scale(&pos, ferRecp((fer_real_t)cs->size));
    for (i = 0; i < `N`; i++){
        cube_pos[i] = (size_t)ferVec`N`Get(&pos, i);
        cube_pos[i] = FER_MIN(cube_pos[i], cs->dim[i] - 1);
    }
}

_fer_inline size_t __ferCubesPosToID(const fer_cubes`N`_t *cs, const size_t *pos)
{
    size_t cube_id, mul;
    size_t i;

    cube_id = pos[0];
    mul = 1;
    for (i = 1; i < `N`; i++){
        mul *= cs->dim[i - 1];
        cube_id += pos[i] * mul;
    }

    return cube_id;
}
