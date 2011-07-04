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
#include <fermat/gng3.h>
#include <fermat/nearest-linear.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/** Operations for fer_gng_ops_t struct */
_fer_inline void gng3_init(fer_gng_node_t **n1, fer_gng_node_t **n2, void *_)
{}

_fer_inline fer_gng_node_t *gng3_new_node(const void *input_signal, void *);

_fer_inline fer_gng_node_t *gng3_new_node_between(const fer_gng_node_t *n1,
                                                    const fer_gng_node_t *n2,
                                                    void *);

static void gng3_del_node(fer_gng_node_t *n, void *);

_fer_inline const void *gng3_input_signal(void *);

_fer_inline void gng3_nearest(const void *input_signal,
                               fer_gng_node_t **n1,
                               fer_gng_node_t **n2,
                               void *);
_fer_inline void nearestLinear(const void *input_signal,
                               fer_gng_node_t **n1,
                               fer_gng_node_t **n2,
                               void *);
_fer_inline void nearestCubes(const void *input_signal,
                              fer_gng_node_t **n1,
                              fer_gng_node_t **n2,
                              void *);

_fer_inline fer_real_t gng3_dist2(const void *input_signal,
                                  const fer_gng_node_t *node,
                                  void *);

_fer_inline void gng3_move_towards(fer_gng_node_t *node,
                                   const void *input_signal,
                                   fer_real_t fraction, void *);


#define OPS(gng, name) gng3_ ## name
#define OPS_DATA(gng, name) (gng)->ops.name ## _data
#define NO_CONNECT_NEW_NODE
#include "gng-algorithm.c"

void ferGNG3OpsInit(fer_gng3_ops_t *ops)
{
    bzero(ops, sizeof(fer_gng3_ops_t));
}

void ferGNG3ParamsInit(fer_gng3_params_t *p)
{
    ferGNGParamsInit(&p->gng);

    p->use_cubes = 1;
    p->num_cubes = 10000;
}

fer_gng3_t *ferGNG3New(const fer_gng3_ops_t *ops3,
                         const fer_gng3_params_t *params)
{
    fer_gng3_t *gng;
    fer_gng_ops_t ops;

    gng = FER_ALLOC(fer_gng3_t);

    gng->ops = *ops3;
    gng->params = *params;

    // set up callbacks' data pointers
    if (!gng->ops.terminate_data)
        gng->ops.terminate_data = gng->ops.data;
    if (!gng->ops.callback_data)
        gng->ops.callback_data = gng->ops.data;

    ferGNGOpsInit(&ops);
    ops.del_node         = gng3_del_node;
    ops.data = gng;

    ops.terminate        = gng->ops.terminate;
    ops.terminate_data   = gng->ops.terminate_data;
    ops.callback         = gng->ops.callback;
    ops.callback_period  = gng->ops.callback_period;
    ops.callback_data    = gng->ops.callback_data;

    gng->gng = ferGNGNew(&ops, &gng->params.gng);

    gng->pc = ferPC3New();

    gng->cubes = NULL;

    return gng;
}


void ferGNG3Del(fer_gng3_t *gng)
{
    ferPC3Del(gng->pc);
    ferGNGDel(gng->gng);

    if (gng->cubes)
        ferCubes3Del(gng->cubes);

    free(gng);
}

void ferGNG3Run(fer_gng3_t *gng)
{
    const fer_real_t *aabb;

    ferPC3Permutate(gng->pc);
    ferPC3ItInit(&gng->pcit, gng->pc);

    if (gng->params.use_cubes){
        if (gng->cubes)
            ferCubes3Del(gng->cubes);
        aabb = ferPC3AABB(gng->pc);
        gng->cubes = ferCubes3New(aabb, gng->params.num_cubes);
    }

    _ferGNGRun(gng->gng);
}


_fer_inline fer_gng_node_t *gng3_new_node(const void *input_signal, void *data)
{
    fer_gng3_t *gng = (fer_gng3_t *)data;
    fer_gng3_node_t *n;

    n = FER_ALLOC(fer_gng3_node_t);
    n->w = ferVec3Clone((const fer_vec3_t *)input_signal);

    if (gng->params.use_cubes){
        ferCubes3ElInit(&n->cubes, n->w);
        ferCubes3Add(gng->cubes, &n->cubes);
    }

    return &n->node;
}

_fer_inline fer_gng_node_t *gng3_new_node_between(const fer_gng_node_t *_n1,
                                                   const fer_gng_node_t *_n2,
                                                   void *data)
{
    fer_vec3_t w;
    fer_gng3_node_t *n1, *n2;

    n1 = fer_container_of(_n1, fer_gng3_node_t, node);
    n2 = fer_container_of(_n2, fer_gng3_node_t, node);

    ferVec3Add2(&w, n1->w, n2->w);
    ferVec3Scale(&w, FER_REAL(0.5));

    return gng3_new_node(&w, data);
}

static void gng3_del_node(fer_gng_node_t *_n, void *data)
{
    fer_gng3_t *gng = (fer_gng3_t *)data;
    fer_gng3_node_t *n;

    n = fer_container_of(_n, fer_gng3_node_t, node);

    if (gng->params.use_cubes){
        ferCubes3Remove(gng->cubes, &n->cubes);
    }

    ferVec3Del(n->w);

    free(n);
}

_fer_inline const void *gng3_input_signal(void *data)
{
    fer_gng3_t *gng = (fer_gng3_t *)data;
    const fer_vec3_t *v;

    if (ferPC3ItEnd(&gng->pcit)){
        ferPC3Permutate(gng->pc);
        ferPC3ItInit(&gng->pcit, gng->pc);
    }

    v = ferPC3ItGet(&gng->pcit);
    ferPC3ItNext(&gng->pcit);

    return v;
}

static fer_real_t dist22(void *is, fer_list_t *nlist, void *_)
{
    fer_gng_node_t *gn;
    fer_gng3_node_t *n;

    gn = ferGNGNodeFromList(nlist);
    n = fer_container_of(gn, fer_gng3_node_t, node);
    return ferVec3Dist2((const fer_vec3_t *)is, n->w);
}

_fer_inline void gng3_nearest(const void *input_signal,
                              fer_gng_node_t **n1,
                              fer_gng_node_t **n2,
                              void *data)
{
    fer_gng3_t *gng = (fer_gng3_t *)data;

    if (gng->params.use_cubes){
        nearestCubes(input_signal, n1, n2, data);
    }else{
        nearestLinear(input_signal, n1, n2, data);
    }
}

_fer_inline void nearestLinear(const void *input_signal,
                               fer_gng_node_t **n1,
                               fer_gng_node_t **n2,
                               void *data)
{
    fer_gng3_t *gng = (fer_gng3_t *)data;
    fer_list_t *ns[2];

    ns[0] = ns[1] = NULL;
    ferNearestLinear(ferGNGNodes(gng->gng), (void *)input_signal, dist22, ns, 2, NULL);

    *n1 = ferGNGNodeFromList(ns[0]);
    *n2 = ferGNGNodeFromList(ns[1]);
}

_fer_inline void nearestCubes(const void *input_signal,
                              fer_gng_node_t **n1,
                              fer_gng_node_t **n2,
                              void *data)
{
    fer_cubes3_el_t *els[2];
    fer_gng3_node_t *n;
    fer_gng3_t *gng = (fer_gng3_t *)data;

    *n1 = *n2 = NULL;

    ferCubes3Nearest(gng->cubes, (const fer_vec3_t *)input_signal, 2, els);

    n = fer_container_of(els[0], fer_gng3_node_t, cubes);
    *n1 = &n->node;
    n = fer_container_of(els[1], fer_gng3_node_t, cubes);
    *n2 = &n->node;
}

_fer_inline fer_real_t gng3_dist2(const void *input_signal,
                                  const fer_gng_node_t *node,
                                  void *_)
{
    fer_gng3_node_t *n;
    n = fer_container_of(node, fer_gng3_node_t, node);
    return ferVec3Dist2((const fer_vec3_t *)input_signal, n->w);
}

_fer_inline void gng3_move_towards(fer_gng_node_t *node,
                                   const void *input_signal,
                                   fer_real_t fraction,
                                   void *data)
{
    fer_gng3_t *gng = (fer_gng3_t *)data;
    fer_gng3_node_t *n;
    fer_vec3_t move;

    n = fer_container_of(node, fer_gng3_node_t, node);
    ferVec3Sub2(&move, (const fer_vec3_t *)input_signal, n->w);
    ferVec3Scale(&move, fraction);
    ferVec3Add(n->w, &move);

    if (gng->params.use_cubes){
        ferCubes3Update(gng->cubes, &n->cubes);
    }
}

void ferGNG3DumpSVT(fer_gng3_t *gng, FILE *out, const char *name)
{
    fer_list_t *list, *item;
    fer_net_node_t *nn;
    fer_gng_node_t *gn;
    fer_net_edge_t *e;
    fer_gng3_node_t *n;
    size_t i, id1, id2;

    fprintf(out, "--------\n");

    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Points:\n");
    list = ferGNGNodes(gng->gng);
    i = 0;
    FER_LIST_FOR_EACH(list, item){
        gn = ferGNGNodeFromList(item);
        n  = ferGNG3NodeFromGNG(gn);

        n->_id = i++;
        ferVec3Print(n->w, out);
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = ferGNGEdges(gng->gng);
    FER_LIST_FOR_EACH(list, item){
        e = FER_LIST_ENTRY(item, fer_net_edge_t, list);

        nn = ferNetEdgeNode(e, 0);
        n  = ferGNG3NodeFromNet(nn);
        id1 = n->_id;

        nn = ferNetEdgeNode(e, 1);
        n  = ferGNG3NodeFromNet(nn);
        id2 = n->_id;
        fprintf(out, "%d %d\n", (int)id1, (int)id2);
    }

    fprintf(out, "--------\n");
}
