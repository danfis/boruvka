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
#include <gann/gng`N`.h>
#include <fermat/nearest-linear.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Operations for gann_gng_ops_t struct */
_fer_inline gann_gng_node_t *gng`N`_new_node(const void *input_signal, void *);

_fer_inline gann_gng_node_t *gng`N`_new_node_between(const gann_gng_node_t *n1,
                                                   const gann_gng_node_t *n2,
                                                   void *);

static void gng`N`_del_node(gann_gng_node_t *n, void *);

_fer_inline const void *gng`N`_input_signal(void *);

_fer_inline void gng`N`_nearest(const void *input_signal,
                              gann_gng_node_t **n1,
                              gann_gng_node_t **n2,
                              void *);
_fer_inline void nearestLinear(const void *input_signal,
                               gann_gng_node_t **n1,
                               gann_gng_node_t **n2,
                               void *);
_fer_inline void nearestCubes(const void *input_signal,
                              gann_gng_node_t **n1,
                              gann_gng_node_t **n2,
                              void *);

_fer_inline fer_real_t gng`N`_dist2(const void *input_signal,
                                  const gann_gng_node_t *node,
                                  void *);

_fer_inline void gng`N`_move_towards(gann_gng_node_t *node,
                                   const void *input_signal,
                                   fer_real_t fraction, void *);


#define OPS(gng, name) gng`N`_ ## name
#define OPS_DATA(gng, name) (gng)->ops.name ## _data
#include "gng-algorithm.c"

void gannGNG`N`OpsInit(gann_gng`N`_ops_t *ops)
{
    bzero(ops, sizeof(gann_gng`N`_ops_t));
}

void gannGNG`N`ParamsInit(gann_gng`N`_params_t *p)
{
    gannGNGParamsInit(&p->gng);

    p->use_cubes = 1;
    p->num_cubes = 10000;
}

gann_gng`N`_t *gannGNG`N`New(const gann_gng`N`_ops_t *ops`N`,
                         const gann_gng`N`_params_t *params)
{
    gann_gng`N`_t *gng;
    gann_gng_ops_t ops;

    gng = FER_ALLOC(gann_gng`N`_t);

    gng->ops = *ops`N`;
    gng->params = *params;

    // set up callbacks' data pointers
    if (!gng->ops.terminate_data)
        gng->ops.terminate_data = gng->ops.data;
    if (!gng->ops.callback_data)
        gng->ops.callback_data = gng->ops.data;

    gannGNGOpsInit(&ops);
    ops.del_node         = gng`N`_del_node;
    ops.data = gng;

    ops.terminate        = gng->ops.terminate;
    ops.terminate_data   = gng->ops.terminate_data;
    ops.callback         = gng->ops.callback;
    ops.callback_period  = gng->ops.callback_period;
    ops.callback_data    = gng->ops.callback_data;

    gng->gng = gannGNGNew(&ops, &gng->params.gng);

    gng->pc = ferPC`N`New();

    gng->cubes = NULL;

    return gng;
}


void gannGNG`N`Del(gann_gng`N`_t *gng)
{
    ferPC`N`Del(gng->pc);
    gannGNGDel(gng->gng);

    if (gng->cubes)
        ferCubes`N`Del(gng->cubes);

    free(gng);
}

void gannGNG`N`Run(gann_gng`N`_t *gng)
{
    const fer_real_t *aabb;

    ferPC`N`Permutate(gng->pc);
    ferPC`N`ItInit(&gng->pcit, gng->pc);

    if (gng->params.use_cubes){
        if (gng->cubes)
            ferCubes`N`Del(gng->cubes);
        aabb = ferPC`N`AABB(gng->pc);
        gng->cubes = ferCubes`N`New(aabb, gng->params.num_cubes);
    }

    _gannGNGRun(gng->gng);
}


_fer_inline gann_gng_node_t *gng`N`_new_node(const void *input_signal, void *data)
{
    gann_gng`N`_t *gng = (gann_gng`N`_t *)data;
    gann_gng`N`_node_t *n;

    n = FER_ALLOC(gann_gng`N`_node_t);
    n->w = ferVec`N`Clone((const fer_vec`N`_t *)input_signal);

    if (gng->params.use_cubes){
        ferCubes`N`ElInit(&n->cubes, n->w);
        ferCubes`N`Add(gng->cubes, &n->cubes);
    }

    return &n->node;
}

_fer_inline gann_gng_node_t *gng`N`_new_node_between(const gann_gng_node_t *_n1,
                                                   const gann_gng_node_t *_n2,
                                                   void *data)
{
    fer_vec`N`_t w;
    gann_gng`N`_node_t *n1, *n2;

    n1 = fer_container_of(_n1, gann_gng`N`_node_t, node);
    n2 = fer_container_of(_n2, gann_gng`N`_node_t, node);

    ferVec`N`Add2(&w, n1->w, n2->w);
    ferVec`N`Scale(&w, FER_REAL(0.5));

    return gng`N`_new_node(&w, data);
}

static void gng`N`_del_node(gann_gng_node_t *_n, void *data)
{
    gann_gng`N`_t *gng = (gann_gng`N`_t *)data;
    gann_gng`N`_node_t *n;

    n = fer_container_of(_n, gann_gng`N`_node_t, node);

    if (gng->params.use_cubes){
        ferCubes`N`Remove(gng->cubes, &n->cubes);
    }

    ferVec`N`Del(n->w);

    free(n);
}

_fer_inline const void *gng`N`_input_signal(void *data)
{
    gann_gng`N`_t *gng = (gann_gng`N`_t *)data;
    const fer_vec`N`_t *v;

    if (ferPC`N`ItEnd(&gng->pcit)){
        ferPC`N`Permutate(gng->pc);
        ferPC`N`ItInit(&gng->pcit, gng->pc);
    }

    v = ferPC`N`ItGet(&gng->pcit);
    ferPC`N`ItNext(&gng->pcit);

    return v;
}

static fer_real_t dist22(void *is, fer_list_t *nlist)
{
    gann_gng_node_t *gn;
    gann_gng`N`_node_t *n;

    gn = gannGNGNodeFromList(nlist);
    n = fer_container_of(gn, gann_gng`N`_node_t, node);
    return ferVec`N`Dist2((const fer_vec`N`_t *)is, n->w);
}

_fer_inline void gng`N`_nearest(const void *input_signal,
                              gann_gng_node_t **n1,
                              gann_gng_node_t **n2,
                              void *data)
{
    gann_gng`N`_t *gng = (gann_gng`N`_t *)data;

    if (gng->params.use_cubes){
        nearestCubes(input_signal, n1, n2, data);
    }else{
        nearestLinear(input_signal, n1, n2, data);
    }
}

_fer_inline void nearestLinear(const void *input_signal,
                               gann_gng_node_t **n1,
                               gann_gng_node_t **n2,
                               void *data)
{
    gann_gng`N`_t *gng = (gann_gng`N`_t *)data;
    fer_list_t *ns[2];

    ns[0] = ns[1] = NULL;
    ferNearestLinear(gannGNGNodes(gng->gng), (void *)input_signal, dist22, ns, 2);

    *n1 = gannGNGNodeFromList(ns[0]);
    *n2 = gannGNGNodeFromList(ns[1]);
}

_fer_inline void nearestCubes(const void *input_signal,
                              gann_gng_node_t **n1,
                              gann_gng_node_t **n2,
                              void *data)
{
    fer_cubes`N`_el_t *els[2];
    gann_gng`N`_node_t *n;
    gann_gng`N`_t *gng = (gann_gng`N`_t *)data;

    *n1 = *n2 = NULL;

    ferCubes`N`Nearest(gng->cubes, (const fer_vec`N`_t *)input_signal, 2, els);

    n = fer_container_of(els[0], gann_gng`N`_node_t, cubes);
    *n1 = &n->node;
    n = fer_container_of(els[1], gann_gng`N`_node_t, cubes);
    *n2 = &n->node;
}

_fer_inline fer_real_t gng`N`_dist2(const void *input_signal,
                                  const gann_gng_node_t *node,
                                  void *_)
{
    gann_gng`N`_node_t *n;
    n = fer_container_of(node, gann_gng`N`_node_t, node);
    return ferVec`N`Dist2((const fer_vec`N`_t *)input_signal, n->w);
}

_fer_inline void gng`N`_move_towards(gann_gng_node_t *node,
                                   const void *input_signal,
                                   fer_real_t fraction,
                                   void *data)
{
    gann_gng`N`_t *gng = (gann_gng`N`_t *)data;
    gann_gng`N`_node_t *n;
    fer_vec`N`_t move;

    n = fer_container_of(node, gann_gng`N`_node_t, node);
    ferVec`N`Sub2(&move, (const fer_vec`N`_t *)input_signal, n->w);
    ferVec`N`Scale(&move, fraction);
    ferVec`N`Add(n->w, &move);

    if (gng->params.use_cubes){
        ferCubes`N`Update(gng->cubes, &n->cubes);
    }
}

void gannGNG`N`DumpSVT(gann_gng`N`_t *gng, FILE *out, const char *name)
{
    fer_list_t *list, *item;
    gann_net_node_t *nn;
    gann_gng_node_t *gn;
    gann_net_edge_t *e;
    gann_gng`N`_node_t *n;
    size_t i, id1, id2;

    fprintf(out, "--------\n");

    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Points:\n");
    list = gannGNGNodes(gng->gng);
    i = 0;
    ferListForEach(list, item){
        gn = gannGNGNodeFromList(item);
        n  = gannGNG`N`NodeFromGNG(gn);

        n->_id = i++;
        ferVec`N`Print(n->w, out);
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = gannGNGEdges(gng->gng);
    ferListForEach(list, item){
        e = ferListEntry(item, gann_net_edge_t, list);

        nn = gannNetEdgeNode(e, 0);
        n  = gannGNG`N`NodeFromNet(nn);
        id1 = n->_id;

        nn = gannNetEdgeNode(e, 1);
        n  = gannGNG`N`NodeFromNet(nn);
        id2 = n->_id;
        fprintf(out, "%d %d\n", id1, id2);
    }

    fprintf(out, "--------\n");
}
