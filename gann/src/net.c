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

#include <gann/net.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

gann_net_node_t *gannNetNodeNew(void)
{
    gann_net_node_t *n;
    n = FER_ALLOC(gann_net_node_t);
    return n;
}

void gannNetNodeDel(gann_net_node_t *v)
{
    free(v);
}

gann_net_edge_t *gannNetNodeCommonEdge(const gann_net_node_t *v1,
                                       const gann_net_node_t *v2)
{
    fer_list_t *item;
    fer_list_m_t *mitem;
    const gann_net_node_t *vtmp;
    gann_net_edge_t *e;

    // set v1 as node with less edges
    if (gannNetNodeEdgesLen(v2) < gannNetNodeEdgesLen(v1)){
        FER_SWAP(v1, v2, vtmp);
    }

    ferListForEach(&v1->edges, item){
        mitem = ferListMFromList(item);
        e = ferListEntry(item, gann_net_edge_t, nlist[mitem->mark]);
        if (v2 == gannNetEdgeNode(e, 0)
                || v2 == gannNetEdgeNode(e, 1)){
            return e;
        }
    }

    return NULL;
}


gann_net_edge_t *gannNetEdgeNew(void)
{
    gann_net_edge_t *e;
    e = FER_ALLOC(gann_net_edge_t);
    return e;
}

void gannNetEdgeDel(gann_net_edge_t *e)
{
    free(e);
}

/** Returns true if two given edges have exactly one common node */
_fer_inline int gannNetEdgeTriCheckCommon(const gann_net_edge_t *e1,
                                           const gann_net_edge_t *e2)
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

int gannNetEdgeTriCheck(const gann_net_edge_t *e1,
                        const gann_net_edge_t *e2,
                        const gann_net_edge_t *e3)
{
    // 1) Any two edges must have exactly one common node.
    // 2) Start and end vertices must differ (within one edge)
    // I think that if these two preconditions hold then it is certain that
    // edges form triangle.

    return gannNetEdgeTriCheckCommon(e1, e2)
                && gannNetEdgeTriCheckCommon(e1, e3)
                && gannNetEdgeTriCheckCommon(e2, e3);
}






gann_net_t *gannNetNew(void)
{
    gann_net_t *m;
    m = FER_ALLOC(gann_net_t);

    ferListInit(&m->nodes);
    m->nodes_len = 0;
    ferListInit(&m->edges);
    m->edges_len = 0;

    return m;
}

void gannNetDel(gann_net_t *m)
{
    gannNetDel2(m, NULL, NULL, NULL, NULL);
}

void gannNetDel2(gann_net_t *m,
                  void (*delnode)(gann_net_node_t *, void *), void *vdata,
                  void (*deledge)(gann_net_edge_t *, void *), void *edata)
{
    gann_net_node_t *v;
    gann_net_edge_t *e;
    fer_list_t *item;

    // disedgeect all edges
    while (!ferListEmpty(&m->edges)){
        item = ferListNext(&m->edges);
        e = ferListEntry(item, gann_net_edge_t, list);
        gannNetRemoveEdge(m, e);

        if (deledge){
            deledge(e, edata);
        }
    }

    // disedgeect all vertices
    while (!ferListEmpty(&m->nodes)){
        item = ferListNext(&m->nodes);
        v = ferListEntry(item, gann_net_node_t, list);
        gannNetRemoveNode(m, v);

        if (delnode){
            delnode(v, vdata);
        }
    }

    free(m);
}

void gannNetAddNode(gann_net_t *m, gann_net_node_t *v)
{
    ferListAppend(&m->nodes, &v->list);
    m->nodes_len++;

    ferListInit(&v->edges);
    v->edges_len = 0;
}

int gannNetRemoveNode(gann_net_t *m, gann_net_node_t *v)
{
    if (!ferListEmpty(&v->edges))
        return -1;

    ferListDel(&v->list);
    m->nodes_len--;
    return 0;
}

void gannNetAddEdge(gann_net_t *m, gann_net_edge_t *e,
                     gann_net_node_t *start, gann_net_node_t *end)
{
    // assign start and end point
    e->n[0] = start;
    e->n[1] = end;

    // append edge to list of edges in vertices
    e->nlist[0].mark = 0;
    ferListAppend(&start->edges, ferListMAsList(&e->nlist[0]));
    start->edges_len++;
    e->nlist[1].mark = 1;
    ferListAppend(&end->edges, ferListMAsList(&e->nlist[1]));
    end->edges_len++;

    // add edge to list of all edges
    ferListAppend(&m->edges, &e->list);
    m->edges_len++;
}

void gannNetRemoveEdge(gann_net_t *m, gann_net_edge_t *e)
{
    // remove edge from lists in vertices
    ferListDel(ferListMAsList(&e->nlist[0]));
    e->n[0]->edges_len--;
    ferListDel(ferListMAsList(&e->nlist[1]));
    e->n[1]->edges_len--;

    // remove edge from list of all edges
    ferListDel(&e->list);
    m->edges_len--;
}
