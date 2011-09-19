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

#include <fermat/gug.h>
#include <fermat/alloc.h>
#include <fermat/vec.h>
#include <fermat/vec3.h>
#include <fermat/dbg.h>
#include <fermat/nn.h>


struct _fer_gug_cache_t {
    fer_gug_el_t **els; /*!< Elements that we are searching for */
    fer_real_t *dist;       /*!< Distances of elements in .els */
    size_t len;             /*!< Number of so far found elements */
    size_t max_len;         /*!< Maximal number of elements we want to find */

    const fer_vec_t *p;

    fer_real_t __dist[4];   /*!< Preallocated array on stack for .dist[] to
                                 avoid allocation on heap */
};
typedef struct _fer_gug_cache_t fer_gug_cache_t;

static void cellInit(fer_gug_t *cs, fer_gug_cell_t *c, size_t id);

static void cellsAlloc(fer_gug_t *cs, size_t num_cells);

/** Creates and destroys cache */
static void cacheInit(fer_gug_cache_t *cache,
                      fer_gug_el_t **els,
                      size_t max_len,
                      const fer_vec_t *p);
static void cacheDestroy(fer_gug_cache_t *cache);


/** Nearest nodes in given radius around center */
static int nearestInRadius(const fer_gug_t *cs, fer_gug_cache_t *cache,
                           int radius,
                           const size_t *center, size_t *pos);
/** For 2D */
static int nearestInRadius2(const fer_gug_t *cs, fer_gug_cache_t *cache,
                            int radius,
                            const size_t *center, size_t *pos);
/** Searches only specified cube */
static void nearestInCell(const fer_gug_t *cs, fer_gug_cache_t *cache,
                          fer_gug_cell_t *c);
/** Checks if given element isn't closer than the ones already stored in
 *  cache. */
static void nearestCheck(const fer_gug_t *cs, fer_gug_cache_t *cache,
                         fer_gug_el_t *el);
/** Bubble sort. Takes the last element in .els and bubble it towards
 *  smaller ones (according to .dist[] value). */
static void nearestBubbleUp(fer_gug_cache_t *c);
/** Returns distance of initial border. */
_fer_inline fer_real_t initBorder(const fer_gug_t *cs, const fer_vec_t *p);


/** Converts position in cells to cell's ID */
_fer_inline size_t __ferGUGPosToID(const fer_gug_t *cs,
                                       const size_t *pos);
_fer_inline size_t __ferGUGPosToID2(const fer_gug_t *cs,
                                        const size_t *pos);
/** Invert function to __ferGUGPosToID() */
_fer_inline void __ferGUGIDToPos(const fer_gug_t *cs, size_t id,
                                     size_t *pos);


void ferGUGParamsInit(fer_gug_params_t *p)
{
    p->dim         = 2;
    p->num_cells   = 10000;
    p->max_dens    = 1;
    p->expand_rate = FER_REAL(2.);
    p->aabb        = NULL;
    p->approx      = 0;
}


fer_gug_t *ferGUGNew(const fer_gug_params_t *params)
{
    size_t i;
    fer_gug_t *c;

    c = FER_ALLOC(fer_gug_t);
    c->type = FER_NN_GUG;

    c->num_els = 0;

    c->d = params->dim;
    if (params->num_cells > 0){
        c->max_dens = 0;
        c->expand   = FER_ZERO;
    }else{
        c->max_dens = params->max_dens;
        c->expand   = params->expand_rate;
    }

    c->aabb = FER_ALLOC_ARR(fer_real_t, c->d * 2);
    for (i = 0; i < c->d * 2; i++){
        c->aabb[i] = params->aabb[i];
    }

    // Compute shifting
    c->shift = FER_ALLOC_ARR(fer_real_t, c->d);
    for (i = 0; i < c->d; i++){
        c->shift[i] = -FER_ONE * c->aabb[2 * i];
    }


    c->dim = FER_ALLOC_ARR(size_t, c->d);
    if (params->num_cells > 0){
        cellsAlloc(c, params->num_cells);
    }else{
        cellsAlloc(c, 1);
    }

    c->approx = params->approx;

    return c;
}

void ferGUGDel(fer_gug_t *c)
{
    if (c->dim)
        free(c->dim);
    if (c->shift)
        free(c->shift);
    if (c->aabb)
        free(c->aabb);

    if (c->cells){
        free(c->cells);
    }

    free(c);
}


static size_t __ferGUGNearest(const fer_gug_t *cs, const fer_vec_t *p,
                                  size_t num, fer_gug_el_t **els,
                                  int approx)
{
    size_t center_id, retlen;
    size_t *center, *pos;
    fer_gug_cell_t *cell;
    fer_gug_cache_t cache;
    int radius;
    fer_real_t border, border2;

    if (ferGUGSize(cs) == 0)
        return 0;

    center = FER_ALLOC_ARR(size_t, cs->d);
    pos    = FER_ALLOC_ARR(size_t, cs->d);

    cacheInit(&cache, els, num, p);

    center_id = __ferGUGCoordsToID(cs, p);
    __ferGUGIDToPos(cs, center_id, center);

    // search in center first
    cell = &cs->cells[center_id];
    nearestInCell(cs, &cache, cell);

    border  = initBorder(cs, p);
    border2 = FER_CUBE(border);

    radius = 1;
    while (1){
        // End searching if we have all points we wanted and the furthest
        // one from them is before border, i.e. we are sure there is no
        // nearest point in other cells.
        if (cache.len == cache.max_len
                && (approx || cache.dist[cache.len - 1] < border2)){
            break;
        }

        if (cs->d == 2){
            if (nearestInRadius2(cs, &cache, radius, center, pos) != 0)
                break;
        }else{
            if (nearestInRadius(cs, &cache, radius, center, pos) != 0)
                break;
        }

        border += cs->edge;
        // border must be squared because internaly are distances managed
        // as squared distances
        border2 = FER_CUBE(border);

        radius++;
    }

    retlen = cache.len;

    free(center);
    free(pos);

    cacheDestroy(&cache);

    return retlen;
}

size_t ferGUGNearest(const fer_gug_t *cs, const fer_vec_t *p, size_t num,
                         fer_gug_el_t **els)
{
    return __ferGUGNearest(cs, p, num, els, cs->approx);
}

size_t ferGUGNearestApprox(const fer_gug_t *cs, const fer_vec_t *p,
                               size_t num, fer_gug_el_t **els)
{
    return __ferGUGNearest(cs, p, num, els, 1);
}

void __ferGUGExpand(fer_gug_t *cs)
{
    fer_gug_cell_t *cells;
    size_t i, cells_len, newlen;
    fer_list_t *item;
    fer_gug_el_t *el;

    // save old cells
    cells     = cs->cells;
    cells_len = cs->cells_len;

    // create new cells
    cs->num_els = 0;
    newlen = cells_len * cs->expand;
    if (newlen == cells_len){
        newlen *= 2;
    }
    cellsAlloc(cs, newlen);

    // copy elements from old cells to the new one
    for (i = 0; i < cells_len; i++){
        while (!ferListEmpty(&cells[i].list)){
            item = ferListNext(&cells[i].list);
            el   = FER_LIST_ENTRY(item, fer_gug_el_t, list);
            ferListDel(item);

            ferGUGAdd(cs, el);
        }
    }

    free(cells);

    //DBG("cells: %d", (int)cs->cells_len);
}

static void cellsAlloc(fer_gug_t *c, size_t num_cells)
{
    size_t i;
    fer_real_t *fdim, volume;

    // Compute dimensions of covered space and size of cell's edge
    fdim   = FER_ALLOC_ARR(fer_real_t, c->d);

    // dimensions and volume of space
    volume = 1.;
    for (i = 0; i < c->d; i++){
        fdim[i] = FER_FABS(c->aabb[2 * i + 1] - c->aabb[2 * i]);
        volume *= fdim[i];
    }

    // estimate volume of one cell
    volume *= ferRecp(num_cells);

    // set edge size as d'th root of volume
    c->edge = FER_POW(volume, ferRecp(c->d));
    c->edge_recp = ferRecp(c->edge);

    // and finally compute number of cells along each axis
    // there is addition of 1 to be sure that whole space will be mapped to
    // cubes
    for (i = 0; i < c->d; i++){
        c->dim[i] = (size_t)(fdim[i] * c->edge_recp) + (size_t)1;
    }
    free(fdim);


    // compute number of cells
    c->cells_len = c->dim[0];
    for (i = 1; i < c->d; i++){
        c->cells_len *= c->dim[i];
    }

    // allocate array of cells
    c->cells = FER_ALLOC_ARR(fer_gug_cell_t, c->cells_len);

    for (i = 0; i < c->cells_len; i++){
        cellInit(c, &c->cells[i], i);
    }

    if (c->max_dens == 0){
        c->next_expand = (size_t)-1;
    }else{
        c->next_expand = c->cells_len * c->max_dens;
    }
}


static void cellInit(fer_gug_t *cs, fer_gug_cell_t *c, size_t id)
{
    ferListInit(&c->list);
}

 

static void cacheInit(fer_gug_cache_t *cache,
                      fer_gug_el_t **els,
                      size_t max_len,
                      const fer_vec_t *p)
{
    // avoid allocation on heap if possible
    if (max_len < 4){
        cache->dist = cache->__dist;
    }else{
        cache->dist = FER_ALLOC_ARR(fer_real_t, max_len);
    }

    cache->els     = els;
    cache->len     = 0;
    cache->max_len = max_len;
    cache->p       = p;
}

static void cacheDestroy(fer_gug_cache_t *cache)
{
    if (cache->dist != cache->__dist)
        free(cache->dist);
}



static void __nearestInRadius(const fer_gug_t *cs, fer_gug_cache_t *cache,
                              int radius,
                              const size_t *center,
                              size_t *pos, int d, int fix)
{
    int i, from, to;
    size_t id;

    if (d == fix){
        if (d == cs->d - 1){
            id = __ferGUGPosToID(cs, pos);
            nearestInCell(cs, cache, &cs->cells[id]);
        }else{
            __nearestInRadius(cs, cache, radius, center, pos, d + 1, fix);
        }
    }else{
        if (d < fix){
            from = center[d] - radius + 1;
            from = FER_MAX(from, 0);
            to   = center[d] + radius - 1;
            to   = FER_MIN(to, cs->dim[d] - 1);
        }else{
            from = center[d] - radius;
            from = FER_MAX(from, 0);
            to   = center[d] + radius;
            to   = FER_MIN(to, cs->dim[d] - 1);
        }

        for (i = from; i <= to; i++){
            pos[d] = i;

            if (d == cs->d - 1){
                id = __ferGUGPosToID(cs, pos);
                nearestInCell(cs, cache, &cs->cells[id]);
            }else{
                __nearestInRadius(cs, cache, radius, center, pos, d + 1, fix);
            }
        }
    }
}

static int nearestInRadius(const fer_gug_t *cs, fer_gug_cache_t *cache,
                           int radius,
                           const size_t *center, size_t *pos)
{
    int d, rad, ret;

    ret = -1;

    for (d = 0; d < cs->d; d++){
        rad = (int)center[d] - radius;
        if (rad >= 0){
            pos[d] = rad;
            __nearestInRadius(cs, cache, radius, center, pos, 0, d);
            ret = 0;
        }

        rad = (int)center[d] + radius;
        if (rad < cs->dim[d]){
            pos[d] = rad;
            __nearestInRadius(cs, cache, radius, center, pos, 0, d);
            ret = 0;
        }
    }

    return ret;
}

static int nearestInRadius2(const fer_gug_t *cs, fer_gug_cache_t *cache,
                            int radius,
                            const size_t *center, size_t *pos)
{
    int d, from, to, id, ret;

    ret = -1;

    from = (int)center[1] - radius;
    from = FER_MAX(from, 0);
    to   = center[1] + radius;
    to   = FER_MIN(to, cs->dim[1] - 1);

    if (center[0] >= (size_t)radius){
        pos[0] = center[0] - radius;
        for (d = from; d <= to; d++){
            pos[1] = d;
            id = __ferGUGPosToID2(cs, pos);
            nearestInCell(cs, cache, &cs->cells[id]);
            ret = 0;
        }
    }

    if (center[0] + radius < cs->dim[0]){
        pos[0] = center[0] + radius;
        for (d = from; d <= to; d++){
            pos[1] = d;
            id = __ferGUGPosToID2(cs, pos);
            nearestInCell(cs, cache, &cs->cells[id]);
            ret = 0;
        }
    }

    from = (int)center[0] - radius + 1;
    from = FER_MAX(from, 0);
    to   = center[0] + radius - 1;
    to   = FER_MIN(to, cs->dim[0] - 1);

    if (center[1] >= (size_t)radius){
        pos[1] = center[1] - radius;
        for (d = from; d <= to; d++){
            pos[0] = d;
            id = __ferGUGPosToID2(cs, pos);
            nearestInCell(cs, cache, &cs->cells[id]);
            ret = 0;
        }
    }

    if (center[1] + radius < cs->dim[1]){
        pos[1] = center[1] + radius;
        for (d = from; d <= to; d++){
            pos[0] = d;
            id = __ferGUGPosToID2(cs, pos);
            nearestInCell(cs, cache, &cs->cells[id]);
            ret = 0;
        }
    }

    return ret;
}

static void nearestInCell(const fer_gug_t *cs, fer_gug_cache_t *cache,
                          fer_gug_cell_t *c)
{
    fer_list_t *list, *item;
    fer_gug_el_t *el;

    list = &c->list;
    FER_LIST_FOR_EACH(list, item){
        el = FER_LIST_ENTRY(item, fer_gug_el_t, list);
        nearestCheck(cs, cache, el);
    }
}

static void nearestCheck(const fer_gug_t *cs, fer_gug_cache_t *c,
                         fer_gug_el_t *el)
{
    fer_real_t dist;

    if (cs->d == 2){
        dist = ferVec2Dist2((fer_vec2_t *)c->p, (fer_vec2_t *)el->p);
    }else if (cs->d == 3){
        dist = ferVec3Dist2((fer_vec3_t *)c->p, (fer_vec3_t *)el->p);
    }else{
        dist = ferVecDist2(cs->d, c->p, el->p);
    }
    if (c->len < c->max_len){
        c->els[c->len]  = el;
        c->dist[c->len] = dist;
        c->len++;

        nearestBubbleUp(c);
    }else if (dist < c->dist[c->len - 1]){
        c->els[c->len - 1]  = el;
        c->dist[c->len - 1] = dist;

        nearestBubbleUp(c);
    }
}

static void nearestBubbleUp(fer_gug_cache_t *c)
{
    size_t i;
    fer_real_t tmpd;
    fer_gug_el_t *tmpn;

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


_fer_inline size_t __ferGUGPosToID(const fer_gug_t *cs, const size_t *pos)
{
    size_t id, mul, i;

    id  = pos[0];
    mul = cs->dim[0];
    for (i = 1; i < cs->d; i++){
        id  += pos[i] * mul;
        mul *= cs->dim[i];
    }

    return id;
}

_fer_inline size_t __ferGUGPosToID2(const fer_gug_t *cs,
                                        const size_t *pos)
{
    return pos[0] + pos[1] * cs->dim[0];
}

_fer_inline void __ferGUGIDToPos(const fer_gug_t *cs, size_t id,
                                     size_t *pos)
{
    size_t i;

    for (i = 0; i < cs->d; i++){
        pos[i] = id % cs->dim[i];
        id     = id / cs->dim[i];
    }
}

_fer_inline fer_real_t initBorder(const fer_gug_t *cs, const fer_vec_t *p)
{
    size_t i;
    fer_real_t local, min, max;
    fer_real_t f, b, border;

    border = FER_REAL_MAX;

    for (i = 0; i < cs->d; i++){
        local = ferVecGet(p, i) + cs->shift[i];

        f = local * cs->edge_recp;
        min = f * cs->edge;
        max = min + cs->edge;

        b = local - min;
        if (b < border)
            border = b;

        b = max - local;
        if (b < border)
            border = b;
    }

    if (border < FER_ZERO)
        border = FER_ZERO;
    return border;
}
