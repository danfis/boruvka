/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_GUG_H__
#define __BOR_GUG_H__

#include <boruvka/list.h>
#include <boruvka/vec.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Internal structure */
struct _bor_gug_cell_t {
    bor_list_t list; /*!< List of all elements inside cell */
};
typedef struct _bor_gug_cell_t bor_gug_cell_t;


/**
 * Growing Uniform Grid
 * =====================
 *
 * TODO: example
 */

/**
 * Parameters
 * -----------
 */
struct _bor_gug_params_t {
    size_t dim;             /*!< Dimension of space. Default: 2 */
    size_t num_cells;       /*!< Number of cells that should be created.
                                 Note that this is only initial guess -
                                 finite number of cells can be little bit
                                 more.
                                 If set to 0 this parameter is ignored and
                                 parameters .max_dens and .expand
                                 parameters are taken.
                                 Default: 10000 */
    bor_real_t max_dens;    /*!< Maximal densinty (i.e., #elements / #cells).
                                 If density exceeds this treshold number of
                                 cells is increased by .expand parameter.
                                 Default: 1 */
    bor_real_t expand_rate; /*!< How fast is growing number of cells.
                                 Everytime density of elements in cells
                                 exceeds .max_dens .expand_rate times more
                                 cells are allocated.
                                 Default: 2. */ 
    bor_real_t *aabb;       /*!< Axis aligned bounding box of space that
                                 should be covered by cells. A format is
                                 [xmin, xmax, ymin, ymax, ...], length of
                                 array must be 2 * dimension.
                                 Default: NULL, i.e. must be set! */
    int approx;             /*!< Set to true if approximate nearest
                                 neighbor search should be used.
                                 Defaule: False */
};
typedef struct _bor_gug_params_t bor_gug_params_t;

/**
 * Initializes params struct.
 */
void borGUGParamsInit(bor_gug_params_t *p);


/**
 * GUG
 * --------
 */
struct _bor_gug_t {
    uint8_t type;        /*!< Type of NN search algorithm. See boruvka/nn.h */

    size_t d;            /*!< Dimension of covered space */
    bor_real_t max_dens; /*!< Maximal density - see params.max_dens */
    bor_real_t expand;   /*!< See params.expand_rate */
    bor_real_t *shift;   /*!< Shifting of points stored in cells.
                              For easiest navigation in cells, cells structure
                              is built from origin towards x, y, z axis.
                              This vector is always added to point coordinates
                              that are searched to move it into space covered by
                              cubes. */
    bor_real_t *aabb;    /*!< Axis aligned bounding box of covered space */
    int approx;          /*!< True if approx. algorithm should be used */

    size_t num_els; /*!< Number of elements in cells */

    size_t *dim;               /*!< How many cells are along x, y, ... axis */
    bor_real_t edge;           /*!< Size of edge of one cell */
    bor_real_t edge_recp;      /*!< 1 / .edge */
    bor_gug_cell_t *cells; /*!< Array of all cells */
    size_t cells_len;          /*!< Length of .cells array */
    size_t next_expand;        /*!< Treshold when number of cells should be
                                    expanded */
};
typedef struct _bor_gug_t bor_gug_t;


/**
 * User structure
 * ---------------
 *
 * TODO: Example
 */
struct _bor_gug_el_t {
    const bor_vec_t *p; /*!< Pointer to user-defined point vector */
    bor_list_t list;    /*!< List struct which is used for connection to cells */
    size_t cell_id;  /*!< Id of cell where is element currently registered,
                          i.e. .list is connected into bor_gug_t's
                          .cells[.cell_id] cell. */
};
typedef struct _bor_gug_el_t bor_gug_el_t;

/**
 * Initialize element struct.
 * You must call this before adding to gug.
 */
_bor_inline void borGUGElInit(bor_gug_el_t *el, const bor_vec_t *p);


/**
 * Functions
 * ----------
 */

/**
 * Creates and initialize new gug structure.
 */
bor_gug_t *borGUGNew(const bor_gug_params_t *params);

/**
 * Deletes gug struct.
 */
void borGUGDel(bor_gug_t *c);

/**
 * Returns number of elements stored in cells.
 */
_bor_inline size_t borGUGSize(const bor_gug_t *c);

/**
 * Returns dimension of covered space.
 */
_bor_inline size_t borGUGD(const bor_gug_t *c);

/**
 * Returns number of cubes alogn x, y, z (, ...) axis.
 */
_bor_inline const size_t *borGUGDim(const bor_gug_t *c);

/**
 * Returns number cells.
 */
_bor_inline size_t borGUGCellsLen(const bor_gug_t *c);

/**
 * Returns size (length) of edge of one cell.
 */
_bor_inline bor_real_t borGUGCellSize(const bor_gug_t *c);


/**
 * Adds element to cells according to its coordinates in space.
 * This function should be called only once.
 *
 * First parameter is pointer to gug structure.
 * Second parameter is gug element structure - this structure is used
 * for connection into cubes structure (don't forget to call
 * borGUGElInit()).
 */
_bor_inline void borGUGAdd(bor_gug_t *cs, bor_gug_el_t *el);

/**
 * Removes element from cells.
 *
 * First parameter is pointer to gug structure.
 * Second parameter is pointer to struct using which were element
 * connected in cubes.
 */
_bor_inline void borGUGRemove(bor_gug_t *cs, bor_gug_el_t *el);

/**
 * Updates elements position in cells.
 * Call this function anytime element is moved. If you don't do that
 * position of element in cells could be incorrect.
 *
 * Parameters are same as in borGUGAdd().
 */
_bor_inline void borGUGUpdate(bor_gug_t *cs, bor_gug_el_t *el);

/**
 * This function si similar to borGUGUpdate() but given element is
 * always removed from cubes and then added again. No checking if
 * re-registering is needed is performed.
 */
_bor_inline void borGUGUpdateForce(bor_gug_t *cs, bor_gug_el_t *el);

/**
 * Finds {num} nearest elements to given point {p}.
 *
 * Array of pointers els must be allocated and must have at least {num}
 * elements. This array is filled with pointers to elements that are
 * nearest to point {p}. Number of found elements is returned.
 */
size_t borGUGNearest(const bor_gug_t *cs, const bor_vec_t *p, size_t num,
                         bor_gug_el_t **els);

/**
 * Same as {borGUGNearest} but approximate algorithm is used.
 */
size_t borGUGNearestApprox(const bor_gug_t *cs, const bor_vec_t *p,
                               size_t num, bor_gug_el_t **els);




/**
 * Returns cell id (position in .cells array) where belongs point with
 * given coordinates.
 * This function _always_ returns correct ID.
 */
_bor_inline size_t __borGUGCoordsToID(const bor_gug_t *cs,
                                          const bor_vec_t *p);


/** Expands number of cells. This is function for internal use. Don't use it! */
void __borGUGExpand(bor_gug_t *cs);

/**** INLINES ****/
_bor_inline void borGUGElInit(bor_gug_el_t *el, const bor_vec_t *p)
{
    el->p = p;
}


_bor_inline size_t borGUGSize(const bor_gug_t *c)
{
    return c->num_els;
}

_bor_inline size_t borGUGD(const bor_gug_t *c)
{
    return c->d;
}

_bor_inline const size_t *borGUGDim(const bor_gug_t *c)
{
    return c->dim;
}

_bor_inline size_t borGUGCellsLen(const bor_gug_t *c)
{
    return c->cells_len;
}

_bor_inline bor_real_t borGUGCellSize(const bor_gug_t *c)
{
    return c->edge;
}

_bor_inline void borGUGAdd(bor_gug_t *cs, bor_gug_el_t *el)
{
    size_t id;

    id = __borGUGCoordsToID(cs, el->p);

    borListAppend(&cs->cells[id].list, &el->list);

    el->cell_id = id;
    cs->num_els++;

    if (cs->num_els >= cs->next_expand)
        __borGUGExpand(cs);
}

_bor_inline void borGUGRemove(bor_gug_t *cs, bor_gug_el_t *el)
{
    borListDel(&el->list);

    el->cell_id = (size_t)-1;
    cs->num_els--;
}

_bor_inline void borGUGUpdate(bor_gug_t *cs, bor_gug_el_t *el)
{
    size_t id;

    id = __borGUGCoordsToID(cs, el->p);
    if (id != el->cell_id){
        borGUGUpdateForce(cs, el);
    }
}

_bor_inline void borGUGUpdateForce(bor_gug_t *cs, bor_gug_el_t *el)
{
    borGUGRemove(cs, el);
    borGUGAdd(cs, el);
}




_bor_inline size_t __borGUGCoordsToID(const bor_gug_t *cs,
                                          const bor_vec_t *p)
{
    size_t i, tmp, id, mul;
    bor_real_t f;

    id  = 0;
    mul = 1;
    for (i = 0; i < cs->d; i++){
        f  = borVecGet(p, i) + cs->shift[i];
        f *= cs->edge_recp;

        tmp = BOR_MAX((int)f, 0);
        tmp = BOR_MIN(tmp, cs->dim[i] - 1);

        id  += tmp * mul;
        mul *= cs->dim[i];
    }

    return id;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_GUG_H__ */
