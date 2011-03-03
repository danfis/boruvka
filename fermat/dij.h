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

#ifndef __FER_DIJ_H__
#define __FER_DIJ_H__

#include <fermat/core.h>
#include <fermat/list.h>
#include <fermat/pairheap.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define FER_DIJ_STATE_UNKNOWN 0
#define FER_DIJ_STATE_CLOSED  1
#define FER_DIJ_STATE_OPEN    2

/**
 * Dij - Dijkstra Algorithm
 * =========================
 *
 */

/**
 * Node in graph
 * --------------
 */
struct _fer_dij_node_t {
    int state;       /*!< State of node: CLOSED, OPEN, UNKNOWN */
    fer_real_t dist; /*!< Overall distance from start node.
                          In other words, length of tentative shortest path
                          from start node */
    struct _fer_dij_node_t *prev; /*!< Pointer to previous node in path */

    fer_list_t _list; /*!< Internal connection into list of nodes.
                           See function ferDijNodeAdd() and operation
                           expand() */
    fer_pairheap_node_t _heap; /*!< Internal connection into heap */
};
typedef struct _fer_dij_node_t fer_dij_node_t;

/**
 * Initializes node.
 * Don't forget to call this once you created new node.
 *
 * Note that no destry function is needed.
 */
_fer_inline void ferDijNodeInit(fer_dij_node_t *n);

/**
 * Returns true if node is closed.
 * If node is closed you don't have to expand into this node anymore...
 */
_fer_inline int ferDijNodeClosed(const fer_dij_node_t *n);

/**
 * Returns overall distance of node from start node.
 */
_fer_inline fer_real_t ferDijDist(const fer_dij_node_t *n);

/**
 * Returns previous node in path.
 */
_fer_inline fer_dij_node_t *ferDijNodePrev(const fer_dij_node_t *n);

/**
 * Adds node into given list.
 */
_fer_inline void ferDijNodeAdd(fer_dij_node_t *n, fer_list_t *list);

/**
 * Returns node stored in list.
 */
_fer_inline fer_dij_node_t *ferDijNodeFromList(fer_list_t *item);


/**
 * Operations
 * -----------
 *
 * See fer_dij_ops_t.
 */

/** vvvv */
/**
 * Return (relative) distance between two nodes.
 */
typedef fer_real_t (*fer_dij_dist)(const fer_dij_node_t *n1,
                                   const fer_dij_node_t *n2, void *);

/**
 * Fill given list by neighbor nodes of {n}.
 * Use ferDijNodeAdd() function to connect node into given list.
 */
typedef void (*fer_dij_expand)(fer_dij_node_t *n, fer_list_t *list, void *);
/** ^^^^ */

struct _fer_dij_ops_t {
    fer_dij_dist dist;     /*!< Distance between two nodes */
    fer_dij_expand expand; /*!< Expands nodes */
    void *data;
};
typedef struct _fer_dij_ops_t fer_dij_ops_t;

/**
 * Initialize operations struct.
 */
_fer_inline void ferDijOpsInit(fer_dij_ops_t *ops);



/**
 * Dijkstra algorithm
 * -------------------
 */
struct _fer_dij_t {
    fer_dij_ops_t ops;    /*!< Operations */
    fer_pairheap_t *heap; /*!< Priority heap */
};
typedef struct _fer_dij_t fer_dij_t;

/**
 * Creates new dij struct.
 */
fer_dij_t *ferDijNew(const fer_dij_ops_t *ops);

/**
 * Deletes dij struct.
 * Note that no nodes are touched.
 */
void ferDijDel(fer_dij_t *dij);

/**
 * Runs dikstra algorithm.
 * Returns 0 if path was found.
 */
int ferDijRun(fer_dij_t *dij, fer_dij_node_t *start,
                              fer_dij_node_t *end);

/**
 * Fills given list by path which ends on endnode.
 */
void ferDijPath(fer_dij_node_t *endnode, fer_list_t *list);


/**** INLINES ****/
_fer_inline void ferDijNodeInit(fer_dij_node_t *n)
{
    n->state = FER_DIJ_STATE_UNKNOWN;
    n->dist  = FER_REAL_MAX;
}

_fer_inline int ferDijNodeClosed(const fer_dij_node_t *n)
{
    return n->state == FER_DIJ_STATE_CLOSED;
}

_fer_inline fer_real_t ferDijDist(const fer_dij_node_t *n)
{
    return n->dist;
}

_fer_inline fer_dij_node_t *ferDijNodePrev(const fer_dij_node_t *n)
{
    return n->prev;
}

_fer_inline void ferDijNodeAdd(fer_dij_node_t *n, fer_list_t *list)
{
    ferListAppend(list, &n->_list);
}

_fer_inline fer_dij_node_t *ferDijNodeFromList(fer_list_t *item)
{
    return ferListEntry(item, fer_dij_node_t, _list);
}



_fer_inline void ferDijOpsInit(fer_dij_ops_t *ops)
{
    bzero(ops, sizeof(fer_dij_ops_t));
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_DIJ_H__ */


