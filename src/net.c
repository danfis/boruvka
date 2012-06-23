/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <boruvka/net.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

bor_net_node_t *borNetNodeNew(void)
{
    bor_net_node_t *n;
    n = BOR_ALLOC(bor_net_node_t);
    return n;
}

void borNetNodeDel(bor_net_node_t *v)
{
    BOR_FREE(v);
}

bor_net_edge_t *borNetNodeCommonEdge(const bor_net_node_t *v1,
                                       const bor_net_node_t *v2)
{
    bor_list_t *item;
    bor_list_m_t *mitem;
    const bor_net_node_t *vtmp;
    bor_net_edge_t *e;

    // set v1 as node with less edges
    if (borNetNodeEdgesLen(v2) < borNetNodeEdgesLen(v1)){
        BOR_SWAP(v1, v2, vtmp);
    }

    BOR_LIST_FOR_EACH(&v1->edges, item){
        mitem = borListMFromList(item);
        e = BOR_LIST_M_ENTRY(item, bor_net_edge_t, nlist, mitem->mark);
        if (v2 == borNetEdgeNode(e, 0)
                || v2 == borNetEdgeNode(e, 1)){
            return e;
        }
    }

    return NULL;
}


bor_net_edge_t *borNetEdgeNew(void)
{
    bor_net_edge_t *e;
    e = BOR_ALLOC(bor_net_edge_t);
    return e;
}

void borNetEdgeDel(bor_net_edge_t *e)
{
    BOR_FREE(e);
}

/** Returns true if two given edges have exactly one common node */
_bor_inline int borNetEdgeTriCheckCommon(const bor_net_edge_t *e1,
                                           const bor_net_edge_t *e2)
{
    if (e1->n[0] == e2->n[0]){
        if (e1->n[1] == e2->n[1])
            return 0; // e1 and e2 have two common vertices
    }else if (e1->n[1] == e2->n[1]){
        if (e1->n[0] == e2->n[0])
            return 0; // e1 and e2 have two common vertices
    }else if (e1->n[1] == e2->n[0]){
        if (e1->n[0] == e2->n[1])
            return 0; // e1 and e2 have two common vertices
    }else{
        return 0; // e1 and e2 have no common node
    }
    return 1;
}

int borNetEdgeTriCheck(const bor_net_edge_t *e1,
                        const bor_net_edge_t *e2,
                        const bor_net_edge_t *e3)
{
    // 1) Any two edges must have exactly one common node.
    // 2) Start and end vertices must differ (within one edge)
    // I think that if these two preconditions hold then it is certain that
    // edges form triangle.

    return borNetEdgeTriCheckCommon(e1, e2)
                && borNetEdgeTriCheckCommon(e1, e3)
                && borNetEdgeTriCheckCommon(e2, e3);
}






bor_net_t *borNetNew(void)
{
    bor_net_t *m;
    m = BOR_ALLOC(bor_net_t);

    borListInit(&m->nodes);
    m->nodes_len = 0;
    borListInit(&m->edges);
    m->edges_len = 0;

    return m;
}

void borNetDel(bor_net_t *m)
{
    borNetDel2(m, NULL, NULL, NULL, NULL);
}

void borNetDel2(bor_net_t *m,
                  void (*delnode)(bor_net_node_t *, void *), void *vdata,
                  void (*deledge)(bor_net_edge_t *, void *), void *edata)
{
    bor_net_node_t *v;
    bor_net_edge_t *e;
    bor_list_t *item;

    // disedgeect all edges
    while (!borListEmpty(&m->edges)){
        item = borListNext(&m->edges);
        e = BOR_LIST_ENTRY(item, bor_net_edge_t, list);
        borNetRemoveEdge(m, e);

        if (deledge){
            deledge(e, edata);
        }
    }

    // disedgeect all vertices
    while (!borListEmpty(&m->nodes)){
        item = borListNext(&m->nodes);
        v = BOR_LIST_ENTRY(item, bor_net_node_t, list);
        borNetRemoveNode(m, v);

        if (delnode){
            delnode(v, vdata);
        }
    }

    BOR_FREE(m);
}

void borNetAddNode(bor_net_t *m, bor_net_node_t *v)
{
    borListAppend(&m->nodes, &v->list);
    m->nodes_len++;

    borListInit(&v->edges);
    v->edges_len = 0;
}

int borNetRemoveNode(bor_net_t *m, bor_net_node_t *v)
{
    if (!borListEmpty(&v->edges))
        return -1;

    borListDel(&v->list);
    m->nodes_len--;
    return 0;
}

void borNetAddEdge(bor_net_t *m, bor_net_edge_t *e,
                     bor_net_node_t *start, bor_net_node_t *end)
{
    // assign start and end point
    e->n[0] = start;
    e->n[1] = end;

    // append edge to list of edges in vertices
    e->nlist[0].mark = 0;
    borListAppend(&start->edges, borListMAsList(&e->nlist[0]));
    start->edges_len++;
    e->nlist[1].mark = 1;
    borListAppend(&end->edges, borListMAsList(&e->nlist[1]));
    end->edges_len++;

    // add edge to list of all edges
    borListAppend(&m->edges, &e->list);
    m->edges_len++;
}

void borNetRemoveEdge(bor_net_t *m, bor_net_edge_t *e)
{
    // remove edge from lists in vertices
    borListDel(borListMAsList(&e->nlist[0]));
    e->n[0]->edges_len--;
    borListDel(borListMAsList(&e->nlist[1]));
    e->n[1]->edges_len--;

    // remove edge from list of all edges
    borListDel(&e->list);
    m->edges_len--;
}


void borNetTraverseNode(bor_net_t *net,
                        void (*cb)(bor_net_t *net, bor_net_node_t *n, void *data),
                        void *data)
{
    bor_list_t *item;
    bor_net_node_t *n;

    BOR_LIST_FOR_EACH(&net->nodes, item){
        n = BOR_LIST_ENTRY(item, bor_net_node_t, list);
        cb(net, n, data);
    }
}

void borNetTraverseEdge(bor_net_t *net,
                        void (*cb)(bor_net_t *net, bor_net_edge_t *e, void *data),
                        void *data)
{
    bor_list_t *item;
    bor_net_edge_t *e;

    BOR_LIST_FOR_EACH(&net->edges, item){
        e = BOR_LIST_ENTRY(item, bor_net_edge_t, list);
        cb(net, e, data);
    }
}

