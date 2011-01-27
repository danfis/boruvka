#include <fermat/mesh/mesh3.h>
#include <fermat/alloc.h>

fer_mesh3_vertex_t *ferMesh3VertexNew(fer_real_t x, fer_real_t y, fer_real_t z)
{
    fer_mesh3_vertex_t *v;
    v = FER_ALLOC(fer_mesh3_vertex_t);
    ferMesh3VertexInit(v, x, y, z);
    return v;
}

fer_mesh3_vertex_t *ferMesh3VertexNew2(const fer_vec3_t *coords)
{
    fer_mesh3_vertex_t *v;
    v = FER_ALLOC(fer_mesh3_vertex_t);
    ferMesh3VertexInit2(v, coords);
    return v;
}

void ferMesh3VertexDel(fer_mesh3_vertex_t *v)
{
    ferMesh3VertexDestroy(v);
    free(v);
}

void ferMesh3VertexInit(fer_mesh3_vertex_t *v,
                        fer_real_t x, fer_real_t y, fer_real_t z)
{
    v->v = ferVec3New(x, y, z);

    ferListInit(&v->list);
    ferListInit(&v->edges);
    v->edges_len = 0;
}

void ferMesh3VertexInit2(fer_mesh3_vertex_t *v, const fer_vec3_t *coords)
{
    v->v = ferVec3Clone(coords);

    ferListInit(&v->list);
    ferListInit(&v->edges);
    v->edges_len = 0;
}

void ferMesh3VertexDestroy(fer_mesh3_vertex_t *v)
{
    if (v->v){
        ferVec3Del(v->v);
        v->v = NULL;
    }

    /*
    // TODO: do this?
    ferListDel(&v->list);
    while (!ferListEmpty(&v->edges)){
        ferListDel(ferListNext(&v->edges));
    }
    v->edges_len = 0;
    */
}




fer_mesh3_edge_t *ferMesh3EdgeNew(void)
{
    fer_mesh3_edge_t *e;
    e = FER_ALLOC(fer_mesh3_edge_t);
    ferMesh3EdgeInit(e);
    return e;
}

void ferMesh3EdgeDel(fer_mesh3_edge_t *e)
{
    ferMesh3EdgeDestroy(e);
    free(e);
}

void ferMesh3EdgeInit(fer_mesh3_edge_t *e)
{
    e->v[0] = e->v[1] = NULL;
    e->f[0] = e->f[1] = NULL;

    ferListInit(&e->list);
    ferListInit(&e->vlist[0]);
    ferListInit(&e->vlist[1]);
}

void ferMesh3EdgeDestroy(fer_mesh3_edge_t *e)
{
    e->v[0] = e->v[1] = NULL;
    e->f[0] = e->f[1] = NULL;

    // TODO: Disconnect from vertex and face and list?
}

int ferMesh3EdgeTriCheck(const fer_mesh3_edge_t *e1,
                         const fer_mesh3_edge_t *e2,
                         const fer_mesh3_edge_t *e3)
{
    // TODO
    return 0;
}




fer_mesh3_face_t *ferMesh3FaceNew(void)
{
    fer_mesh3_face_t *f;
    f = FER_ALLOC(fer_mesh3_face_t);
    ferMesh3FaceInit(f);
    return f;
}

void ferMesh3FaceDel(fer_mesh3_face_t *f)
{
    ferMesh3FaceDestroy(f);
    free(f);
}

void ferMesh3FaceInit(fer_mesh3_face_t *f)
{
    f->e[0] = f->e[1] = f->e[2] = NULL;
    ferListInit(&f->list);
}

void ferMesh3FaceDestroy(fer_mesh3_face_t *f)
{
    f->e[0] = f->e[1] = f->e[2] = NULL;

    // TODO: Disconnect from list?
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
            delvertex(v, edata);
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
    ferListAppend(&start->edges, &e->vlist[0]);
    start->edges_len++;
    ferListAppend(&end->edges, &e->vlist[1]);
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
    ferListDel(&e->vlist[0]);
    e->v[0]->edges_len--;
    ferListDel(&e->vlist[1]);
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
