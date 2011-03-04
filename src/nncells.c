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

#include <fermat/nncells.h>
#include <fermat/alloc.h>
#include <fermat/vec.h>
#include <fermat/dbg.h>


struct fer_nncells_cache_t {
    fer_nncells_el_t **els; /*!< Elements that we are searching for */
    fer_real_t *dist;       /*!< Distances of elements in .els */
    size_t dist_alloc;      /*!< Amount of allocated memory for .dist */
    size_t len;             /*!< Number of so far found elements */
    size_t max_len;         /*!< Maximal number of elements we want to find */

    const fer_vec_t *p;
};

static void cellInit(fer_nncells_t *cs, fer_nncells_cell_t *c, size_t id);

/** Initializes and destroys .cache member of fer_cubes`N`_t */
static void cacheInit(fer_nncells_t *cs, fer_nncells_el_t **els,
                      size_t max_len, const fer_vec_t *p);
static void cacheDestroy(fer_nncells_t *cs);


/** Nearest nodes in given radius around center */
static int nearestInRadius(fer_nncells_t *cs, int radius,
                           const size_t *center, size_t *pos);
/** For 2D */
static int nearestInRadius2(fer_nncells_t *cs, int radius,
                            const size_t *center, size_t *pos);
/** Searches only specified cube */
static void nearestInCell(fer_nncells_t *cs, fer_nncells_cell_t *c);
/** Checks if given element isn't closer than the ones already stored in
 *  cache. */
static void nearestCheck(fer_nncells_t *cs, fer_nncells_el_t *el);
/** Bubble sort. Takes the last element in .els and bubble it towards
 *  smaller ones (according to .dist[] value). */
static void nearestBubbleUp(struct fer_nncells_cache_t *c);
/** Returns distance of initial border. */
_fer_inline fer_real_t initBorder(fer_nncells_t *cs, const fer_vec_t *p);


/** Converts position in cells to cell's ID */
_fer_inline size_t __ferNNCellsPosToID(const fer_nncells_t *cs,
                                       const size_t *pos);
_fer_inline size_t __ferNNCellsPosToID2(const fer_nncells_t *cs,
                                        const size_t *pos);
/** Invert function to __ferNNCellsPosToID() */
_fer_inline void __ferNNCellsIDToPos(const fer_nncells_t *cs, size_t id,
                                     size_t *pos);

fer_nncells_t *ferNNCellsNew(size_t d, const fer_real_t *bound, size_t num_cells)
{
    size_t i;
    fer_nncells_t *c;
    fer_real_t *fdim, volume;

    c = FER_ALLOC(fer_nncells_t);
    c->num_els = 0;

    c->d = d;

    // Compute shifting
    c->shift = FER_ALLOC_ARR(fer_real_t, c->d);
    for (i = 0; i < c->d; i++){
        c->shift[i] = -FER_ONE * bound[2 * i];
    }

    // Compute dimensions of covered space and size of cell's edge
    c->dim = FER_ALLOC_ARR(size_t, c->d);
    fdim   = FER_ALLOC_ARR(fer_real_t, c->d);

    // dimensions and volume of space
    volume = 1.;
    for (i = 0; i < c->d; i++){
        fdim[i] = FER_FABS(bound[2 * i + 1] - bound[2 * i]);
        volume *= fdim[i];
    }

    // estimate volume of one cell
    volume *= ferRecp(num_cells);

    // set edge size as d'th root of volume
    c->edge = FER_POW(volume, ferRecp(c->d));

    // and finally compute number of cells along each axis
    // there is addition of 1 to be sure that whole space will be mapped to
    // cubes
    for (i = 0; i < c->d; i++){
        c->dim[i] = (size_t)(fdim[i] / c->edge) + (size_t)1;
    }
    free(fdim);


    // compute number of cells
    c->cells_len = c->dim[0];
    for (i = 1; i < c->d; i++){
        c->cells_len *= c->dim[i];
    }

    // allocate array of cells
    c->cells = FER_ALLOC_ARR(fer_nncells_cell_t, c->cells_len);

    for (i = 0; i < c->cells_len; i++){
        cellInit(c, &c->cells[i], i);
    }

    c->cache = NULL;

    return c;
}

void ferNNCellsDel(fer_nncells_t *c)
{
    if (c->dim)
        free(c->dim);
    if (c->shift)
        free(c->shift);

    if (c->cells){
        free(c->cells);
    }

    if (c->cache)
        cacheDestroy(c);

    free(c);
}

size_t ferNNCellsNearest(fer_nncells_t *cs, const fer_vec_t *p, size_t num,
                         fer_nncells_el_t **els)
{
    size_t center_id;
    size_t *center, *pos;
    fer_nncells_cell_t *cell;
    int radius;
    fer_real_t border, border2;

    if (ferNNCellsSize(cs) == 0)
        return 0;

    center = FER_ALLOC_ARR(size_t, cs->d);
    pos    = FER_ALLOC_ARR(size_t, cs->d);

    cacheInit(cs, els, num, p);

    center_id = __ferNNCellsCoordsToID(cs, p);
    __ferNNCellsIDToPos(cs, center_id, center);

    // search in center first
    cell = &cs->cells[center_id];
    nearestInCell(cs, cell);

    border  = initBorder(cs, p);
    border2 = FER_CUBE(border);

    radius = 1;
    while (1){
        // End searching if we have all points we wanted and the furthest
        // one from them is before border, i.e. we are sure there is no
        // nearest point in other cells.
        if (cs->cache->len == cs->cache->max_len
                && cs->cache->dist[cs->cache->len - 1] < border2){
            break;
        }

        if (cs->d == 2){
            if (nearestInRadius2(cs, radius, center, pos) != 0)
                break;
        }else{
            if (nearestInRadius(cs, radius, center, pos) != 0)
                break;
        }

        border += cs->edge;
        // border must be squared because internaly are distances managed
        // as squared distances
        border2 = FER_CUBE(border);

        radius++;
    }

    free(center);
    free(pos);

    return cs->cache->len;
}



static void cellInit(fer_nncells_t *cs, fer_nncells_cell_t *c, size_t id)
{
    ferListInit(&c->list);
}


static void cacheInit(fer_nncells_t *cs, fer_nncells_el_t **els,
                      size_t max_len, const fer_vec_t *p)
{
    if (cs->cache == NULL){
        cs->cache = FER_ALLOC(struct fer_nncells_cache_t);
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

static void cacheDestroy(fer_nncells_t *cs)
{
    if (cs->cache){
        if (cs->cache->dist)
            free(cs->cache->dist);
        free(cs->cache);
    }
    cs->cache = NULL;
}



static void __nearestInRadius(fer_nncells_t *cs, int radius,
                              const size_t *center,
                              size_t *pos, int d, int fix)
{
    int i, from, to;
    size_t id;

    if (d == fix){
        if (d == cs->d - 1){
            id = __ferNNCellsPosToID(cs, pos);
            nearestInCell(cs, &cs->cells[id]);
        }else{
            __nearestInRadius(cs, radius, center, pos, d + 1, fix);
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
                id = __ferNNCellsPosToID(cs, pos);
                nearestInCell(cs, &cs->cells[id]);
            }else{
                __nearestInRadius(cs, radius, center, pos, d + 1, fix);
            }
        }
    }
}

static int nearestInRadius(fer_nncells_t *cs, int radius,
                           const size_t *center, size_t *pos)
{
    int d, rad, ret;

    ret = -1;

    for (d = 0; d < cs->d; d++){
        rad = (int)center[d] - radius;
        if (rad >= 0){
            pos[d] = rad;
            __nearestInRadius(cs, radius, center, pos, 0, d);
            ret = 0;
        }

        rad = (int)center[d] + radius;
        if (rad < cs->dim[d]){
            pos[d] = rad;
            __nearestInRadius(cs, radius, center, pos, 0, d);
            ret = 0;
        }
    }

    return ret;
}

static int nearestInRadius2(fer_nncells_t *cs, int radius,
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
            id = __ferNNCellsPosToID2(cs, pos);
            nearestInCell(cs, &cs->cells[id]);
            ret = 0;
        }
    }

    if (center[0] + radius < cs->dim[0]){
        pos[0] = center[0] + radius;
        for (d = from; d <= to; d++){
            pos[1] = d;
            id = __ferNNCellsPosToID2(cs, pos);
            nearestInCell(cs, &cs->cells[id]);
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
            id = __ferNNCellsPosToID2(cs, pos);
            nearestInCell(cs, &cs->cells[id]);
            ret = 0;
        }
    }

    if (center[1] + radius < cs->dim[1]){
        pos[1] = center[1] + radius;
        for (d = from; d <= to; d++){
            pos[0] = d;
            id = __ferNNCellsPosToID2(cs, pos);
            nearestInCell(cs, &cs->cells[id]);
            ret = 0;
        }
    }

    return ret;
}

static void nearestInCell(fer_nncells_t *cs, fer_nncells_cell_t *c)
{
    fer_list_t *list, *item;
    fer_nncells_el_t *el;

    list = &c->list;
    ferListForEach(list, item){
        el = ferListEntry(item, fer_nncells_el_t, list);
        nearestCheck(cs, el);
    }
}

static void nearestCheck(fer_nncells_t *cs, fer_nncells_el_t *el)
{
    fer_real_t dist;
    struct fer_nncells_cache_t *c = cs->cache;

    dist = ferVecDist2(cs->d, c->p, el->coords);
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

static void nearestBubbleUp(struct fer_nncells_cache_t *c)
{
    size_t i;
    fer_real_t tmpd;
    fer_nncells_el_t *tmpn;

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


_fer_inline size_t __ferNNCellsPosToID(const fer_nncells_t *cs, const size_t *pos)
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

_fer_inline size_t __ferNNCellsPosToID2(const fer_nncells_t *cs,
                                        const size_t *pos)
{
    return pos[0] + pos[1] * cs->dim[0];
}

_fer_inline void __ferNNCellsIDToPos(const fer_nncells_t *cs, size_t id,
                                     size_t *pos)
{
    size_t i;

    for (i = 0; i < cs->d; i++){
        pos[i] = id % cs->dim[i];
        id     = id / cs->dim[i];
    }
}

_fer_inline fer_real_t initBorder(fer_nncells_t *cs, const fer_vec_t *p)
{
    size_t i;
    fer_real_t local, min, max;
    fer_real_t f, b, border;

    border = FER_REAL_MAX;

    for (i = 0; i < cs->d; i++){
        local = ferVecGet(p, i) + cs->shift[i];

        f = local * ferRecp((fer_real_t)cs->edge);
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
