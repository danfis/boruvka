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

#include <boruvka/mesh3.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

bor_mesh3_vertex_t *borMesh3VertexNew(void)
{
    bor_mesh3_vertex_t *v;
    v = BOR_ALLOC(bor_mesh3_vertex_t);
    v->v = NULL;
    return v;
}

void borMesh3VertexDel(bor_mesh3_vertex_t *v)
{
    BOR_FREE(v);
}

bor_mesh3_edge_t *borMesh3VertexCommonEdge(const bor_mesh3_vertex_t *v1,
                                           const bor_mesh3_vertex_t *v2)
{
    bor_list_t *item;
    bor_list_m_t *mitem;
    const bor_mesh3_vertex_t *vtmp;
    bor_mesh3_edge_t *e;

    // set v1 as vertex with less edges
    if (borMesh3VertexEdgesLen(v2) < borMesh3VertexEdgesLen(v1)){
        BOR_SWAP(v1, v2, vtmp);
    }

    BOR_LIST_FOR_EACH(&v1->edges, item){
        mitem = borListMFromList(item);
        e = BOR_LIST_M_ENTRY(item, bor_mesh3_edge_t, vlist, mitem->mark);
        if (v2 == borMesh3EdgeVertex(e, 0)
                || v2 == borMesh3EdgeVertex(e, 1)){
            return e;
        }
    }

    return NULL;
}


bor_mesh3_edge_t *borMesh3EdgeNew(void)
{
    bor_mesh3_edge_t *e;
    e = BOR_ALLOC(bor_mesh3_edge_t);
    return e;
}

void borMesh3EdgeDel(bor_mesh3_edge_t *e)
{
    BOR_FREE(e);
}

/** Returns true if two given edges have exactly one common vertex */
_bor_inline int borMesh3EdgeTriCheckCommon(const bor_mesh3_edge_t *e1,
                                           const bor_mesh3_edge_t *e2)
{
    if (e1->v[0] == e2->v[0]){
        if (e1->v[1] == e2->v[1])
            return 0; // e1 and e2 have two common vertices
    }else if (e1->v[1] == e2->v[1]){
        if (e1->v[0] == e2->v[0])
            return 0; // e1 and e2 have two common vertices
    }else if (e1->v[1] == e2->v[0]){
        if (e1->v[0] == e2->v[1])
            return 0; // e1 and e2 have two common vertices
    }else{
        return 0; // e1 and e2 have no common vertex
    }
    return 1;
}

int borMesh3EdgeTriCheck(const bor_mesh3_edge_t *e1,
                         const bor_mesh3_edge_t *e2,
                         const bor_mesh3_edge_t *e3)
{
    // 1) Any two edges must have exactly one common vertex.
    // 2) Start and end vertices must differ (within one edge)
    // I think that if these two preconditions hold then it is certain that
    // edges form triangle.

    return borMesh3EdgeTriCheckCommon(e1, e2)
                && borMesh3EdgeTriCheckCommon(e1, e3)
                && borMesh3EdgeTriCheckCommon(e2, e3);
}




bor_mesh3_face_t *borMesh3FaceNew(void)
{
    bor_mesh3_face_t *f;
    f = BOR_ALLOC(bor_mesh3_face_t);
    return f;
}

void borMesh3FaceDel(bor_mesh3_face_t *f)
{
    BOR_FREE(f);
}



bor_mesh3_t *borMesh3New(void)
{
    bor_mesh3_t *m;
    m = BOR_ALLOC(bor_mesh3_t);

    borListInit(&m->verts);
    m->verts_len = 0;
    borListInit(&m->edges);
    m->edges_len = 0;
    borListInit(&m->faces);
    m->faces_len = 0;

    return m;
}

void borMesh3Del(bor_mesh3_t *m)
{
    borMesh3Del2(m, NULL, NULL, NULL, NULL, NULL, NULL);
}

void borMesh3Del2(bor_mesh3_t *m,
                  void (*delvertex)(bor_mesh3_vertex_t *, void *), void *vdata,
                  void (*deledge)(bor_mesh3_edge_t *, void *), void *edata,
                  void (*delface)(bor_mesh3_face_t *, void *), void *fdata)
{
    bor_mesh3_vertex_t *v;
    bor_mesh3_edge_t *e;
    bor_mesh3_face_t *f;
    bor_list_t *item;

    // disconnect all faces
    while (!borListEmpty(&m->faces)){
        item = borListNext(&m->faces);
        f = BOR_LIST_ENTRY(item, bor_mesh3_face_t, list);
        borMesh3RemoveFace(m, f);

        if (delface){
            delface(f, fdata);
        }
    }

    // disconnect all edges
    while (!borListEmpty(&m->edges)){
        item = borListNext(&m->edges);
        e = BOR_LIST_ENTRY(item, bor_mesh3_edge_t, list);
        borMesh3RemoveEdge(m, e);

        if (deledge){
            deledge(e, edata);
        }
    }

    // disconnect all vertices
    while (!borListEmpty(&m->verts)){
        item = borListNext(&m->verts);
        v = BOR_LIST_ENTRY(item, bor_mesh3_vertex_t, list);
        borMesh3RemoveVertex(m, v);

        if (delvertex){
            delvertex(v, vdata);
        }
    }

    BOR_FREE(m);
}

void borMesh3AddVertex(bor_mesh3_t *m, bor_mesh3_vertex_t *v)
{
    borListAppend(&m->verts, &v->list);
    m->verts_len++;

    borListInit(&v->edges);
    v->edges_len = 0;
}

int borMesh3RemoveVertex(bor_mesh3_t *m, bor_mesh3_vertex_t *v)
{
    if (!borListEmpty(&v->edges))
        return -1;

    borListDel(&v->list);
    m->verts_len--;
    return 0;
}

void borMesh3AddEdge(bor_mesh3_t *m, bor_mesh3_edge_t *e,
                     bor_mesh3_vertex_t *start, bor_mesh3_vertex_t *end)
{
    // assign start and end point
    e->v[0] = start;
    e->v[1] = end;

    // append edge to list of edges in vertices
    e->vlist[0].mark = 0;
    borListAppend(&start->edges, borListMAsList(&e->vlist[0]));
    start->edges_len++;
    e->vlist[1].mark = 1;
    borListAppend(&end->edges, borListMAsList(&e->vlist[1]));
    end->edges_len++;

    // add edge to list of all edges
    borListAppend(&m->edges, &e->list);
    m->edges_len++;

    // initialize incidenting faces
    e->f[0] = e->f[1] = NULL;
}

int borMesh3RemoveEdge(bor_mesh3_t *m, bor_mesh3_edge_t *e)
{
    if (e->f[0] != NULL || e->f[1] != NULL)
        return -1;

    // remove edge from lists in vertices
    borListDel(borListMAsList(&e->vlist[0]));
    e->v[0]->edges_len--;
    borListDel(borListMAsList(&e->vlist[1]));
    e->v[1]->edges_len--;

    // remove edge from list of all edges
    borListDel(&e->list);
    m->edges_len--;

    // reset .v[]
    e->v[0] = e->v[1] = NULL;

    return 0;
}

int borMesh3AddFace(bor_mesh3_t *m, bor_mesh3_face_t *f,
                    bor_mesh3_edge_t *e1, bor_mesh3_edge_t *e2,
                    bor_mesh3_edge_t *e3)
{
    if (borMesh3EdgeFacesLen(e1) == 2
            || borMesh3EdgeFacesLen(e2) == 2
            || borMesh3EdgeFacesLen(e3) == 2)
        return -1;

    // assign edges and also back pointers
    f->e[0] = e1;
    if (!e1->f[0]){
        e1->f[0] = f;
    }else{
        e1->f[1] = f;
    }

    f->e[1] = e2;
    if (!e2->f[0]){
        e2->f[0] = f;
    }else{
        e2->f[1] = f;
    }

    f->e[2] = e3;
    if (!e3->f[0]){
        e3->f[0] = f;
    }else{
        e3->f[1] = f;
    }

    // add face to list of all faces
    borListAppend(&m->faces, &f->list);
    m->faces_len++;

    return 0;
}

void borMesh3RemoveFace(bor_mesh3_t *m, bor_mesh3_face_t *f)
{
    size_t i;

    // disconnect face from edges
    for (i = 0; i < 3; i++){
        if (f->e[i]->f[0] == f)
            f->e[i]->f[0] = f->e[i]->f[1];
        f->e[i]->f[1] = NULL;
    }

    // disconnect face from list of all faces
    borListDel(&f->list);
    m->faces_len--;

    // zeroize pointers to edges
    f->e[0] = f->e[1] = f->e[2] = NULL;
}

void borMesh3DumpSVT(bor_mesh3_t *m, FILE *out, const char *name)
{
    bor_list_t *item;
    bor_mesh3_vertex_t *v;
    bor_mesh3_vertex_t *vs[3];
    bor_mesh3_edge_t *e;
    bor_mesh3_face_t *f;
    size_t i;

    fprintf(out, "--------\n");
    if (name){
        fprintf(out, "Name: %s\n", name);
    }

    fprintf(out, "Points:\n");
    i = 0;
    BOR_LIST_FOR_EACH(&m->verts, item){
        v = BOR_LIST_ENTRY(item, bor_mesh3_vertex_t, list);
        v->_id = i++;
        fprintf(out, "%g %g %g\n",
                borVec3X(borMesh3VertexCoords(v)),
                borVec3Y(borMesh3VertexCoords(v)),
                borVec3Z(borMesh3VertexCoords(v)));
    }

    fprintf(out, "Edges:\n");
    BOR_LIST_FOR_EACH(&m->edges, item){
        e = BOR_LIST_ENTRY(item, bor_mesh3_edge_t, list);
        fprintf(out, "%d %d\n", borMesh3EdgeVertex(e, 0)->_id,
                                borMesh3EdgeVertex(e, 1)->_id);
    }

    fprintf(out, "Faces:\n");
    BOR_LIST_FOR_EACH(&m->faces, item){
        f = BOR_LIST_ENTRY(item, bor_mesh3_face_t, list);
        borMesh3FaceVertices(f, vs);
        fprintf(out, "%d %d %d\n", vs[0]->_id, vs[1]->_id, vs[2]->_id);
    }

    fprintf(out, "--------\n");
}

void borMesh3DumpTriangles(bor_mesh3_t *m, FILE *out)
{
    bor_mesh3_vertex_t *vs[3];
    bor_mesh3_face_t *f;
    bor_list_t *item;

    BOR_LIST_FOR_EACH(&m->faces, item){
        f = BOR_LIST_ENTRY(item, bor_mesh3_face_t, list);
        borMesh3FaceVertices(f, vs);

        fprintf(out, "%g %g %g %g %g %g %g %g %g\n",
                borVec3X(borMesh3VertexCoords(vs[0])),
                borVec3Y(borMesh3VertexCoords(vs[0])),
                borVec3Z(borMesh3VertexCoords(vs[0])),
                borVec3X(borMesh3VertexCoords(vs[1])),
                borVec3Y(borMesh3VertexCoords(vs[1])),
                borVec3Z(borMesh3VertexCoords(vs[1])),
                borVec3X(borMesh3VertexCoords(vs[2])),
                borVec3Y(borMesh3VertexCoords(vs[2])),
                borVec3Z(borMesh3VertexCoords(vs[2])));
    }

    fflush(out);
}

void borMesh3DumpPovray(bor_mesh3_t *m, FILE *out)
{
    bor_mesh3_vertex_t *vs[3];
    bor_mesh3_face_t *f;
    bor_list_t *item;

    fprintf(out, "mesh {\n");

    BOR_LIST_FOR_EACH(&m->faces, item){
        f = BOR_LIST_ENTRY(item, bor_mesh3_face_t, list);
        borMesh3FaceVertices(f, vs);

        fprintf(out, "triangle { <%g %g %g>, <%g %g %g>, <%g %g %g> }\n",
                borVec3X(borMesh3VertexCoords(vs[0])),
                borVec3Y(borMesh3VertexCoords(vs[0])),
                borVec3Z(borMesh3VertexCoords(vs[0])),
                borVec3X(borMesh3VertexCoords(vs[1])),
                borVec3Y(borMesh3VertexCoords(vs[1])),
                borVec3Z(borMesh3VertexCoords(vs[1])),
                borVec3X(borMesh3VertexCoords(vs[2])),
                borVec3Y(borMesh3VertexCoords(vs[2])),
                borVec3Z(borMesh3VertexCoords(vs[2])));
    }

    fprintf(out, "}\n");

    fflush(out);
}
