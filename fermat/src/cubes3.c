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

#include <fermat/cubes3.h>
#include <fermat/alloc.h>

struct _fer_cubes3_cache_t {
    fer_cubes3_el_t **els; /*!< Elements that we are searching for */
    fer_real_t *dist;      /*!< Distances of elements in .els */
    size_t dist_alloc;     /*!< Amount of allocated memory for .dist */
    size_t len;            /*!< Number of so far found elements */
    size_t max_len;        /*!< Maximal number of elements we want to find */

    const fer_vec3_t *p;
};
typedef struct _fer_cubes3_cache_t fer_cubes3_cache_t;

/** Initializes and destroys .cache member of fer_cubes3_t */
static void ferCubes3CacheInit(fer_cubes3_t *cs, fer_cubes3_el_t **els,
                               size_t max_len, const fer_vec3_t *p);
static void ferCubes3CacheDestroy(fer_cubes3_t *cs);

/** Searches for nearest points in radius from center. */
static void ferCubes3NearestInRadius(fer_cubes3_t *cs, size_t *center,
                                     size_t radius);

/** Searches specified area of cubes. Area is specified in cube coordinates
 *  (xmin, xmax, ymin, ymax, zmin, zmax) */
static void ferCubes3NearestInArea(fer_cubes3_t *cs, size_t *r);

/** Searches specified only specified cube */
static void ferCubes3NearestInCube(fer_cubes3_t *cs, size_t id);

/** Checks if given element isn't closer than the ones already stored in
 *  cache. */
static void ferCubes3NearestCheck(fer_cubes3_cache_t *c, fer_cubes3_el_t *el);

/** Bubble sort. Takes the last element in .els and bubble it towards
 *  smaller ones (according to .dist[] value). */
static void ferCubes3NearestBubbleUp(fer_cubes3_cache_t *c);

/** Fills sides[6] with 0 or 1 depending on whether this side of cube area
 *  has to be searched, i.e. 0 means that on that side we are out of
 *  covered area (and we searched that in previous step).
 *  It also fills ranges[6] with range of coordinates (x, y, z) which
 *  covers search area in given radius from center. */
static void ferCubes3NearestRangesSides(fer_cubes3_t *cs, size_t *center,
                                        size_t radius,
                                        size_t *ranges, unsigned int *sides);

/** Fills pos[3] with coordinates of cube where belongs point with given
 *  coordinates. */
_fer_inline void __ferCubes3PosCoords(const fer_cubes3_t *cs,
                                      const fer_vec3_t *coords,
                                      size_t *pos);


fer_cubes3_t *ferCubes3New(const fer_real_t *bound, size_t num_cubes)
{
    fer_real_t edge[3]; // length of edge along of each axis
    fer_real_t volume; // volume of mapped space
    fer_real_t cvol; // volume of one cube
    size_t len; // length of cs->cubes array
    size_t i;
    fer_cubes3_t *cs;

    cs = FER_ALLOC(fer_cubes3_t);

    // compute shifting
    cs->shift = ferVec3New(bound[0], bound[2], bound[4]);
    ferVec3Scale(cs->shift, -FER_ONE);
    //DBG("Cubes: shift[0]: %f", cs->shift[0]);
    //DBG("Cubes: shift[1]: %f", cs->shift[1]);
    //DBG("Cubes: shift[2]: %f", cs->shift[2]);

    // compute dimension of mapped space
    edge[0] = FER_FABS(bound[1] - bound[0]);
    edge[1] = FER_FABS(bound[3] - bound[2]);
    edge[2] = FER_FABS(bound[5] - bound[4]);

    // Compute size of cube:

    // volume of mapped space
    volume = edge[0] * edge[1] * edge[2];
    //DBG("Cubes: volume: %f", volume);
    // estimate volume of one cube
    cvol = volume * ferRecp(num_cubes);
    //DBG("Cubes: cvol: %f", cvol);
    // size of cube - third root of volume
    cs->size = FER_POW(cvol, ferRecp(FER_REAL(3.)));
    //DBG("Cubes: size: %f", cs->size);

    // and finally cumpute number of cubes in each direction (x, y, z)
    // there is addition of 1 to be sure that whole space will be mapped to
    // cubes
    for (i = 0; i < 3; i++){
        cs->dim[i] = (size_t)(edge[i] / cs->size) + (size_t)1;
        //DBG("Cubes: dim[%d]: %d", i, cs->dim[i]);
    }

    // length of array
    len = cs->dim[0] * cs->dim[1] * cs->dim[2];
    //DBG("Cubes: len: %d", len);

    // allocate and initialize cubes
    cs->cubes = FER_ALLOC_ARR(fer_cubes3_cube_t, len);
    cs->cubes_len = len;
    for (i=0; i < len; i++){
        ferCubes3CubeInit(&cs->cubes[i]);
    }

    cs->num_els = 0;
    cs->cache = NULL;

    return cs;
}

void ferCubes3Del(fer_cubes3_t *cs)
{
    size_t i;

    for (i = 0; i < cs->cubes_len; i++){
        ferCubes3CubeDestroy(&cs->cubes[i]);
    }

    ferVec3Del(cs->shift);

    if (cs->cache)
        ferCubes3CacheDestroy(cs);

    free(cs->cubes);
    free(cs);
}


size_t ferCubes3Nearest(fer_cubes3_t *cs, const fer_vec3_t *p, size_t num,
                        fer_cubes3_el_t **els)
{
    size_t center[3];
    fer_real_t border;
    int radius;

    // initialize cache
    ferCubes3CacheInit(cs, els, FER_MIN(num, cs->num_els), p);

    if (cs->cache->max_len == 0)
        return 0;

    // compute center cube from point p - this is where we start searching
    __ferCubes3PosCoords(cs, p, center);

    border = FER_ZERO;
    radius = 0;
    while (1){
        ferCubes3NearestInRadius(cs, center, radius);

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


static void ferCubes3CacheInit(fer_cubes3_t *cs, fer_cubes3_el_t **els,
                               size_t max_len, const fer_vec3_t *p)
{
    if (cs->cache == NULL){
        cs->cache = FER_ALLOC(fer_cubes3_cache_t);
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

static void ferCubes3CacheDestroy(fer_cubes3_t *cs)
{
    if (cs->cache){
        if (cs->cache->dist)
            free(cs->cache->dist);
        free(cs->cache);
    }
    cs->cache = NULL;
}

static void ferCubes3NearestInRadius(fer_cubes3_t *cs, size_t *center,
                                     size_t radius)
{
    size_t ranges[6];
    unsigned int sides[6];
    size_t i, j, p;
    size_t r[6];

    // fill ranges and sides
    ferCubes3NearestRangesSides(cs, center, radius, ranges, sides);

    for (i = 0; i < 6; i++){
        if (!sides[i])
            continue;


        for (j=0; j < 6; j++)
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
            if (sides[3])
                r[3] -= 1;
        }

        ferCubes3NearestInArea(cs, r);
    }
}

#define FOR_DIM(dim) \
    for (pos[dim] = r[2 * dim]; pos[dim] <= r[2 * dim + 1]; pos[dim]++)

static void ferCubes3NearestInArea(fer_cubes3_t *cs, size_t *r)
{
    size_t pos[3], id;

    // search whole area
    // note that in one axis is dimension always 1
    FOR_DIM(0){
        FOR_DIM(1){
            FOR_DIM(2){
                //DBG("(%d %d %d)", pos[0], pos[1], pos[2]);
                // TODO: move this to separate function?
                id = pos[0]
                        + pos[1] * cs->dim[0]
                        + pos[2] * cs->dim[0] * cs->dim[1];
                ferCubes3NearestInCube(cs, id);
            }
        }
    }
}

static void ferCubes3NearestInCube(fer_cubes3_t *cs, size_t id)
{
    fer_list_t *list, *item;
    fer_cubes3_el_t *el;

    list = ferCubes3CubeList(&cs->cubes[id]);
    ferListForEach(list, item){
        el = ferListEntry(item, fer_cubes3_el_t, list);
        ferCubes3NearestCheck(cs->cache, el);
    }
}

static void ferCubes3NearestCheck(fer_cubes3_cache_t *c, fer_cubes3_el_t *el)
{
    fer_real_t dist;

    dist = ferVec3Dist2(c->p, el->coords);
    if (c->len < c->max_len){
        c->els[c->len]  = el;
        c->dist[c->len] = dist;
        c->len++;

        ferCubes3NearestBubbleUp(c);
    }else if (dist < c->dist[c->len - 1]){
        c->els[c->len - 1]  = el;
        c->dist[c->len - 1] = dist;

        ferCubes3NearestBubbleUp(c);
    }
}

static void ferCubes3NearestBubbleUp(fer_cubes3_cache_t *c)
{
    size_t i;
    fer_real_t tmpd;
    fer_cubes3_el_t *tmpn;

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

static void ferCubes3NearestRangesSides(fer_cubes3_t *cs, size_t *center,
                                        size_t radius,
                                        size_t *ranges, unsigned int *sides)
{
    size_t i;
    int coord;

    // set ranges and sides to browse just center
    if (radius == 0){
        for (i=0; i < 6; i++){
            sides[i] = 0;
            ranges[i] = center[i / 2];
        }
        sides[0] = 1;

        return;
    }

    // set all sides as enabled
    for (i=0; i < 6; i++)
        sides[i] = 1;

    for (i=0; i < 3; i++){
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



size_t __ferCubes3IDCoords(const fer_cubes3_t *cs, const fer_vec3_t *coords)
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
