/***
 * Boruvka
 * --------
 * Copyright (c)2015 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_SCC_H__
#define __BOR_SCC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Strongly Connected Component
 * =============================
 *
 * See bor_scc_t.
 */

/**
 * Returns iterator over neighbors of the specified node.
 */
typedef long (*bor_scc_neighbor_it_fn)(int node_id, void *userdata);

/**
 * Returns next neighbor node ID or -1 if there are no other nodes.
 */
typedef int (*bor_scc_neighbor_next_fn)(int node_id, long *it, void *userdata);

/**
 * One strongly connected component
 */
struct _bor_scc_comp_t {
    int *node;     /*!< A node ID */
    int node_size; /*!< Number of nodes in component */
};
typedef struct _bor_scc_comp_t bor_scc_comp_t;

/**
 * Strongly connected components
 */
struct _bor_scc_t {
    int node_size;                 /*!< Number of nodes in the graph */
    bor_scc_neighbor_it_fn it;     /*!< Callback for retrieving neighbor
                                        nodes via iterator. */
    bor_scc_neighbor_next_fn next; /*!< Next callback for iterator */
    void *userdata;                /*!< Userdata argument for callbacks */

    bor_scc_comp_t *comp; /*!< List of components */
    int comp_size;        /*!< Number of components */
};
typedef struct _bor_scc_t bor_scc_t;

/**
 * Initializes structure with size of the graph and callback returning
 * neighbor nodes.
 */
void borSCCInit(bor_scc_t *scc, int node_size,
                bor_scc_neighbor_it_fn it_fn,
                bor_scc_neighbor_next_fn next_fn,
                void *userdata);

/**
 * Frees allocated resources.
 */
void borSCCFree(bor_scc_t *scc);

/**
 * Finds strongly connected components -- it fills .comp* members.
 */
void borSCC(bor_scc_t *scc);

/**
 * Finds only one strongly connected component that contains the specified
 * node.
 */
void borSCC1(bor_scc_t *scc, int node);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_SCC_H__ */
