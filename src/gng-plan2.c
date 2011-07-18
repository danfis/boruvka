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

#include <fermat/gng-plan2.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>


static const void *ferGNGPlanInputSignal(void *);
static void ferGNGPlanInit(fer_gng_node_t **n1, fer_gng_node_t **n2, void *);
static fer_gng_node_t *ferGNGPlanNewNode(const void *input_signal, void *);
static fer_gng_node_t *ferGNGPlanNewNodeBetween(const fer_gng_node_t *n1,
                                                const fer_gng_node_t *n2,
                                                void *);
static void ferGNGPlanDelNode(fer_gng_node_t *n, void *);
static void ferGNGPlanNearest(const void *input_signal,
                              fer_gng_node_t **n1, fer_gng_node_t **n2,
                              void *);
static fer_real_t ferGNGPlanDist2(const void *input_signal,
                                  const fer_gng_node_t *node, void *);
static void ferGNGPlanMoveTowards(fer_gng_node_t *node,
                                  const void *input_signal,
                                  fer_real_t fraction, void *);


void ferGNGPlanOpsInit(fer_gng_plan_ops_t *ops)
{
    memset(ops, 0, sizeof(*ops));
}


void ferGNGPlanParamsInit(fer_gng_plan_params_t *params)
{
    ferGNGParamsInit(&params->gng);
    ferNNCellsParamsInit(&params->cells);
}


fer_gng_plan_t *ferGNGPlanNew(const fer_gng_plan_ops_t *ops,
                              const fer_gng_plan_params_t *params)
{
    fer_gng_plan_t *gng;
    fer_gng_ops_t gng_ops;

    gng = FER_ALLOC(fer_gng_plan_t);
    gng->dim = params->dim;

    ferGNGOpsInit(&gng_ops);
    gng_ops.terminate         = ops->terminate;
    gng_ops.callback          = ops->callback;
    gng_ops.callback_period   = ops->callback_period;
    gng_ops.terminate_data    = ops->terminate_data;
    gng_ops.callback_data     = ops->callback_data;
    if (!gng_ops.terminate_data)
        gng_ops.terminate_data = ops->data;
    if (!gng_ops.callback_data)
        gng_ops.callback_data = ops->data;
    gng_ops.data = gng;
    gng_ops.input_signal     = ferGNGPlanInputSignal;
    gng_ops.init             = ferGNGPlanInit;
    gng_ops.new_node         = ferGNGPlanNewNode;
    gng_ops.new_node_between = ferGNGPlanNewNodeBetween;
    gng_ops.del_node         = ferGNGPlanDelNode;
    gng_ops.nearest          = ferGNGPlanNearest;
    gng_ops.dist2            = ferGNGPlanDist2;
    gng_ops.move_towards     = ferGNGPlanMoveTowards;

    gng->gng = ferGNGNew(&gng_ops, &params->gng);

    gng->cells = ferNNCellsNew(&params->cells);

    gng->eval = ops->eval;
    gng->input_signal = ops->input_signal;
    gng->eval_data = ops->eval_data;
    gng->input_signal_data = ops->input_signal_data;
    if (!gng->eval_data)
        gng->eval_data = ops->data;
    if (!gng->input_signal_data)
        gng->input_signal_data = ops->data;

    gng->tmpv = ferVecNew(gng->dim);

    return gng;
}

void ferGNGPlanDel(fer_gng_plan_t *gng)
{
    ferGNGDel(gng->gng);
    ferNNCellsDel(gng->cells);
    ferVecDel(gng->tmpv);
    free(gng);
}

void ferGNGPlanRun(fer_gng_plan_t *gng)
{
    ferGNGRun(gng->gng);
}

static const void *ferGNGPlanInputSignal(void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    const fer_vec_t *vec;

    vec = gng->input_signal(gng->input_signal_data);
    // TODO

    return (void *)vec;
}

static void ferGNGPlanInit(fer_gng_node_t **n1, fer_gng_node_t **n2, void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;

    // TODO
    ferVecSetZero(gng->dim, gng->tmpv);
    *n1 = ferGNGPlanNewNode((const void *)gng->tmpv, (void *)gng);
    *n2 = ferGNGPlanNewNode((const void *)gng->tmpv, (void *)gng);
}

static fer_gng_node_t *ferGNGPlanNewNode(const void *input_signal, void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    fer_gng_plan_node_t *node;

    node = FER_ALLOC(fer_gng_plan_node_t);

    node->w = ferVecClone(gng->dim, (const fer_vec_t *)input_signal);

    ferNNCellsElInit(&node->cells, node->w);
    ferNNCellsAdd(gng->cells, &node->cells);

    return &node->node;
}

static fer_gng_node_t *ferGNGPlanNewNodeBetween(const fer_gng_node_t *_n1,
                                                const fer_gng_node_t *_n2,
                                                void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    fer_gng_plan_node_t *n1 = (fer_gng_plan_node_t *)_n1;
    fer_gng_plan_node_t *n2 = (fer_gng_plan_node_t *)_n2;

    ferVecAdd2(gng->dim, gng->tmpv, n1->w, n2->w);
    ferVecScale(gng->dim, gng->tmpv, FER_REAL(0.5));

    return ferGNGPlanNewNode((const void *)gng->tmpv, (void *)gng);
}

static void ferGNGPlanDelNode(fer_gng_node_t *_n, void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    fer_gng_plan_node_t *n = (fer_gng_plan_node_t *)_n;

    ferVecDel(n->w);
    ferNNCellsRemove(gng->cells, &n->cells);
    free(n);
}

static void ferGNGPlanNearest(const void *input_signal,
                              fer_gng_node_t **n1_out, fer_gng_node_t **n2_out,
                              void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    fer_nncells_el_t *els[2];
    fer_gng_plan_node_t *n1, *n2;

    ferNNCellsNearest(gng->cells, (const fer_vec_t *)input_signal, 2, els);
    n1 = fer_container_of(els[0], fer_gng_plan_node_t, cells);
    n2 = fer_container_of(els[1], fer_gng_plan_node_t, cells);

    *n1_out = &n1->node;
    *n2_out = &n2->node;
}

static fer_real_t ferGNGPlanDist2(const void *input_signal,
                                  const fer_gng_node_t *node, void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    const fer_gng_plan_node_t *n; 

    n = fer_container_of(node, fer_gng_plan_node_t, node);

    return ferVecDist2(gng->dim, (const fer_vec_t *)input_signal, n->w);
}

static void ferGNGPlanMoveTowards(fer_gng_node_t *node,
                                  const void *input_signal,
                                  fer_real_t fraction, void *data)
{
    fer_gng_plan_t *gng = (fer_gng_plan_t *)data;
    fer_gng_plan_node_t *n; 

    n = fer_container_of(node, fer_gng_plan_node_t, node);

    ferVecSub2(gng->dim, gng->tmpv, (const fer_vec_t *)input_signal, n->w);
    ferVecScale(gng->dim, gng->tmpv, fraction);
    ferVecAdd(gng->dim, n->w, gng->tmpv);

    ferNNCellsUpdate(gng->cells, &n->cells);
}
