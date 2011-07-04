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

#include <stdio.h>
#include <fermat/gng-eu.h>
#include <fermat/nearest-linear.h>
#include <fermat/vec3.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Operations for fer_gng_ops_t struct */
static fer_gng_node_t *ferGNGEuNewNode(const void *input_signal, void *);
static fer_gng_node_t *ferGNGEuNewNodeBetween(const fer_gng_node_t *n1,
                                              const fer_gng_node_t *n2,
                                              void *);
static void ferGNGEuDelNode(fer_gng_node_t *n, void *);
static const void *ferGNGEuInputSignal(void *);
static void ferGNGEuNearest(const void *input_signal,
                            fer_gng_node_t **n1,
                            fer_gng_node_t **n2,
                            void *);
_fer_inline void ferGNGEuNearestLinear(const void *input_signal,
                                       fer_gng_node_t **n1,
                                       fer_gng_node_t **n2,
                                       void *);
_fer_inline void ferGNGEuNearestCells(const void *input_signal,
                                      fer_gng_node_t **n1,
                                      fer_gng_node_t **n2,
                                      void *);

static fer_real_t ferGNGEuDist2(const void *input_signal,
                                const fer_gng_node_t *node,
                                void *);

static void ferGNGEuMoveTowards(fer_gng_node_t *node,
                                const void *input_signal,
                                fer_real_t fraction, void *);


void ferGNGEuOpsInit(fer_gng_ops_t *ops)
{
    ferGNGOpsInit(ops);

    ops->init             = NULL;
    ops->new_node         = ferGNGEuNewNode;
    ops->new_node_between = ferGNGEuNewNodeBetween;
    ops->del_node         = ferGNGEuDelNode;
    ops->input_signal     = ferGNGEuInputSignal;
    ops->nearest          = ferGNGEuNearest;
    ops->dist2            = ferGNGEuDist2;
    ops->move_towards     = ferGNGEuMoveTowards;
}

void ferGNGEuParamsInit(fer_gng_eu_params_t *p)
{
    p->dim = 2;

    ferGNGParamsInit(&p->gng);

    p->use_cells = 1;
    ferNNCellsParamsInit(&p->cells);
}

fer_gng_eu_t *ferGNGEuNew(const fer_gng_ops_t *_ops,
                          const fer_gng_eu_params_t *params)
{
    fer_gng_eu_t *gng;
    fer_gng_ops_t ops;

    gng = FER_ALLOC(fer_gng_eu_t);

    gng->dim = params->dim;

    ops = *_ops;
    ops.init_data             = (void *)gng;
    ops.new_node_data         = (void *)gng;
    ops.new_node_between_data = (void *)gng;
    ops.del_node_data         = (void *)gng;
    ops.input_signal_data     = (void *)gng;
    ops.nearest_data          = (void *)gng;
    ops.dist2_data            = (void *)gng;
    ops.move_towards_data     = (void *)gng;
    gng->gng = ferGNGNew(&ops, &params->gng);

    gng->pc = ferPCNew(gng->dim);

    gng->cells = NULL;
    gng->use_cells = params->use_cells;
    gng->cells_params = params->cells;
    gng->cells_params.d = gng->dim;

    return gng;
}


void ferGNGEuDel(fer_gng_eu_t *gng)
{
    ferPCDel(gng->pc);
    ferGNGDel(gng->gng);

    if (gng->cells)
        ferNNCellsDel(gng->cells);

    free(gng);
}

void ferGNGEuRun(fer_gng_eu_t *gng)
{
    fer_real_t *aabb;

    ferPCPermutate(gng->pc);
    ferPCItInit(&gng->pcit, gng->pc);

    if (gng->use_cells){
        if (!gng->cells_params.aabb){
            aabb = FER_ALLOC_ARR(fer_real_t, 2 * gng->dim);
            ferPCAABB(gng->pc, aabb);
            gng->cells_params.aabb = aabb;

            gng->cells = ferNNCellsNew(&gng->cells_params);

            gng->cells_params.aabb = NULL;
            free(aabb);
        }else{
            gng->cells = ferNNCellsNew(&gng->cells_params);
        }
    }

    ferGNGRun(gng->gng);
}


static fer_gng_node_t *ferGNGEuNewNode(const void *input_signal, void *data)
{
    fer_gng_eu_t *gng = (fer_gng_eu_t *)data;
    fer_gng_eu_node_t *n;

    n = FER_ALLOC(fer_gng_eu_node_t);
    n->w = ferVecClone(gng->dim, (const fer_vec_t *)input_signal);

    if (gng->cells){
        ferNNCellsElInit(&n->cells, n->w);
        ferNNCellsAdd(gng->cells, &n->cells);
    }

    return &n->node;
}

static fer_gng_node_t *ferGNGEuNewNodeBetween(const fer_gng_node_t *_n1,
                                              const fer_gng_node_t *_n2,
                                              void *data)
{
    fer_gng_eu_t *gng = (fer_gng_eu_t *)data;
    fer_vec_t w;
    fer_gng_eu_node_t *n1, *n2;

    n1 = fer_container_of(_n1, fer_gng_eu_node_t, node);
    n2 = fer_container_of(_n2, fer_gng_eu_node_t, node);

    ferVecAdd2(gng->dim, &w, n1->w, n2->w);
    ferVecScale(gng->dim, &w, FER_REAL(0.5));

    return ferGNGEuNewNode(&w, data);
}

static void ferGNGEuDelNode(fer_gng_node_t *_n, void *data)
{
    fer_gng_eu_t *gng = (fer_gng_eu_t *)data;
    fer_gng_eu_node_t *n;

    n = fer_container_of(_n, fer_gng_eu_node_t, node);

    if (gng->cells){
        ferNNCellsRemove(gng->cells, &n->cells);
    }

    ferVecDel(n->w);

    free(n);
}

static const void *ferGNGEuInputSignal(void *data)
{
    fer_gng_eu_t *gng = (fer_gng_eu_t *)data;
    const fer_vec_t *v;

    if (ferPCItEnd(&gng->pcit)){
        ferPCPermutate(gng->pc);
        ferPCItInit(&gng->pcit, gng->pc);
    }

    v = ferPCItGet(&gng->pcit);
    ferPCItNext(&gng->pcit);

    return v;
}

static fer_real_t dist22(void *is, fer_list_t *nlist, void *data)
{
    fer_gng_eu_t *gng = (fer_gng_eu_t *)data;
    fer_gng_node_t *gn;
    fer_gng_eu_node_t *n;

    gn = ferGNGNodeFromList(nlist);
    n = fer_container_of(gn, fer_gng_eu_node_t, node);
    return ferVecDist2(gng->dim, (const fer_vec_t *)is, n->w);
}

static void ferGNGEuNearest(const void *input_signal,
                            fer_gng_node_t **n1,
                            fer_gng_node_t **n2,
                            void *data)
{
    fer_gng_eu_t *gng = (fer_gng_eu_t *)data;

    if (gng->cells){
        ferGNGEuNearestCells(input_signal, n1, n2, data);
    }else{
        ferGNGEuNearestLinear(input_signal, n1, n2, data);
    }
}

_fer_inline void ferGNGEuNearestLinear(const void *input_signal,
                                       fer_gng_node_t **n1,
                                       fer_gng_node_t **n2,
                                       void *data)
{
    fer_gng_eu_t *gng = (fer_gng_eu_t *)data;
    fer_list_t *ns[2];

    ns[0] = ns[1] = NULL;
    ferNearestLinear(ferGNGNodes(gng->gng), (void *)input_signal, dist22, ns, 2,
                     (void *)gng);

    *n1 = ferGNGNodeFromList(ns[0]);
    *n2 = ferGNGNodeFromList(ns[1]);
}

_fer_inline void ferGNGEuNearestCells(const void *input_signal,
                                      fer_gng_node_t **n1,
                                      fer_gng_node_t **n2,
                                      void *data)
{
    fer_gng_eu_t *gng = (fer_gng_eu_t *)data;
    fer_nncells_el_t *els[2];
    fer_gng_eu_node_t *n;

    *n1 = *n2 = NULL;

    ferNNCellsNearest(gng->cells, (const fer_vec_t *)input_signal, 2, els);

    n = fer_container_of(els[0], fer_gng_eu_node_t, cells);
    *n1 = &n->node;
    n = fer_container_of(els[1], fer_gng_eu_node_t, cells);
    *n2 = &n->node;
}

static fer_real_t ferGNGEuDist2(const void *input_signal,
                                const fer_gng_node_t *node,
                                void *data)
{
    fer_gng_eu_t *gng = (fer_gng_eu_t *)data;
    fer_gng_eu_node_t *n;
    n = fer_container_of(node, fer_gng_eu_node_t, node);
    return ferVecDist2(gng->dim, (const fer_vec_t *)input_signal, n->w);
}

static void ferGNGEuMoveTowards(fer_gng_node_t *node,
                                const void *input_signal,
                                fer_real_t fraction, void *data)
{
    fer_gng_eu_t *gng = (fer_gng_eu_t *)data;
    fer_gng_eu_node_t *n;
    fer_vec_t move;

    n = fer_container_of(node, fer_gng_eu_node_t, node);
    ferVecSub2(gng->dim, &move, (const fer_vec_t *)input_signal, n->w);
    ferVecScale(gng->dim, &move, fraction);
    ferVecAdd(gng->dim, n->w, &move);

    if (gng->cells){
        ferNNCellsUpdate(gng->cells, &n->cells);
    }
}

void ferGNGEuDumpSVT(fer_gng_eu_t *gng, FILE *out, const char *name)
{
    fer_list_t *list, *item;
    fer_net_node_t *nn;
    fer_gng_node_t *gn;
    fer_net_edge_t *e;
    fer_gng_eu_node_t *n;
    size_t i, id1, id2;

    if (gng->dim != 2 && gng->dim != 3)
        return;

    fprintf(out, "--------\n");

    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Points:\n");
    list = ferGNGNodes(gng->gng);
    i = 0;
    FER_LIST_FOR_EACH(list, item){
        gn = ferGNGNodeFromList(item);
        n  = ferGNGEuNodeFromGNG(gn);

        n->_id = i++;
        if (gng->dim == 2){
            ferVec2Print((const fer_vec2_t *)n->w, out);
        }else{
            ferVec3Print((const fer_vec3_t *)n->w, out);
        }
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = ferGNGEdges(gng->gng);
    FER_LIST_FOR_EACH(list, item){
        e = FER_LIST_ENTRY(item, fer_net_edge_t, list);

        nn = ferNetEdgeNode(e, 0);
        n  = ferGNGEuNodeFromNet(nn);
        id1 = n->_id;

        nn = ferNetEdgeNode(e, 1);
        n  = ferGNGEuNodeFromNet(nn);
        id2 = n->_id;
        fprintf(out, "%d %d\n", (int)id1, (int)id2);
    }

    fprintf(out, "--------\n");
}

