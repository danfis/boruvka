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

#include <string.h>
#include <fermat/rrt.h>
#include <fermat/alloc.h>

static fer_rrt_node_t *nodeNew(fer_rrt_t *rrt, const fer_vec_t *p);
static void nodeDel(fer_rrt_t *rrt, fer_rrt_node_t *n);
static void edgeNew(fer_rrt_t *rrt, fer_rrt_node_t *n1, fer_rrt_node_t *n2);
static void edgeDel(fer_rrt_t *rrt, fer_net_edge_t *n);
static void nodeNetDel(fer_net_node_t *n, void *);
static void edgeNetDel(fer_net_edge_t *n, void *);
/*
static fer_rrt_node_t *connectNewNode(fer_rrt_t *rrt, const fer_vec_t *c);
static void nodeDelWithEdges(fer_rrt_t *rrt, fer_rrt_node_t *n);
*/

void ferRRTOpsInit(fer_rrt_ops_t *ops)
{
    memset(ops, 0, sizeof(fer_rrt_ops_t));
}

void ferRRTParamsInit(fer_rrt_params_t *params)
{
    params->d = 2;

    ferNNCellsParamsInit(&params->cells);
}

fer_rrt_t *ferRRTNew(const fer_rrt_ops_t *ops,
                     const fer_rrt_params_t *params)
{
    fer_rrt_t *rrt;
    fer_nncells_params_t pcells;

    rrt = FER_ALLOC(fer_rrt_t);

    rrt->params = *params;
    rrt->ops    = *ops;
    if (rrt->ops.conf_data == NULL)
        rrt->ops.conf_data = rrt->ops.data;
    if (rrt->ops.terminate_data == NULL)
        rrt->ops.terminate_data = rrt->ops.data;
    if (rrt->ops.new_conf_data == NULL)
        rrt->ops.new_conf_data = rrt->ops.data;
    if (rrt->ops.callback_data == NULL)
        rrt->ops.callback_data = rrt->ops.data;

    rrt->net = ferNetNew();

    pcells   = params->cells;
    pcells.d = params->d;
    rrt->cells = ferNNCellsNew(&pcells);

    rrt->node_init = NULL;
    rrt->node_last = NULL;

    return rrt;
}

void ferRRTDel(fer_rrt_t *rrt)
{
    if (rrt->net)
        ferNetDel2(rrt->net, nodeNetDel, (void *)rrt,
                             edgeNetDel, (void *)rrt);

    if (rrt->cells)
        ferNNCellsDel(rrt->cells);

    free(rrt);
}

void ferRRTRun(fer_rrt_t *rrt, const fer_vec_t *init)
{
    fer_rrt_node_t *n, *near, *new;
    fer_nncells_el_t *el;
    const fer_vec_t *rand_conf, *new_conf;
    unsigned long counter;

    // create inital node
    n = nodeNew(rrt, init);
    rrt->node_init = rrt->node_last = n;

    counter = 1;
    while (!rrt->ops.terminate(rrt->ops.terminate_data)){
        // get random configuration
        rand_conf = rrt->ops.conf(rrt->ops.conf_data);

        // get nearest node from net
        ferNNCellsNearest(rrt->cells, rand_conf, 1, &el);
        near = fer_container_of(el, fer_rrt_node_t, cells);

        // get new configuration
        new_conf = rrt->ops.new_conf(near->conf, rand_conf,
                                     rrt->ops.new_conf_data);

        if (!new_conf)
            continue;

        // add node to net
        new = nodeNew(rrt, new_conf);
        rrt->node_last = new;
        // and connect in with nearest node
        edgeNew(rrt, near, new);

        if (rrt->ops.callback && counter == rrt->ops.callback_period){
            rrt->ops.callback(rrt->ops.callback_data);
            counter = 0L;
        }
        counter += 1L;
    }
}

const fer_rrt_node_t *ferRRTNodeNew(fer_rrt_t *rrt, const fer_vec_t *conf,
                                    const fer_rrt_node_t *_n)
{
    fer_rrt_node_t *n = (fer_rrt_node_t *)_n;
    fer_rrt_node_t *new;

    new = nodeNew(rrt, conf);
    edgeNew(rrt, new, n);

    return new;
}

void ferRRTDumpSVT(fer_rrt_t *rrt, FILE *out, const char *name)
{
    fer_list_t *list, *item;
    fer_net_node_t *nn;
    fer_rrt_node_t *n;
    fer_net_edge_t *e;
    size_t i, id1, id2;

    fprintf(out, "--------\n");

    if (name){
        fprintf(out, "Name: %s\n", name);
    }

    fprintf(out, "Points:\n");
    list = ferNetNodes(rrt->net);
    i = 0;
    ferListForEach(list, item){
        nn = ferListEntry(item, fer_net_node_t, list);
        n  = fer_container_of(nn, fer_rrt_node_t, node);

        n->_id = i++;
        ferVec2Print((const fer_vec2_t *)n->conf, out);
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = ferNetEdges(rrt->net);
    ferListForEach(list, item){
        e = ferListEntry(item, fer_net_edge_t, list);

        nn = ferNetEdgeNode(e, 0);
        n  = fer_container_of(nn, fer_rrt_node_t, node);
        id1 = n->_id;

        nn = ferNetEdgeNode(e, 1);
        n  = fer_container_of(nn, fer_rrt_node_t, node);
        id2 = n->_id;

        fprintf(out, "%d %d\n", id1, id2);
    }

    fprintf(out, "--------\n");
}



/*** Find path ***/
static fer_real_t findPathDist(const fer_dij_node_t *_n1,
                               const fer_dij_node_t *_n2, void *data)
{
    fer_rrt_t *rrt = (fer_rrt_t *)data;

    fer_rrt_node_t *n1, *n2;
    n1 = fer_container_of(_n1, fer_rrt_node_t, dij);
    n2 = fer_container_of(_n2, fer_rrt_node_t, dij);
    return ferVecDist(rrt->params.d, n1->conf, n2->conf);
}

static void findPathExpand(fer_dij_node_t *_n, fer_list_t *expand, void *_)
{
    fer_list_t *list, *item;
    fer_rrt_node_t *n, *o;
    fer_net_edge_t *edge;
    fer_net_node_t *node;

    n = fer_container_of(_n, fer_rrt_node_t, dij);

    list = ferNetNodeEdges(&n->node);
    ferListForEach(list, item){
        edge = ferNetEdgeFromNodeList(item);
        node = ferNetEdgeOtherNode(edge, &n->node);
        o    = fer_container_of(node, fer_rrt_node_t, node);

        if (!ferDijNodeClosed(&o->dij)){
            ferDijNodeAdd(&o->dij, expand);
        }
    }
}

/** Initializes all nodes in net for dijkstra search */
static void findPathDijInit(fer_rrt_t *rrt)
{
    fer_list_t *list, *item;
    fer_net_node_t *node;
    fer_rrt_node_t *n;

    list = ferNetNodes(rrt->net);
    ferListForEach(list, item){
        node = ferListEntry(item, fer_net_node_t, list);
        n    = fer_container_of(node, fer_rrt_node_t, node);
        ferDijNodeInit(&n->dij);
    }
}

/** Fills given list by path from s to g.
 *  It is assumed that the path exists! */
static void obtainPath(fer_rrt_node_t *s, fer_rrt_node_t *g,
                       fer_list_t *list)
{
    fer_rrt_node_t *n;
    fer_dij_node_t *dn;

    ferListPrepend(list, &g->path);
    dn = g->dij.prev;
    while (dn != &s->dij){
        n = fer_container_of(dn, fer_rrt_node_t, dij);
        ferListPrepend(list, &n->path);

        dn = dn->prev;
    }
}
int ferRRTFindPath(fer_rrt_t *rrt,
                   const fer_rrt_node_t *_init, const fer_rrt_node_t *_goal,
                   fer_list_t *list)
{
    fer_dij_ops_t ops;
    fer_dij_t *dij;
    fer_rrt_node_t *init, *goal;
    int result;

    init = (fer_rrt_node_t *)_init;
    goal = (fer_rrt_node_t *)_goal;

    // initialize whole net
    findPathDijInit(rrt);

    // initialize operations
    ferDijOpsInit(&ops);
    ops.dist   = findPathDist;
    ops.expand = findPathExpand;
    ops.data   = (void *)rrt;

    // create dijkstra algorithm
    dij = ferDijNew(&ops);

    // run dijkstra
    result = ferDijRun(dij, &init->dij, &goal->dij);

    if (result == 0){
        obtainPath(init, goal, list);
        ferDijDel(dij);
        return 0;
    }

    ferDijDel(dij);

    return -1;
}



static fer_rrt_node_t *nodeNew(fer_rrt_t *rrt, const fer_vec_t *p)
{
    fer_rrt_node_t *n;

    n = FER_ALLOC(fer_rrt_node_t);
    n->conf = ferVecClone(rrt->params.d, p);

    ferNetAddNode(rrt->net, &n->node);

    ferNNCellsElInit(&n->cells, n->conf);
    ferNNCellsAdd(rrt->cells, &n->cells);

    return n;
}

static void nodeDel(fer_rrt_t *rrt, fer_rrt_node_t *n)
{
    ferVecDel(n->conf);
    free(n);
}

static void edgeNew(fer_rrt_t *rrt, fer_rrt_node_t *n1, fer_rrt_node_t *n2)
{
    fer_net_edge_t *e;

    e = FER_ALLOC(fer_net_edge_t);
    ferNetAddEdge(rrt->net, e, &n1->node, &n2->node);
}

static void edgeDel(fer_rrt_t *rrt, fer_net_edge_t *e)
{
    free(e);
}

static void nodeNetDel(fer_net_node_t *_n, void *_)
{
    fer_rrt_node_t *n;

    n = fer_container_of(_n, fer_rrt_node_t, node);

    ferVecDel(n->conf);
    free(n);
}

static void edgeNetDel(fer_net_edge_t *n, void *_)
{
    free(n);
}
