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
#include <fermat/prm.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

static fer_prm_node_t *nodeNew(fer_prm_t *prm, const fer_vec_t *p);
static void nodeDel(fer_prm_t *prm, fer_prm_node_t *n);
static void edgeNew(fer_prm_t *prm, fer_prm_node_t *n1, fer_prm_node_t *n2);
static void edgeDel(fer_prm_t *prm, fer_net_edge_t *n);
static void nodeNetDel(fer_net_node_t *n, void *);
static void edgeNetDel(fer_net_edge_t *n, void *);
static fer_prm_node_t *connectNewNode(fer_prm_t *prm, const fer_vec_t *c);
static void nodeDelWithEdges(fer_prm_t *prm, fer_prm_node_t *n);

/** Finds maximaly prm->params.max_neighbors nearest to given node.
 *  Number of found nodes is returned. */
static size_t findNearest(fer_prm_t *prm, const fer_vec_t *conf,
                          fer_prm_node_t **nearest, fer_gug_el_t **els);
/** Creates new component consisting of one node */
static void componentNew(fer_prm_t *prm, fer_prm_node_t *n);
/** Returns true if two nodes belong to same component */
static int sameComponent(const fer_prm_node_t *n1, const fer_prm_node_t *n2);
/** Returns top component of node */
static fer_prm_component_t *topComponent(const fer_prm_node_t *n);


void ferPRMOpsInit(fer_prm_ops_t *ops)
{
    memset(ops, 0, sizeof(fer_prm_ops_t));
}

void ferPRMParamsInit(fer_prm_params_t *params)
{
    params->d = 2;

    params->max_dist = 0.001;
    params->max_neighbors = 10;

    ferGUGParamsInit(&params->gug);
}


fer_prm_t *ferPRMNew(const fer_prm_ops_t *ops,
                     const fer_prm_params_t *params)
{
    fer_prm_t *prm;
    fer_gug_params_t pcells;

    prm = FER_ALLOC(fer_prm_t);

    prm->params = *params;
    prm->ops    = *ops;
    if (prm->ops.conf_data == NULL)
        prm->ops.conf_data = prm->ops.data;
    if (prm->ops.terminate_data == NULL)
        prm->ops.terminate_data = prm->ops.data;
    if (prm->ops.eval_data == NULL)
        prm->ops.eval_data = prm->ops.data;
    if (prm->ops.find_path_data == NULL)
        prm->ops.find_path_data = prm->ops.data;
    if (prm->ops.callback_data == NULL)
        prm->ops.callback_data = prm->ops.data;

    prm->net = ferNetNew();

    pcells   = params->gug;
    pcells.dim = params->d;
    prm->gug = ferGUGNew(&pcells);

    ferListInit(&prm->components);

    return prm;
}

void ferPRMDel(fer_prm_t *prm)
{
    fer_list_t *item;
    fer_prm_component_t *comp;

    if (prm->net)
        ferNetDel2(prm->net,
                   nodeNetDel, (void *)prm,
                   edgeNetDel, (void *)prm);
    if (prm->gug)
        ferGUGDel(prm->gug);

    while (!ferListEmpty(&prm->components)){
        item = ferListNext(&prm->components);
        ferListDel(item);
        comp = fer_container_of(item, fer_prm_component_t, list);
        free(comp);
    }

    free(prm);
}

void ferPRMRun(fer_prm_t *prm)
{
    const fer_vec_t *c;
    fer_prm_node_t *cn;
    fer_prm_node_t **nearest;
    fer_gug_el_t **tmp_nearest;
    size_t i, nearest_len;
    unsigned long counter = 1;

    nearest = FER_ALLOC_ARR(fer_prm_node_t *, prm->params.max_neighbors);
    tmp_nearest = FER_ALLOC_ARR(fer_gug_el_t *, prm->params.max_neighbors);

    while (!prm->ops.terminate(prm->ops.terminate_data)){
        // obtain random configuration
        c = prm->ops.conf(prm->ops.conf_data);

        // evaluate configuration
        if (prm->ops.eval(c, prm->ops.eval_data) == FER_PRM_FREE){
            // we have configuration in free space

            // obtain nearest nodes
            nearest_len = findNearest(prm, c, nearest, tmp_nearest);

            // create new node
            cn = nodeNew(prm, c);

            // try to connect to found nodes
            for (i = 0; i < nearest_len; i++){
                // skip those that belongs to same component
                if (sameComponent(nearest[i], cn))
                    continue;

                // check if there is path between nodes
                if (prm->ops.find_path(nearest[i]->conf, cn->conf,
                                       prm->ops.find_path_data)){
                    edgeNew(prm, nearest[i], cn);
                }
            }

            if (cn->comp == NULL){
                componentNew(prm, cn);
            }

            if (counter == prm->ops.callback_period){
                prm->ops.callback(prm->ops.callback_data);
                counter = 0;
            }
            counter++;
        }
    }

    free(nearest);
    free(tmp_nearest);
}

void ferPRMDumpSVT(fer_prm_t *prm, FILE *out, const char *name)
{
    fer_list_t *list, *item;
    fer_net_node_t *nn;
    fer_prm_node_t *n;
    fer_net_edge_t *e;
    size_t i, id1, id2;

    fprintf(out, "--------\n");

    if (name){
        fprintf(out, "Name: %s\n", name);
    }

    fprintf(out, "Points:\n");
    list = ferNetNodes(prm->net);
    i = 0;
    FER_LIST_FOR_EACH(list, item){
        nn = FER_LIST_ENTRY(item, fer_net_node_t, list);
        n  = fer_container_of(nn, fer_prm_node_t, node);

        n->_id = i++;
        ferVec2Print((const fer_vec2_t *)n->conf, out);
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = ferNetEdges(prm->net);
    FER_LIST_FOR_EACH(list, item){
        e = FER_LIST_ENTRY(item, fer_net_edge_t, list);

        nn = ferNetEdgeNode(e, 0);
        n  = fer_container_of(nn, fer_prm_node_t, node);
        id1 = n->_id;

        nn = ferNetEdgeNode(e, 1);
        n  = fer_container_of(nn, fer_prm_node_t, node);
        id2 = n->_id;

        fprintf(out, "%d %d\n", (int)id1, (int)id2);
    }

    fprintf(out, "--------\n");
}


/*** Find path ***/
static void findPathExpand(fer_dij_node_t *_n, fer_list_t *expand, void *data)
{
    fer_prm_t *prm = (fer_prm_t *)data;
    fer_list_t *list, *item;
    fer_prm_node_t *n, *o;
    fer_net_edge_t *edge;
    fer_net_node_t *node;
    fer_real_t dist;

    n = fer_container_of(_n, fer_prm_node_t, dij);

    list = ferNetNodeEdges(&n->node);
    FER_LIST_FOR_EACH(list, item){
        edge = ferNetEdgeFromNodeList(item);
        node = ferNetEdgeOtherNode(edge, &n->node);
        o    = fer_container_of(node, fer_prm_node_t, node);

        if (!ferDijNodeClosed(&o->dij)){
            dist = ferVecDist(prm->params.d, n->conf, o->conf);
            ferDijNodeAdd(&o->dij, expand, dist);
        }
    }
}

/** Initializes all nodes in net for dijkstra search */
static void findPathDijInit(fer_prm_t *prm)
{
    fer_list_t *list, *item;
    fer_net_node_t *node;
    fer_prm_node_t *n;

    list = ferNetNodes(prm->net);
    FER_LIST_FOR_EACH(list, item){
        node = FER_LIST_ENTRY(item, fer_net_node_t, list);
        n    = fer_container_of(node, fer_prm_node_t, node);
        ferDijNodeInit(&n->dij);
    }
}

/** Fills given list by path from s to g.
 *  It is assumed that the path exists! */
static void obtainPath(fer_prm_node_t *s, fer_prm_node_t *g,
                       fer_list_t *list)
{
    fer_prm_node_t *n;
    fer_dij_node_t *dn;

    ferListPrepend(list, &g->path);
    dn = g->dij.prev;
    while (dn != &s->dij){
        n = fer_container_of(dn, fer_prm_node_t, dij);
        ferListPrepend(list, &n->path);

        dn = dn->prev;
    }
}

int ferPRMFindPath(fer_prm_t *prm,
                   const fer_vec_t *cstart, const fer_vec_t *cgoal,
                   fer_list_t *list)
{
    fer_dij_ops_t ops;
    fer_dij_t *dij;
    fer_prm_node_t *start, *goal;
    int result;

    // create start and goal nodes
    start = connectNewNode(prm, cstart);
    goal  = connectNewNode(prm, cgoal);

    // initialize whole net
    findPathDijInit(prm);

    // initialize operations
    ferDijOpsInit(&ops);
    ops.expand = findPathExpand;
    ops.data   = (void *)prm;

    // create dijkstra algorithm
    dij = ferDijNew(&ops);

    // run dijkstra
    result = ferDijRun(dij, &start->dij, &goal->dij);

    if (result == 0){
        obtainPath(start, goal, list);
        ferDijDel(dij);
        return 0;
    }

    // remove previously created nodes
    nodeDelWithEdges(prm, start);
    nodeDelWithEdges(prm, goal);

    ferDijDel(dij);

    return -1;
}



static fer_prm_node_t *nodeNew(fer_prm_t *prm, const fer_vec_t *p)
{
    fer_prm_node_t *n;

    n = FER_ALLOC(fer_prm_node_t);
    n->conf = ferVecClone(prm->params.d, p);
    n->comp = NULL;
    ferNetAddNode(prm->net, &n->node);

    ferGUGElInit(&n->gug, n->conf);
    ferGUGAdd(prm->gug, &n->gug);

    return n;
}

static void nodeDel(fer_prm_t *prm, fer_prm_node_t *n)
{
    if (n->conf)
        ferVecDel(n->conf);
    ferNetRemoveNode(prm->net, &n->node);
    ferGUGRemove(prm->gug, &n->gug);
    free(n);
}

static void edgeNew(fer_prm_t *prm, fer_prm_node_t *n1, fer_prm_node_t *n2)
{
    fer_net_edge_t *e;
    fer_prm_component_t *c1, *c2;

    e = FER_ALLOC(fer_net_edge_t);
    ferNetAddEdge(prm->net, e, &n1->node, &n2->node);

    if (n1->comp == NULL){
        n1->comp = n2->comp;
    }else if (n2->comp == NULL){
        n2->comp = n1->comp;
    }else{
        c1 = topComponent(n1);
        c2 = topComponent(n2);
        c1->parent = c2;
    }
}

static void edgeDel(fer_prm_t *prm, fer_net_edge_t *e)
{
    ferNetRemoveEdge(prm->net, e);
    free(e);
}

static void nodeNetDel(fer_net_node_t *_n, void *_)
{
    fer_prm_node_t *n;

    n = fer_container_of(_n, fer_prm_node_t, node);
    if (n->conf)
        ferVecDel(n->conf);
    free(n);
}

static void edgeNetDel(fer_net_edge_t *n, void *_)
{
    free(n);
}


static size_t findNearest(fer_prm_t *prm, const fer_vec_t *conf,
                          fer_prm_node_t **nearest, fer_gug_el_t **els)
{
    size_t size, found;
    fer_prm_node_t *m;

    size = ferGUGNearest(prm->gug, conf,
                              prm->params.max_neighbors, els);

    for (found = 0; found < size; found++){
        m = fer_container_of(els[found], fer_prm_node_t, gug);

        if (ferVecDist(prm->params.d, m->conf, conf) < prm->params.max_dist){
            nearest[found] = m;
        }else{
            break;
        }
    }

    return found;
}

static void componentNew(fer_prm_t *prm, fer_prm_node_t *n)
{
    fer_prm_component_t *comp;

    comp = FER_ALLOC(fer_prm_component_t);
    comp->parent = NULL;
    ferListAppend(&prm->components, &comp->list);

    n->comp = comp;
}

static int sameComponent(const fer_prm_node_t *n1, const fer_prm_node_t *n2)
{
    fer_prm_component_t *c1, *c2;
    if (n1->comp == NULL || n2->comp == NULL)
        return 0;

    c1 = topComponent(n1);
    c2 = topComponent(n2);

    return c1 == c2;
}

static fer_prm_component_t *topComponent(const fer_prm_node_t *n)
{
    fer_prm_component_t *c;

    if (n->comp == NULL)
        return NULL;

    c = n->comp;
    while (c->parent != NULL)
        c = c->parent;

    return c;
}

static fer_prm_node_t *connectNewNode(fer_prm_t *prm, const fer_vec_t *c)
{
    fer_prm_node_t **nearest;
    fer_gug_el_t **tmp_nearest;
    size_t nearest_len, i;
    fer_prm_node_t *n;

    nearest = FER_ALLOC_ARR(fer_prm_node_t *, prm->params.max_neighbors);
    tmp_nearest = FER_ALLOC_ARR(fer_gug_el_t *, prm->params.max_neighbors);

    nearest_len = findNearest(prm, c, nearest, tmp_nearest);

    n = nodeNew(prm, c);

    for (i = 0; i < nearest_len; i++){
        // skip those that belongs to same component
        if (sameComponent(nearest[i], n))
            continue;

        // check if there is path between nodes
        if (prm->ops.find_path(nearest[i]->conf, n->conf,
                               prm->ops.find_path_data)){
            edgeNew(prm, nearest[i], n);
        }
    }

    free(nearest);
    free(tmp_nearest);

    return n;
}

static void nodeDelWithEdges(fer_prm_t *prm, fer_prm_node_t *n)
{
    fer_list_t *list, *item, *item_tmp;
    fer_net_edge_t *edge;

    list = ferNetNodeEdges(&n->node);
    FER_LIST_FOR_EACH_SAFE(list, item, item_tmp){
        edge = ferNetEdgeFromNodeList(item);
        edgeDel(prm, edge);
    }
    nodeDel(prm, n);
}
