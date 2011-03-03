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

static fer_prm_node_t *nodeNew(fer_prm_t *prm, const fer_vec_t *p);
static void nodeDel(fer_prm_t *prm, fer_prm_node_t *n);
static void edgeNew(fer_prm_t *prm, fer_prm_node_t *n1, fer_prm_node_t *n2);
static void nodeNetDel(fer_net_node_t *n, void *);
static void edgeNetDel(fer_net_edge_t *n, void *);

/** Finds maximaly prm->params.max_neighbors nearest to given node.
 *  Number of found nodes is returned. */
static size_t findNearest(fer_prm_t *prm, const fer_prm_node_t *n,
                          fer_prm_node_t **nearest, fer_nncells_el_t **els);
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

    params->num_cells = 10000;
    params->aabb[0] = -FER_ONE;
    params->aabb[1] =  FER_ONE;
    params->aabb[2] = -FER_ONE;
    params->aabb[3] =  FER_ONE;
}


fer_prm_t *ferPRMNew(const fer_prm_ops_t *ops,
                     const fer_prm_params_t *params)
{
    fer_prm_t *prm;

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
    prm->cells = ferNNCellsNew(prm->params.d, prm->params.aabb,
                               prm->params.num_cells);

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
    if (prm->cells)
        ferNNCellsDel(prm->cells);

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
    fer_nncells_el_t **tmp_nearest;
    size_t i, nearest_len;

    nearest = FER_ALLOC_ARR(fer_prm_node_t *, prm->params.max_neighbors);
    tmp_nearest = FER_ALLOC_ARR(fer_nncells_el_t *, prm->params.max_neighbors);

    while (!prm->ops.terminate(prm->ops.terminate_data)){
        // obtain random configuration
        c = prm->ops.conf(prm->ops.conf_data);

        // evaluate configuration
        if (prm->ops.eval(c, prm->ops.eval_data) == FER_PRM_FREE){
            // we have configuration in free space - create new node
            cn = nodeNew(prm, c);

            // obtain nearest nodes
            nearest_len = findNearest(prm, cn, nearest, tmp_nearest);

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
        }
    }

    free(nearest);
    free(tmp_nearest);
}



static fer_prm_node_t *nodeNew(fer_prm_t *prm, const fer_vec_t *p)
{
    fer_prm_node_t *n;

    n = FER_ALLOC(fer_prm_node_t);
    n->conf = ferVecClone(prm->params.d, p);
    n->comp = NULL;
    ferNetAddNode(prm->net, &n->node);

    ferNNCellsElInit(&n->cells, n->conf);
    ferNNCellsAdd(prm->cells, &n->cells);

    return n;
}

static void nodeDel(fer_prm_t *prm, fer_prm_node_t *n)
{
    if (n->conf)
        ferVecDel(n->conf);
    ferNetRemoveNode(prm->net, &n->node);
    ferNNCellsRemove(prm->cells, &n->cells);
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


static size_t findNearest(fer_prm_t *prm, const fer_prm_node_t *n,
                          fer_prm_node_t **nearest, fer_nncells_el_t **els)
{
    size_t found, i;

    found = ferNNCellsNearest(prm->cells, n->conf,
                              prm->params.max_neighbors, els);
    for (i = 0; i < found; i++){
        nearest[i] = fer_container_of(els[i], fer_prm_node_t, cells);
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
