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

#include <fermat/gann/net.h>
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

gann_net_conn_t *gannNetNodeCommonConn(const gann_net_node_t *v1,
                                       const gann_net_node_t *v2)
{
    fer_list_t *item;
    fer_list_m_t *mitem;
    const gann_net_node_t *vtmp;
    gann_net_conn_t *e;

    // set v1 as node with less conns
    if (gannNetNodeConnsLen(v2) < gannNetNodeConnsLen(v1)){
        FER_SWAP(v1, v2, vtmp);
    }

    ferListForEach(&v1->conns, item){
        mitem = ferListMFromList(item);
        e = ferListEntry(item, gann_net_conn_t, nlist[mitem->mark]);
        if (v2 == gannNetConnNode(e, 0)
                || v2 == gannNetConnNode(e, 1)){
            return e;
        }
    }

    return NULL;
}


gann_net_conn_t *gannNetConnNew(void)
{
    gann_net_conn_t *e;
    e = FER_ALLOC(gann_net_conn_t);
    return e;
}

void gannNetConnDel(gann_net_conn_t *e)
{
    free(e);
}

/** Returns true if two given conns have exactly one common node */
_fer_inline int gannNetConnTriCheckCommon(const gann_net_conn_t *e1,
                                           const gann_net_conn_t *e2)
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

int gannNetConnTriCheck(const gann_net_conn_t *e1,
                        const gann_net_conn_t *e2,
                        const gann_net_conn_t *e3)
{
    // 1) Any two conns must have exactly one common node.
    // 2) Start and end vertices must differ (within one conn)
    // I think that if these two preconditions hold then it is certain that
    // conns form triangle.

    return gannNetConnTriCheckCommon(e1, e2)
                && gannNetConnTriCheckCommon(e1, e3)
                && gannNetConnTriCheckCommon(e2, e3);
}






gann_net_t *gannNetNew(void)
{
    gann_net_t *m;
    m = FER_ALLOC(gann_net_t);

    ferListInit(&m->nodes);
    m->nodes_len = 0;
    ferListInit(&m->conns);
    m->conns_len = 0;

    return m;
}

void gannNetDel(gann_net_t *m)
{
    gannNetDel2(m, NULL, NULL, NULL, NULL);
}

void gannNetDel2(gann_net_t *m,
                  void (*delnode)(gann_net_node_t *, void *), void *vdata,
                  void (*delconn)(gann_net_conn_t *, void *), void *edata)
{
    gann_net_node_t *v;
    gann_net_conn_t *e;
    fer_list_t *item;

    // disconnect all conns
    while (!ferListEmpty(&m->conns)){
        item = ferListNext(&m->conns);
        e = ferListEntry(item, gann_net_conn_t, list);
        gannNetRemoveConn(m, e);

        if (delconn){
            delconn(e, edata);
        }
    }

    // disconnect all vertices
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

    ferListInit(&v->conns);
    v->conns_len = 0;
}

int gannNetRemoveNode(gann_net_t *m, gann_net_node_t *v)
{
    if (!ferListEmpty(&v->conns))
        return -1;

    ferListDel(&v->list);
    m->nodes_len--;
    return 0;
}

void gannNetAddConn(gann_net_t *m, gann_net_conn_t *e,
                     gann_net_node_t *start, gann_net_node_t *end)
{
    // assign start and end point
    e->n[0] = start;
    e->n[1] = end;

    // append conn to list of conns in vertices
    e->nlist[0].mark = 0;
    ferListAppend(&start->conns, ferListMAsList(&e->nlist[0]));
    start->conns_len++;
    e->nlist[1].mark = 1;
    ferListAppend(&end->conns, ferListMAsList(&e->nlist[1]));
    end->conns_len++;

    // add conn to list of all conns
    ferListAppend(&m->conns, &e->list);
    m->conns_len++;
}

void gannNetRemoveConn(gann_net_t *m, gann_net_conn_t *e)
{
    // remove conn from lists in vertices
    ferListDel(ferListMAsList(&e->nlist[0]));
    e->n[0]->conns_len--;
    ferListDel(ferListMAsList(&e->nlist[1]));
    e->n[1]->conns_len--;

    // remove conn from list of all conns
    ferListDel(&e->list);
    m->conns_len--;
}
