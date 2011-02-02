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

#include <fermat/mesh/mesh3.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

fer_mesh3_vertex_t *ferMesh3VertexNew(void)
{
    fer_mesh3_vertex_t *v;
    v = FER_ALLOC(fer_mesh3_vertex_t);
    v->v = NULL;
    return v;
}

void ferMesh3VertexDel(fer_mesh3_vertex_t *v)
{
    free(v);
}

fer_mesh3_edge_t *ferMesh3VertexCommonEdge(const fer_mesh3_vertex_t *v1,
                                           const fer_mesh3_vertex_t *v2)
{
    fer_list_t *item;
    fer_list_m_t *mitem;
    const fer_mesh3_vertex_t *vtmp;
    fer_mesh3_edge_t *e;

    // set v1 as vertex with less edges
    if (ferMesh3VertexEdgesLen(v2) < ferMesh3VertexEdgesLen(v1)){
        FER_SWAP(v1, v2, vtmp);
    }

    ferListForEach(&v1->edges, item){
        mitem = ferListMFromList(item);
        e = ferListEntry(item, fer_mesh3_edge_t, vlist[mitem->mark]);
        if (v2 == ferMesh3EdgeVertex(e, 0)
                || v2 == ferMesh3EdgeVertex(e, 1)){
            return e;
        }
    }

    return NULL;
}


fer_mesh3_edge_t *ferMesh3EdgeNew(void)
{
    fer_mesh3_edge_t *e;
    e = FER_ALLOC(fer_mesh3_edge_t);
    return e;
}

void ferMesh3EdgeDel(fer_mesh3_edge_t *e)
{
    free(e);
}

/** Returns true if two given edges have exactly one common vertex */
_fer_inline int ferMesh3EdgeTriCheckCommon(const fer_mesh3_edge_t *e1,
                                           const fer_mesh3_edge_t *e2)
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

int ferMesh3EdgeTriCheck(const fer_mesh3_edge_t *e1,
                         const fer_mesh3_edge_t *e2,
                         const fer_mesh3_edge_t *e3)
{
    // 1) Any two edges must have exactly one common vertex.
    // 2) Start and end vertices must differ (within one edge)
    // I think that if these two preconditions hold then it is certain that
    // edges form triangle.

    return ferMesh3EdgeTriCheckCommon(e1, e2)
                && ferMesh3EdgeTriCheckCommon(e1, e3)
                && ferMesh3EdgeTriCheckCommon(e2, e3);
}




fer_mesh3_face_t *ferMesh3FaceNew(void)
{
    fer_mesh3_face_t *f;
    f = FER_ALLOC(fer_mesh3_face_t);
    return f;
}

void ferMesh3FaceDel(fer_mesh3_face_t *f)
{
    free(f);
}



fer_mesh3_t *ferMesh3New(void)
{
    fer_mesh3_t *m;
    m = FER_ALLOC(fer_mesh3_t);

    ferListInit(&m->verts);
    m->verts_len = 0;
    ferListInit(&m->edges);
    m->edges_len = 0;
    ferListInit(&m->faces);
    m->faces_len = 0;

    return m;
}

void ferMesh3Del(fer_mesh3_t *m)
{
    ferMesh3Del2(m, NULL, NULL, NULL, NULL, NULL, NULL);
}

void ferMesh3Del2(fer_mesh3_t *m,
                  void (*delvertex)(fer_mesh3_vertex_t *, void *), void *vdata,
                  void (*deledge)(fer_mesh3_edge_t *, void *), void *edata,
                  void (*delface)(fer_mesh3_face_t *, void *), void *fdata)
{
    fer_mesh3_vertex_t *v;
    fer_mesh3_edge_t *e;
    fer_mesh3_face_t *f;
    fer_list_t *item;

    // disconnect all faces
    while (!ferListEmpty(&m->faces)){
        item = ferListNext(&m->faces);
        f = ferListEntry(item, fer_mesh3_face_t, list);
        ferMesh3RemoveFace(m, f);

        if (delface){
            delface(f, fdata);
        }
    }

    // disconnect all edges
    while (!ferListEmpty(&m->edges)){
        item = ferListNext(&m->edges);
        e = ferListEntry(item, fer_mesh3_edge_t, list);
        ferMesh3RemoveEdge(m, e);

        if (deledge){
            deledge(e, edata);
        }
    }

    // disconnect all vertices
    while (!ferListEmpty(&m->verts)){
        item = ferListNext(&m->verts);
        v = ferListEntry(item, fer_mesh3_vertex_t, list);
        ferMesh3RemoveVertex(m, v);

        if (delvertex){
            delvertex(v, vdata);
        }
    }

    free(m);
}

void ferMesh3AddVertex(fer_mesh3_t *m, fer_mesh3_vertex_t *v)
{
    ferListAppend(&m->verts, &v->list);
    m->verts_len++;

    ferListInit(&v->edges);
    v->edges_len = 0;
}

int ferMesh3RemoveVertex(fer_mesh3_t *m, fer_mesh3_vertex_t *v)
{
    if (!ferListEmpty(&v->edges))
        return -1;

    ferListDel(&v->list);
    m->verts_len--;
    return 0;
}

void ferMesh3AddEdge(fer_mesh3_t *m, fer_mesh3_edge_t *e,
                     fer_mesh3_vertex_t *start, fer_mesh3_vertex_t *end)
{
    // assign start and end point
    e->v[0] = start;
    e->v[1] = end;

    // append edge to list of edges in vertices
    e->vlist[0].mark = 0;
    ferListAppend(&start->edges, ferListMAsList(&e->vlist[0]));
    start->edges_len++;
    e->vlist[1].mark = 1;
    ferListAppend(&end->edges, ferListMAsList(&e->vlist[1]));
    end->edges_len++;

    // add edge to list of all edges
    ferListAppend(&m->edges, &e->list);
    m->edges_len++;

    // initialize incidenting faces
    e->f[0] = e->f[1] = NULL;
}

int ferMesh3RemoveEdge(fer_mesh3_t *m, fer_mesh3_edge_t *e)
{
    if (e->f[0] != NULL || e->f[1] != NULL)
        return -1;

    // remove edge from lists in vertices
    ferListDel(ferListMAsList(&e->vlist[0]));
    e->v[0]->edges_len--;
    ferListDel(ferListMAsList(&e->vlist[1]));
    e->v[1]->edges_len--;

    // remove edge from list of all edges
    ferListDel(&e->list);
    m->edges_len--;

    // reset .v[]
    e->v[0] = e->v[1] = NULL;

    return 0;
}

int ferMesh3AddFace(fer_mesh3_t *m, fer_mesh3_face_t *f,
                    fer_mesh3_edge_t *e1, fer_mesh3_edge_t *e2,
                    fer_mesh3_edge_t *e3)
{
    if (ferMesh3EdgeFacesLen(e1) == 2
            || ferMesh3EdgeFacesLen(e2) == 2
            || ferMesh3EdgeFacesLen(e3) == 2)
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
    ferListAppend(&m->faces, &f->list);
    m->faces_len++;

    return 0;
}

void ferMesh3RemoveFace(fer_mesh3_t *m, fer_mesh3_face_t *f)
{
    size_t i;

    // disconnect face from edges
    for (i = 0; i < 3; i++){
        if (f->e[i]->f[0] == f)
            f->e[i]->f[0] = f->e[i]->f[1];
        f->e[i]->f[1] = NULL;
    }

    // disconnect face from list of all faces
    ferListDel(&f->list);
    m->faces_len--;

    // zeroize pointers to edges
    f->e[0] = f->e[1] = f->e[2] = NULL;
}

void ferMesh3DumpSVT(fer_mesh3_t *m, FILE *out, const char *name)
{
    fer_list_t *item;
    fer_mesh3_vertex_t *v;
    fer_mesh3_vertex_t *vs[3];
    fer_mesh3_edge_t *e;
    fer_mesh3_face_t *f;
    size_t i;

    fprintf(out, "--------\n");
    if (name){
        fprintf(out, "Name: %s\n", name);
    }

    fprintf(out, "Points:\n");
    i = 0;
    ferListForEach(&m->verts, item){
        v = ferListEntry(item, fer_mesh3_vertex_t, list);
        v->_id = i++;
        fprintf(out, "%g %g %g\n",
                ferVec3X(ferMesh3VertexCoords(v)),
                ferVec3Y(ferMesh3VertexCoords(v)),
                ferVec3Z(ferMesh3VertexCoords(v)));
    }

    fprintf(out, "Edges:\n");
    ferListForEach(&m->edges, item){
        e = ferListEntry(item, fer_mesh3_edge_t, list);
        fprintf(out, "%d %d\n", ferMesh3EdgeVertex(e, 0)->_id,
                                ferMesh3EdgeVertex(e, 1)->_id);
    }

    fprintf(out, "Faces:\n");
    ferListForEach(&m->faces, item){
        f = ferListEntry(item, fer_mesh3_face_t, list);
        ferMesh3FaceVertices(f, vs);
        fprintf(out, "%d %d %d\n", vs[0]->_id, vs[1]->_id, vs[2]->_id);
    }

    fprintf(out, "--------\n");
}

void ferMesh3DumpTriangles(fer_mesh3_t *m, FILE *out)
{
    fer_mesh3_vertex_t *vs[3];
    fer_mesh3_face_t *f;
    fer_list_t *item;

    ferListForEach(&m->faces, item){
        f = ferListEntry(item, fer_mesh3_face_t, list);
        ferMesh3FaceVertices(f, vs);

        fprintf(out, "%g %g %g %g %g %g %g %g %g\n",
                ferVec3X(ferMesh3VertexCoords(vs[0])),
                ferVec3Y(ferMesh3VertexCoords(vs[0])),
                ferVec3Z(ferMesh3VertexCoords(vs[0])),
                ferVec3X(ferMesh3VertexCoords(vs[1])),
                ferVec3Y(ferMesh3VertexCoords(vs[1])),
                ferVec3Z(ferMesh3VertexCoords(vs[1])),
                ferVec3X(ferMesh3VertexCoords(vs[2])),
                ferVec3Y(ferMesh3VertexCoords(vs[2])),
                ferVec3Z(ferMesh3VertexCoords(vs[2])));
    }

    fflush(out);
}

void ferMesh3DumpPovray(fer_mesh3_t *m, FILE *out)
{
    fer_mesh3_vertex_t *vs[3];
    fer_mesh3_face_t *f;
    fer_list_t *item;

    fprintf(out, "mesh {\n");

    ferListForEach(&m->faces, item){
        f = ferListEntry(item, fer_mesh3_face_t, list);
        ferMesh3FaceVertices(f, vs);

        fprintf(out, "triangle { <%g %g %g>, <%g %g %g>, <%g %g %g> }\n",
                ferVec3X(ferMesh3VertexCoords(vs[0])),
                ferVec3Y(ferMesh3VertexCoords(vs[0])),
                ferVec3Z(ferMesh3VertexCoords(vs[0])),
                ferVec3X(ferMesh3VertexCoords(vs[1])),
                ferVec3Y(ferMesh3VertexCoords(vs[1])),
                ferVec3Z(ferMesh3VertexCoords(vs[1])),
                ferVec3X(ferMesh3VertexCoords(vs[2])),
                ferVec3Y(ferMesh3VertexCoords(vs[2])),
                ferVec3Z(ferMesh3VertexCoords(vs[2])));
    }

    fprintf(out, "}\n");

    fflush(out);
}
