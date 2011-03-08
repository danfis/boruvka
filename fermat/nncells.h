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

#ifndef __FER_NNCELLS_H__
#define __FER_NNCELLS_H__

#include <fermat/list.h>
#include <fermat/vec.h>

struct fer_nncells_cache_t;

/** Internal structure */
struct _fer_nncells_cell_t {
    fer_list_t list; /*!< List of all elements inside cell */
};
typedef struct _fer_nncells_cell_t fer_nncells_cell_t;


/**
 * NNCells - Nearest Neighbor Cells
 * =================================
 *
 * TODO: example
 */

/**
 * Parameters
 * -----------
 */
struct _fer_nncells_params_t {
    size_t d;         /*!< Dimension of space. Default: 2 */
    size_t num_cells; /*!< Number of cells that should be created.
                           Note that this is only initial guess - finite
                           number of cells can be little bit more.
                           If set to 0 this parameter is ignored and
                           parameters .max_dens and .expand parameters are
                           taken. Default: 10000 */
    size_t max_dens;  /*!< Maximal densinty (i.e., #elements / #cells).
                           If density exceeds this treshold number of cells
                           is increased by .expand parameter. Default: 1 */
    size_t expand;    /*!< How many cells are added if density exceeds
                           .max_dens parameter. Default: 1000 */

    fer_real_t *aabb; /*!< Axis aligned bounding box of space that should
                           be covered by cells. A format is [xmin, xmax,
                           ymin, ymax, ...], length of array must be
                           2 * dimension. Default: NULL, i.e. must be set! */
};
typedef struct _fer_nncells_params_t fer_nncells_params_t;

/**
 * Initializes params struct.
 */
void ferNNCellsParamsInit(fer_nncells_params_t *p);


/**
 * NNCells
 * --------
 */
struct _fer_nncells_t {
    size_t d;          /*!< Dimension of covered space */
    size_t max_dens;   /*!< Maximal density - see params.max_dens */
    size_t expand;     /*!< See params.expand */
    fer_real_t *shift; /*!< Shifting of points stored in cells.
                            For easiest navigation in cells, cells structure
                            is built from origin towards x, y, z axis.
                            This vector is always added to point coordinates
                            that are searched to move it into space covered by
                            cubes. */
    fer_real_t *aabb;  /*!< Axis aligned bounding box of covered space */

    size_t num_els; /*!< Number of elements in cells */

    size_t *dim;               /*!< How many cells are along x, y, ... axis */
    fer_real_t edge;           /*!< Size of edge of one cell */
    fer_nncells_cell_t *cells; /*!< Array of all cells */
    size_t cells_len;          /*!< Length of .cells array */

    struct fer_nncells_cache_t *cache;
};
typedef struct _fer_nncells_t fer_nncells_t;


/**
 * User structure
 * ---------------
 *
 * TODO: Example
 */
struct _fer_nncells_el_t {
    fer_list_t list; /*!< List struct which is used for connection to cells */
    size_t cell_id;  /*!< Id of cell where is element currently registered,
                          i.e. .list is connected into fer_nncells_t's
                          .cells[.cell_id] cell. */

    const fer_vec_t *coords; /*!< Pointer to user's position vector */
};
typedef struct _fer_nncells_el_t fer_nncells_el_t;

/**
 * Initialize element struct.
 * You must call this before adding to nncells.
 */
_fer_inline void ferNNCellsElInit(fer_nncells_el_t *el, const fer_vec_t *coords);


/**
 * Functions
 * ----------
 */

/**
 * Creates and initialize new nncells structure.
 */
fer_nncells_t *ferNNCellsNew(const fer_nncells_params_t *params);

/**
 * Deletes nncells struct.
 */
void ferNNCellsDel(fer_nncells_t *c);

/**
 * Returns number of elements stored in cells.
 */
_fer_inline size_t ferNNCellsSize(const fer_nncells_t *c);

/**
 * Returns dimension of covered space.
 */
_fer_inline size_t ferNNCellsD(const fer_nncells_t *c);

/**
 * Returns number of cubes alogn x, y, z (, ...) axis.
 */
_fer_inline const size_t *ferNNCellsDim(const fer_nncells_t *c);

/**
 * Returns number cells.
 */
_fer_inline size_t ferNNCellsCellsLen(const fer_nncells_t *c);

/**
 * Returns size (length) of edge of one cell.
 */
_fer_inline fer_real_t ferNNCellsCellSize(const fer_nncells_t *c);


/**
 * Adds element to cells according to its coordinates in space.
 * This function should be called only once.
 *
 * First parameter is pointer to nncells structure.
 * Second parameter is nncells element structure - this structure is used
 * for connection into cubes structure (don't forget to call
 * ferNNCellsElInit()).
 */
_fer_inline void ferNNCellsAdd(fer_nncells_t *cs, fer_nncells_el_t *el);

/**
 * Removes element from cells.
 *
 * First parameter is pointer to nncells structure.
 * Second parameter is pointer to struct using which were element
 * connected in cubes.
 */
_fer_inline void ferNNCellsRemove(fer_nncells_t *cs, fer_nncells_el_t *el);

/**
 * Updates elements position in cells.
 * Call this function anytime element is moved. If you don't do that
 * position of element in cells could be incorrect.
 *
 * Parameters are same as in ferNNCellsAdd().
 */
_fer_inline void ferNNCellsUpdate(fer_nncells_t *cs, fer_nncells_el_t *el);

/**
 * This function si similar to ferNNCellsUpdate() but given element is
 * always removed from cubes and then added again. No checking if
 * re-registering is needed is performed.
 */
_fer_inline void ferNNCellsUpdateForce(fer_nncells_t *cs, fer_nncells_el_t *el);

/**
 * Finds {num} nearest elements to given point {p}.
 *
 * Array of pointers els must be allocated and must have at least {num}
 * elements. This array is filled with pointers to elements that are
 * nearest to point {p}. Number of found elements is returned.
 */
size_t ferNNCellsNearest(fer_nncells_t *cs, const fer_vec_t *p, size_t num,
                         fer_nncells_el_t **els);




/**
 * Returns cell id (position in .cells array) where belongs point with
 * given coordinates.
 * This function _always_ returns correct ID.
 */
_fer_inline size_t __ferNNCellsCoordsToID(const fer_nncells_t *cs,
                                          const fer_vec_t *coords);


/**** INLINES ****/
_fer_inline void ferNNCellsElInit(fer_nncells_el_t *el, const fer_vec_t *coords)
{
    el->coords = coords;
}


_fer_inline size_t ferNNCellsSize(const fer_nncells_t *c)
{
    return c->num_els;
}

_fer_inline size_t ferNNCellsD(const fer_nncells_t *c)
{
    return c->d;
}

_fer_inline const size_t *ferNNCellsDim(const fer_nncells_t *c)
{
    return c->dim;
}

_fer_inline size_t ferNNCellsCellsLen(const fer_nncells_t *c)
{
    return c->cells_len;
}

_fer_inline fer_real_t ferNNCellsCellSize(const fer_nncells_t *c)
{
    return c->edge;
}

_fer_inline void ferNNCellsAdd(fer_nncells_t *cs, fer_nncells_el_t *el)
{
    size_t id;

    id = __ferNNCellsCoordsToID(cs, el->coords);

    ferListAppend(&cs->cells[id].list, &el->list);

    el->cell_id = id;
    cs->num_els++;
}

_fer_inline void ferNNCellsRemove(fer_nncells_t *cs, fer_nncells_el_t *el)
{
    ferListDel(&el->list);

    el->cell_id = (size_t)-1;
    cs->num_els--;
}

_fer_inline void ferNNCellsUpdate(fer_nncells_t *cs, fer_nncells_el_t *el)
{
    size_t id;

    id = __ferNNCellsCoordsToID(cs, el->coords);
    if (id != el->cell_id){
        ferNNCellsUpdateForce(cs, el);
    }
}

_fer_inline void ferNNCellsUpdateForce(fer_nncells_t *cs, fer_nncells_el_t *el)
{
    ferNNCellsRemove(cs, el);
    ferNNCellsAdd(cs, el);
}




_fer_inline size_t __ferNNCellsCoordsToID(const fer_nncells_t *cs,
                                          const fer_vec_t *coords)
{
    size_t i, tmp, id, mul;
    fer_real_t f;

    id  = 0;
    mul = 1;
    for (i = 0; i < cs->d; i++){
        f  = ferVecGet(coords, i) + cs->shift[i];
        f *= ferRecp((fer_real_t)cs->edge);

        tmp = FER_MAX((int)f, 0);
        tmp = FER_MIN(tmp, cs->dim[i] - 1);

        id  += tmp * mul;
        mul *= cs->dim[i];
    }

    return id;
}
#endif /* __FER_NNCELLS_H__ */
