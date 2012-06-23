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

#ifndef __BOR_DIJ_H__
#define __BOR_DIJ_H__

#include <boruvka/core.h>
#include <boruvka/list.h>
#include <boruvka/pairheap.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define BOR_DIJ_STATE_UNKNOWN 0
#define BOR_DIJ_STATE_CLOSED  1
#define BOR_DIJ_STATE_OPEN    2

/**
 * Dij - Dijkstra Algorithm
 * =========================
 *
 */

/**
 * Node in graph
 * --------------
 */
struct _bor_dij_node_t {
    int state;       /*!< State of node: CLOSED, OPEN, UNKNOWN */
    bor_real_t dist; /*!< Overall distance from start node.
                          In other words, length of tentative shortest path
                          from start node */
    struct _bor_dij_node_t *prev; /*!< Pointer to previous node in path */

    bor_list_t _list; /*!< Internal connection into list of nodes.
                           See function borDijNodeAdd() and operation
                           expand() */
    bor_real_t _loc_dist; /*!< Local distance computed in last expand().
                               See function borDijNodeAdd() and operation
                               expand() */

    bor_pairheap_node_t _heap; /*!< Internal connection into heap */
};
typedef struct _bor_dij_node_t bor_dij_node_t;

/**
 * Initializes node.
 * Don't forget to call this once you created new node.
 *
 * Note that no destry function is needed.
 */
_bor_inline void borDijNodeInit(bor_dij_node_t *n);

/**
 * Returns true if node is closed.
 * If node is closed you don't have to expand into this node anymore...
 */
_bor_inline int borDijNodeClosed(const bor_dij_node_t *n);

/**
 * Returns overall distance of node from start node.
 */
_bor_inline bor_real_t borDijDist(const bor_dij_node_t *n);

/**
 * Returns previous node in path.
 */
_bor_inline bor_dij_node_t *borDijNodePrev(const bor_dij_node_t *n);

/**
 * Adds node into given list and sets local distance from previous node.
 *
 * Use this function in expand() operation.
 */
_bor_inline void borDijNodeAdd(bor_dij_node_t *n,
                               bor_list_t *list, bor_real_t dist);

/**
 * Returns node stored in list.
 */
_bor_inline bor_dij_node_t *borDijNodeFromList(bor_list_t *item);


/**
 * Operations
 * -----------
 *
 * See bor_dij_ops_t.
 */

/** vvvv */

/**
 * Fill given list by neighbor nodes of {n}.
 * Use borDijNodeAdd() function to connect node into given list.
 */
typedef void (*bor_dij_expand)(bor_dij_node_t *n, bor_list_t *list, void *);

/** ^^^^ */

struct _bor_dij_ops_t {
    bor_dij_expand expand; /*!< Expands nodes */
    void *data;
};
typedef struct _bor_dij_ops_t bor_dij_ops_t;

/**
 * Initialize operations struct.
 */
_bor_inline void borDijOpsInit(bor_dij_ops_t *ops);



/**
 * Dijkstra algorithm
 * -------------------
 */
struct _bor_dij_t {
    bor_dij_ops_t ops;    /*!< Operations */
    bor_pairheap_t *heap; /*!< Priority heap */
};
typedef struct _bor_dij_t bor_dij_t;

/**
 * Creates new dij struct.
 */
bor_dij_t *borDijNew(const bor_dij_ops_t *ops);

/**
 * Deletes dij struct.
 * Note that no nodes are touched.
 */
void borDijDel(bor_dij_t *dij);

/**
 * Runs dikstra algorithm.
 * Returns 0 if path was found.
 */
int borDijRun(bor_dij_t *dij, bor_dij_node_t *start,
                              bor_dij_node_t *end);

/**
 * Fills given list by path which ends on endnode.
 */
void borDijPath(bor_dij_node_t *endnode, bor_list_t *list);


/**** INLINES ****/
_bor_inline void borDijNodeInit(bor_dij_node_t *n)
{
    n->state = BOR_DIJ_STATE_UNKNOWN;
    n->dist  = BOR_REAL_MAX;
}

_bor_inline int borDijNodeClosed(const bor_dij_node_t *n)
{
    return n->state == BOR_DIJ_STATE_CLOSED;
}

_bor_inline bor_real_t borDijDist(const bor_dij_node_t *n)
{
    return n->dist;
}

_bor_inline bor_dij_node_t *borDijNodePrev(const bor_dij_node_t *n)
{
    return n->prev;
}

_bor_inline void borDijNodeAdd(bor_dij_node_t *n,
                               bor_list_t *list, bor_real_t dist)
{
    borListAppend(list, &n->_list);
    n->_loc_dist = dist;
}

_bor_inline bor_dij_node_t *borDijNodeFromList(bor_list_t *item)
{
    return BOR_LIST_ENTRY(item, bor_dij_node_t, _list);
}



_bor_inline void borDijOpsInit(bor_dij_ops_t *ops)
{
    bzero(ops, sizeof(bor_dij_ops_t));
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_DIJ_H__ */


