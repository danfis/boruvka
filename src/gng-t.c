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

#include <fermat/gng-t.h>
#include <fermat/dbg.h>
#include <fermat/alloc.h>

static void ferGNGTHebbianLearning(fer_gngt_t *gng,
                                   fer_gngt_node_t *n1,
                                   fer_gngt_node_t *n2);
static fer_gngt_node_t *ferGNGTNodeNeighborWithHighestErr(fer_gngt_t *gng,
                                                          fer_gngt_node_t *n);

/** Delete callbacks */
static void nodeFinalDel(fer_net_node_t *node, void *data);
static void delEdge(fer_net_edge_t *edge, void *data);

void ferGNGTOpsInit(fer_gngt_ops_t *ops)
{
    bzero(ops, sizeof(fer_gngt_ops_t));
}

void ferGNGTParamsInit(fer_gngt_params_t *params)
{
    params->lambda  = 200;
    params->eb      = 0.05;
    params->en      = 0.0006;
    params->age_max = 200;
    params->target  = 100.;
}

fer_gngt_t *ferGNGTNew(const fer_gngt_ops_t *ops,
                       const fer_gngt_params_t *params)
{
    fer_gngt_t *gng;

    gng = FER_ALLOC(fer_gngt_t);

    gng->net = ferNetNew();

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

    return gng;
}

void ferGNGTDel(fer_gngt_t *gng)
{
    if (gng->net){
        ferNetDel2(gng->net, nodeFinalDel, gng,
                             delEdge, gng);
    }

    FER_FREE(gng);
}

void ferGNGTRun(fer_gngt_t *gng)
{
    unsigned long cycle = 0L;
    size_t i;

    ferGNGTInit(gng);

    do {
        ferGNGTReset(gng);
        for (i = 0; i < gng->params.lambda; i++){
            ferGNGTAdapt(gng);
        }

        ferGNGTGrowShrink(gng);

        cycle++;
        if (gng->ops.callback && gng->ops.callback_period == cycle){
            gng->ops.callback(gng->ops.callback_data);
            cycle = 0L;
        }
    } while (!gng->ops.terminate(gng->ops.terminate_data));
}

void ferGNGTInit(fer_gngt_t *gng)
{
    const void *is;
    fer_gngt_node_t *n1 = NULL, *n2 = NULL;

    if (gng->ops.init){
        gng->ops.init(&n1, &n2, gng->ops.init_data);
    }else{
        is = gng->ops.input_signal(gng->ops.input_signal_data);
        n1 = gng->ops.new_node(is, gng->ops.new_node_data);

        is = gng->ops.input_signal(gng->ops.input_signal_data);
        n2 = gng->ops.new_node(is, gng->ops.new_node_data);
    }

    ferGNGTNodeAdd(gng, n1);
    ferGNGTNodeAdd(gng, n2);
    ferGNGTEdgeNew(gng, n1, n2);
}

void ferGNGTReset(fer_gngt_t *gng)
{
    fer_list_t *list, *item;
    fer_gngt_node_t *n;

    list = ferGNGTNodes(gng);
    FER_LIST_FOR_EACH(list, item){
        n = ferGNGTNodeFromList(item);
        n->err = 0;
        n->won = 0;
    }
}

void ferGNGTAdapt(fer_gngt_t *gng)
{
    const void *is;
    fer_net_edge_t *ne;
    fer_net_node_t *nn;
    fer_gngt_node_t *n1, *n2;
    fer_gngt_edge_t *e;
    fer_real_t dist2;
    fer_list_t *list, *item, *item_tmp;

    // 1. Get input signal
    is = gng->ops.input_signal(gng->ops.input_signal_data);

    // 2. Find two nearest nodes to input signal
    gng->ops.nearest(is, &n1, &n2, gng->ops.nearest_data);
    n1->won = 1;

    // 3. Create (or refresh) an edge between n1 and n2
    ferGNGTHebbianLearning(gng, n1, n2);

    // 4. Update accumulator
    dist2 = gng->ops.dist2(is, n1, gng->ops.dist2_data);
    n1->err += dist2;

    // 5. Move winner node towards is
    gng->ops.move_towards(n1, is, gng->params.eb,
                          gng->ops.move_towards_data);

    // 6. Move n1's neighbors towards is
    // + 7. Increment age of all edges emanating from n1
    // + 8. Remove edges with age > age_max
    list = ferNetNodeEdges(&n1->node);
    FER_LIST_FOR_EACH_SAFE(list, item, item_tmp){
        ne = ferNetEdgeFromNodeList(item);
        e  = fer_container_of(ne, fer_gngt_edge_t, edge);
        nn = ferNetEdgeOtherNode(ne, &n1->node);
        n2 = fer_container_of(nn, fer_gngt_node_t, node);

        // increment age (7.)
        e->age += 1;

        // delete edge (8.)
        if (e->age > gng->params.age_max){
            ferGNGTEdgeDel(gng, e);

            if (ferNetNodeEdgesLen(&n2->node) == 0){
                // remove node if not connected into net anymore
                ferGNGTNodeDel(gng, n2);
            }
        }else{
            // move node (6.)
            gng->ops.move_towards(n2, is, gng->params.en,
                                  gng->ops.move_towards_data);
        }
    }

    // remove winning node if not connected into net
    if (ferNetNodeEdgesLen(&n1->node) == 0){
        // remove node if not connected into net anymore
        ferGNGTNodeDel(gng, n1);
    }
}

void ferGNGTGrowShrink(fer_gngt_t *gng)
{
    fer_list_t *list, *item, *item_tmp;
    fer_gngt_node_t *n, *max, *min, *max2;
    fer_gngt_edge_t *e;
    fer_real_t avg, num;

    avg = num = FER_ZERO;
    max = min = NULL;

    list = ferGNGTNodes(gng);
    FER_LIST_FOR_EACH_SAFE(list, item, item_tmp){
        n = ferGNGTNodeFromList(item);

        /*
        if (!n->won){
            ferGNGTNodeDel(gng, n);
            continue;
        }
        */

        avg += n->err;
        num += FER_ONE;

        if (!max || max->err < n->err)
            max = n;
        if (!min || min->err > n->err)
            min = n;
    }

    // compute average error
    avg /= num;
    gng->avg_err = avg;

    if (gng->params.target < avg){
        // more accuracy required
        if (max && (max2 = ferGNGTNodeNeighborWithHighestErr(gng, max))){
            n = gng->ops.new_node_between(max, max2,
                                          gng->ops.new_node_between_data);
            ferGNGTNodeAdd(gng, n);
            ferGNGTEdgeNew(gng, n, max);
            ferGNGTEdgeNew(gng, n, max2);

            e = ferGNGTEdgeBetween(gng, max, max2);
            ferGNGTEdgeDel(gng, e);
        }
    }else{
        // too much accuracy, remove the node with the smallest error
        if (min)
            ferGNGTNodeDel(gng, min);
    }

    if (ferGNGTNodesLen(gng) < 2){
        fprintf(stderr, "GNG-T Error: Check the parameters! The network shrinks too much.\n");
        exit(-1);
    }
}




void ferGNGTNodeDisconnect(fer_gngt_t *gng, fer_gngt_node_t *n)
{
    fer_list_t *edges, *item, *itemtmp;
    fer_net_edge_t *ne;
    fer_gngt_edge_t *edge;

    // remove incidenting edges
    edges = ferNetNodeEdges(&n->node);
    FER_LIST_FOR_EACH_SAFE(edges, item, itemtmp){
        ne = ferNetEdgeFromNodeList(item);
        edge = ferGNGTEdgeFromNet(ne);
        ferGNGTEdgeDel(gng, edge);
    }
}

fer_gngt_node_t *ferGNGTNodeNewAtPos(fer_gngt_t *gng, const void *is)
{
    fer_gngt_node_t *r, *n1, *n2;

    gng->ops.nearest(is, &n1, &n2, gng->ops.nearest_data);

    r = gng->ops.new_node(is, gng->ops.new_node_data);
    ferGNGTNodeAdd(gng, r);

    ferGNGTEdgeNew(gng, r, n1);
    ferGNGTEdgeNew(gng, r, n2);

    return r;
}

fer_gngt_edge_t *ferGNGTEdgeNew(fer_gngt_t *gng, fer_gngt_node_t *n1,
                                                 fer_gngt_node_t *n2)
{
    fer_gngt_edge_t *e;

    e = FER_ALLOC(fer_gngt_edge_t);
    e->age = 0;

    ferNetAddEdge(gng->net, &e->edge, &n1->node, &n2->node);

    return e;
}

void ferGNGTEdgeDel(fer_gngt_t *gng, fer_gngt_edge_t *edge)
{
    ferNetRemoveEdge(gng->net, &edge->edge);
    FER_FREE(edge);
}

void ferGNGTEdgeBetweenDel(fer_gngt_t *gng,
                           fer_gngt_node_t *n1, fer_gngt_node_t *n2)
{
    fer_gngt_edge_t *e;

    if ((e = ferGNGTEdgeBetween(gng, n1, n2)) != NULL)
        ferGNGTEdgeDel(gng, e);
}



static void ferGNGTHebbianLearning(fer_gngt_t *gng,
                                   fer_gngt_node_t *n1,
                                   fer_gngt_node_t *n2)
{
    fer_gngt_edge_t *e;

    e = ferGNGTEdgeBetween(gng, n1, n2);
    if (!e)
        e = ferGNGTEdgeNew(gng, n1, n2);
    e->age = 0;
}

static fer_gngt_node_t *ferGNGTNodeNeighborWithHighestErr(fer_gngt_t *gng,
                                                          fer_gngt_node_t *q)
{
    fer_list_t *list, *item;
    fer_net_edge_t *ne;
    fer_net_node_t *nn;
    fer_gngt_node_t *n, *max;
    fer_real_t err;

    max = NULL;
    err = -FER_REAL_MAX;

    list = ferNetNodeEdges(&q->node);
    FER_LIST_FOR_EACH(list, item){
        ne = ferNetEdgeFromNodeList(item);
        nn = ferNetEdgeOtherNode(ne, &q->node);
        n  = fer_container_of(nn, fer_gngt_node_t, node);

        if (n->err > err){
            err = n->err;
            max = n;
        }
    }

    return max;
}


static void nodeFinalDel(fer_net_node_t *node, void *data)
{
    fer_gngt_t *gng = (fer_gngt_t *)data;
    fer_gngt_node_t *n;

    n = fer_container_of(node, fer_gngt_node_t, node);
    gng->ops.del_node(n, gng->ops.del_node_data);
}

static void delEdge(fer_net_edge_t *edge, void *data)
{
    FER_FREE(edge);
}
