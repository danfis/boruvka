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
#include <fermat/gng2.h>
#include <fermat/nearest-linear.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Operations for fer_gng_ops_t struct */
static fer_gng_node_t *ferGNG2NewNode(const void *input_signal, void *);
static fer_gng_node_t *ferGNG2NewNodeBetween(const fer_gng_node_t *n1,
                                             const fer_gng_node_t *n2,
                                             void *);
static void ferGNG2DelNode(fer_gng_node_t *n, void *);
static const void *ferGNG2InputSignal(void *);
static void ferGNG2Nearest(const void *input_signal,
                           fer_gng_node_t **n1,
                           fer_gng_node_t **n2,
                           void *);
_fer_inline void ferGNG2NearestLinear(const void *input_signal,
                                      fer_gng_node_t **n1,
                                      fer_gng_node_t **n2,
                                      void *);
_fer_inline void ferGNG2NearestCubes(const void *input_signal,
                                     fer_gng_node_t **n1,
                                     fer_gng_node_t **n2,
                                     void *);

static fer_real_t ferGNG2Dist2(const void *input_signal,
                               const fer_gng_node_t *node,
                               void *);

static void ferGNG2MoveTowards(fer_gng_node_t *node,
                               const void *input_signal,
                               fer_real_t fraction, void *);


void ferGNG2OpsInit(fer_gng_ops_t *ops)
{
    ferGNGOpsInit(ops);

    ops->init             = NULL;
    ops->new_node         = ferGNG2NewNode;
    ops->new_node_between = ferGNG2NewNodeBetween;
    ops->del_node         = ferGNG2DelNode;
    ops->input_signal     = ferGNG2InputSignal;
    ops->nearest          = ferGNG2Nearest;
    ops->dist2            = ferGNG2Dist2;
    ops->move_towards     = ferGNG2MoveTowards;
}

void ferGNG2ParamsInit(fer_gng2_params_t *p)
{
    ferGNGParamsInit(&p->gng);

    p->use_cubes = 1;
    p->num_cubes = 10000;
}

fer_gng2_t *ferGNG2New(const fer_gng_ops_t *_ops,
                       const fer_gng2_params_t *params)
{
    fer_gng2_t *gng;
    fer_gng_ops_t ops;

    gng = FER_ALLOC(fer_gng2_t);

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

    gng->pc = ferPC2New();

    // TODO
    gng->cubes = NULL;
    if (params->use_cubes){
        gng->num_cubes = params->num_cubes;
    }else{
        gng->num_cubes = 0;
    }

    return gng;
}


void ferGNG2Del(fer_gng2_t *gng)
{
    ferPC2Del(gng->pc);
    ferGNGDel(gng->gng);

    if (gng->cubes)
        ferCubes2Del(gng->cubes);

    free(gng);
}

void ferGNG2Run(fer_gng2_t *gng)
{
    const fer_real_t *aabb;

    ferPC2Permutate(gng->pc);
    ferPC2ItInit(&gng->pcit, gng->pc);

    if (gng->num_cubes){
        if (gng->cubes)
            ferCubes2Del(gng->cubes);
        aabb = ferPC2AABB(gng->pc);
        gng->cubes = ferCubes2New(aabb, gng->num_cubes);
    }

    ferGNGRun(gng->gng);
}


static fer_gng_node_t *ferGNG2NewNode(const void *input_signal, void *data)
{
    fer_gng2_t *gng = (fer_gng2_t *)data;
    fer_gng2_node_t *n;

    n = FER_ALLOC(fer_gng2_node_t);
    n->w = ferVec2Clone((const fer_vec2_t *)input_signal);

    if (gng->cubes){
        ferCubes2ElInit(&n->cubes, n->w);
        ferCubes2Add(gng->cubes, &n->cubes);
    }

    return &n->node;
}

static fer_gng_node_t *ferGNG2NewNodeBetween(const fer_gng_node_t *_n1,
                                             const fer_gng_node_t *_n2,
                                             void *data)
{
    fer_vec2_t w;
    fer_gng2_node_t *n1, *n2;

    n1 = fer_container_of(_n1, fer_gng2_node_t, node);
    n2 = fer_container_of(_n2, fer_gng2_node_t, node);

    ferVec2Add2(&w, n1->w, n2->w);
    ferVec2Scale(&w, FER_REAL(0.5));

    return ferGNG2NewNode(&w, data);
}

static void ferGNG2DelNode(fer_gng_node_t *_n, void *data)
{
    fer_gng2_t *gng = (fer_gng2_t *)data;
    fer_gng2_node_t *n;

    n = fer_container_of(_n, fer_gng2_node_t, node);

    if (gng->cubes){
        ferCubes2Remove(gng->cubes, &n->cubes);
    }

    ferVec2Del(n->w);

    free(n);
}

static const void *ferGNG2InputSignal(void *data)
{
    fer_gng2_t *gng = (fer_gng2_t *)data;
    const fer_vec2_t *v;

    if (ferPC2ItEnd(&gng->pcit)){
        ferPC2Permutate(gng->pc);
        ferPC2ItInit(&gng->pcit, gng->pc);
    }

    v = ferPC2ItGet(&gng->pcit);
    ferPC2ItNext(&gng->pcit);

    return v;
}

static fer_real_t dist22(void *is, fer_list_t *nlist, void *_)
{
    fer_gng_node_t *gn;
    fer_gng2_node_t *n;

    gn = ferGNGNodeFromList(nlist);
    n = fer_container_of(gn, fer_gng2_node_t, node);
    return ferVec2Dist2((const fer_vec2_t *)is, n->w);
}

static void ferGNG2Nearest(const void *input_signal,
                           fer_gng_node_t **n1,
                           fer_gng_node_t **n2,
                           void *data)
{
    fer_gng2_t *gng = (fer_gng2_t *)data;

    if (gng->cubes){
        ferGNG2NearestCubes(input_signal, n1, n2, data);
    }else{
        ferGNG2NearestLinear(input_signal, n1, n2, data);
    }
}

_fer_inline void ferGNG2NearestLinear(const void *input_signal,
                                      fer_gng_node_t **n1,
                                      fer_gng_node_t **n2,
                                      void *data)
{
    fer_gng2_t *gng = (fer_gng2_t *)data;
    fer_list_t *ns[2];

    ns[0] = ns[1] = NULL;
    ferNearestLinear(ferGNGNodes(gng->gng), (void *)input_signal, dist22, ns, 2, NULL);

    *n1 = ferGNGNodeFromList(ns[0]);
    *n2 = ferGNGNodeFromList(ns[1]);
}

_fer_inline void ferGNG2NearestCubes(const void *input_signal,
                                     fer_gng_node_t **n1,
                                     fer_gng_node_t **n2,
                                     void *data)
{
    fer_cubes2_el_t *els[2];
    fer_gng2_node_t *n;
    fer_gng2_t *gng = (fer_gng2_t *)data;

    *n1 = *n2 = NULL;

    ferCubes2Nearest(gng->cubes, (const fer_vec2_t *)input_signal, 2, els);

    n = fer_container_of(els[0], fer_gng2_node_t, cubes);
    *n1 = &n->node;
    n = fer_container_of(els[1], fer_gng2_node_t, cubes);
    *n2 = &n->node;
}

static fer_real_t ferGNG2Dist2(const void *input_signal,
                               const fer_gng_node_t *node,
                               void *_)
{
    fer_gng2_node_t *n;
    n = fer_container_of(node, fer_gng2_node_t, node);
    return ferVec2Dist2((const fer_vec2_t *)input_signal, n->w);
}

static void ferGNG2MoveTowards(fer_gng_node_t *node,
                               const void *input_signal,
                               fer_real_t fraction, void *data)
{
    fer_gng2_t *gng = (fer_gng2_t *)data;
    fer_gng2_node_t *n;
    fer_vec2_t move;

    n = fer_container_of(node, fer_gng2_node_t, node);
    ferVec2Sub2(&move, (const fer_vec2_t *)input_signal, n->w);
    ferVec2Scale(&move, fraction);
    ferVec2Add(n->w, &move);

    if (gng->cubes){
        ferCubes2Update(gng->cubes, &n->cubes);
    }
}

void ferGNG2DumpSVT(fer_gng2_t *gng, FILE *out, const char *name)
{
    fer_list_t *list, *item;
    fer_net_node_t *nn;
    fer_gng_node_t *gn;
    fer_net_edge_t *e;
    fer_gng2_node_t *n;
    size_t i, id1, id2;

    fprintf(out, "--------\n");

    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Points:\n");
    list = ferGNGNodes(gng->gng);
    i = 0;
    FER_LIST_FOR_EACH(list, item){
        gn = ferGNGNodeFromList(item);
        n  = ferGNG2NodeFromGNG(gn);

        n->_id = i++;
        ferVec2Print(n->w, out);
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = ferGNGEdges(gng->gng);
    FER_LIST_FOR_EACH(list, item){
        e = FER_LIST_ENTRY(item, fer_net_edge_t, list);

        nn = ferNetEdgeNode(e, 0);
        n  = ferGNG2NodeFromNet(nn);
        id1 = n->_id;

        nn = ferNetEdgeNode(e, 1);
        n  = ferGNG2NodeFromNet(nn);
        id2 = n->_id;
        fprintf(out, "%d %d\n", (int)id1, (int)id2);
    }

    fprintf(out, "--------\n");
}
