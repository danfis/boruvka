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
#include <fermat/dbg.h>


struct fer_nncells_cache_t {
    fer_nncells_el_t **els; /*!< Elements that we are searching for */
    fer_real_t *dist;       /*!< Distances of elements in .els */
    size_t dist_alloc;      /*!< Amount of allocated memory for .dist */
    size_t len;             /*!< Number of so far found elements */
    size_t max_len;         /*!< Maximal number of elements we want to find */

    const fer_vec2_t *p;
};

static void cellInit(fer_nncells_t *cs, fer_nncells_cell_t *c, size_t id);
static void cellDestroy(fer_nncells_cell_t *c);
static void cellExpandLayer(fer_list_t *layer, fer_list_t *clear);
/** Clears all cells in list */
static void cellClear(fer_list_t *list);

/** Initializes and destroys .cache member of fer_cubes`N`_t */
static void cacheInit(fer_nncells_t *cs, fer_nncells_el_t **els,
                      size_t max_len, const fer_vec2_t *p);
static void cacheDestroy(fer_nncells_t *cs);


/** Searches only specified cube */
static void nearestInCube(fer_nncells_t *cs, fer_nncells_cell_t *c);
/** Checks if given element isn't closer than the ones already stored in
 *  cache. */
static void nearestCheck(struct fer_nncells_cache_t *c, fer_nncells_el_t *el);
/** Bubble sort. Takes the last element in .els and bubble it towards
 *  smaller ones (according to .dist[] value). */
static void nearestBubbleUp(struct fer_nncells_cache_t *c);


/** Converts position in cells to cell's ID */
_fer_inline size_t __ferNNCellsPosToID(const fer_nncells_t *cs,
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
    size_t i;

    if (c->dim)
        free(c->dim);
    if (c->shift)
        free(c->shift);

    if (c->cells){
        for (i = 0; i < c->cells_len; i++){
            cellDestroy(&c->cells[i]);
        }
        free(c->cells);
    }

    if (c->cache)
        cacheDestroy(c);

    free(c);
}

size_t ferNNCellsNearest(fer_nncells_t *cs, const fer_vec2_t *p, size_t num,
                         fer_nncells_el_t **els)
{
    size_t center_id;
    fer_list_t layer, clear, *item;
    fer_nncells_cell_t *cell;
    int radius;
    fer_real_t border;

    cacheInit(cs, els, num, p);

    center_id = __ferNNCellsCoordsToID(cs, p);

    ferListInit(&layer);
    ferListInit(&clear);

    radius = 0;
    border = -1;

    // push center cell into lists
    cell = &cs->cells[center_id];
    cell->radius = radius;
    ferListAppend(&layer, &cell->layer_list);
    ferListAppend(&clear, &cell->clear_list);

    while (!ferListEmpty(&layer)){
        ferListForEach(&layer, item){
            cell = ferListEntry(item, fer_nncells_cell_t, layer_list);
            nearestInCube(cs, cell);
        }

        // End searching if we have all points we wanted and the furthest
        // one from them is before border, i.e. we are sure there is no
        // nearest point in other cells.
        if (cs->cache->len == cs->cache->max_len
                && cs->cache->dist[cs->cache->len - 1] < border){
            break;
        }

        radius++;
        cellExpandLayer(&layer, &clear);

        border = (fer_real_t)(radius) * cs->edge;
        // border must be squared because internaly are distances managed
        // as squared distances
        border *= border;
    }

    cellClear(&clear);

    return cs->cache->len;
}



static void __cellInit(fer_nncells_t *cs, fer_nncells_cell_t *c, size_t cid,
                        size_t *pos, size_t d, size_t *opos)
{
    size_t id;
    int i, min, max;

    min = FER_MAX((int)pos[d] - 1, 0);
    max = FER_MIN(pos[d] + 1, cs->dim[d] - 1);
    for (i = min; i <= max; i++){
        opos[d] = i;

        if (d == cs->d - 1){
            id = __ferNNCellsPosToID(cs, opos);
            if (id != cid){
                c->cells[c->cells_len] = &cs->cells[id];
                c->cells_len++;
            }
        }else{
            __cellInit(cs, c, cid, pos, d + 1, opos);
        }
    }
}

static void cellInit(fer_nncells_t *cs, fer_nncells_cell_t *c, size_t id)
{
    size_t *pos, *opos;
    size_t len;

    ferListInit(&c->list);
    c->radius = -1;

    len = FER_POWL(3, cs->d);
    c->cells = FER_ALLOC_ARR(fer_nncells_cell_t *, len);
    c->cells_len = 0;

    pos  = FER_ALLOC_ARR(size_t, cs->d);
    opos = FER_ALLOC_ARR(size_t, cs->d);

    __ferNNCellsIDToPos(cs, id, pos);

    __cellInit(cs, c, id, pos, 0, opos);

    free(pos);
    free(opos);
}

static void cellDestroy(fer_nncells_cell_t *c)
{
    free(c->cells);
}

static void cellExpandLayer(fer_list_t *layer, fer_list_t *clear)
{
    size_t i;
    int radius;
    fer_list_t *item, *item_tmp;
    fer_nncells_cell_t *c;

    item = ferListNext(layer);
    c    = ferListEntry(item, fer_nncells_cell_t, layer_list);
    radius = c->radius + 1;

    ferListForEachSafe(layer, item, item_tmp){
        c = ferListEntry(item, fer_nncells_cell_t, layer_list);
        if (c->radius == radius)
            break;

        ferListDel(item);

        for (i = 0; i < c->cells_len; i++){
            if (c->cells[i]->radius == -1){
                ferListAppend(layer, &c->cells[i]->layer_list);
                ferListAppend(clear, &c->cells[i]->clear_list);
                c->cells[i]->radius = radius;
            }
        }
    }
}

static void cellClear(fer_list_t *list)
{
    fer_list_t *item;
    fer_nncells_cell_t *c;

    ferListForEach(list, item){
        c = ferListEntry(item, fer_nncells_cell_t, clear_list);
        c->radius = -1;
    }
}


static void cacheInit(fer_nncells_t *cs, fer_nncells_el_t **els,
                      size_t max_len, const fer_vec2_t *p)
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


static void nearestInCube(fer_nncells_t *cs, fer_nncells_cell_t *c)
{
    fer_list_t *list, *item;
    fer_nncells_el_t *el;

    list = &c->list;
    ferListForEach(list, item){
        el = ferListEntry(item, fer_nncells_el_t, list);
        nearestCheck(cs->cache, el);
    }
}

static void nearestCheck(struct fer_nncells_cache_t *c, fer_nncells_el_t *el)
{
    fer_real_t dist;

    dist = ferVec2Dist2(c->p, el->coords);
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

_fer_inline void __ferNNCellsIDToPos(const fer_nncells_t *cs, size_t id,
                                     size_t *pos)
{
    size_t i;

    for (i = 0; i < cs->d; i++){
        pos[i] = id % cs->dim[i];
        id     = id / cs->dim[i];
    }
}
