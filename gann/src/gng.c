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

#include <gann/gng.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

#define OPS(gng, name) (gng)->ops.name
#define OPS_DATA(gng, name) (gng)->ops.name ## _data
#include "gng-algorithm.c"

/** Delete callbacks */
static void nodeFinalDel(gann_net_node_t *node, void *data);
static void delEdge(gann_net_edge_t *edge, void *data);

void gannGNGOpsInit(gann_gng_ops_t *ops)
{
    bzero(ops, sizeof(gann_gng_ops_t));
}

void gannGNGParamsInit(gann_gng_params_t *params)
{
    params->lambda  = 200;
    params->eb      = 0.05;
    params->en      = 0.0006;
    params->alpha   = 0.95;
    params->beta    = 0.9995;
    params->age_max = 200;
}


gann_gng_t *gannGNGNew(const gann_gng_ops_t *ops,
                       const gann_gng_params_t *params)
{
    gann_gng_t *gng;

    gng = FER_ALLOC(gann_gng_t);

    gng->net = gannNetNew();

    gng->ops    = *ops;
    gng->params = *params;

    // set up ops data pointers
    if (!gng->ops.init_data)
        gng->ops.init_data = gng->ops.data;
    if (!gng->ops.new_node_data)
        gng->ops.new_node_data = gng->ops.data;
    if (!gng->ops.new_node_between_data)
        gng->ops.new_node_between_data = gng->ops.data;
    if (!gng->ops.del_node_data)
        gng->ops.del_node_data = gng->ops.data;
    if (!gng->ops.input_signal_data)
        gng->ops.input_signal_data = gng->ops.data;
    if (!gng->ops.nearest_data)
        gng->ops.nearest_data = gng->ops.data;
    if (!gng->ops.dist2_data)
        gng->ops.dist2_data = gng->ops.data;
    if (!gng->ops.move_towards_data)
        gng->ops.move_towards_data = gng->ops.data;
    if (!gng->ops.terminate_data)
        gng->ops.terminate_data = gng->ops.data;
    if (!gng->ops.callback_data)
        gng->ops.callback_data = gng->ops.data;

    gng->beta_n = NULL;

    return gng;
}

void gannGNGDel(gann_gng_t *gng)
{
    if (gng->beta_n)
        free(gng->beta_n);

    if (gng->net){
        gannNetDel2(gng->net, nodeFinalDel, gng,
                              delEdge, gng);
    }

    free(gng);
}


void gannGNGRun(gann_gng_t *gng)
{
    _gannGNGRun(gng);
}

gann_gng_node_t *gannGNGConnectNewNode(gann_gng_t *gng, const void *is)
{
    return _gannGNGConnectNewNode(gng, is);
}


static void nodeFinalDel(gann_net_node_t *node, void *data)
{
    gann_gng_t *gng = (gann_gng_t *)data;
    gann_gng_node_t *n;

    n = fer_container_of(node, gann_gng_node_t, node);
    gng->ops.del_node(n, gng->ops.del_node_data);
}

static void delEdge(gann_net_edge_t *edge, void *data)
{
    free(edge);
}
