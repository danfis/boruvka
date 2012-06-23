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

#include <boruvka/chull3.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>
#include <boruvka/predicates.h>

struct _bor_chull3_vert_t {
    bor_vec3_t v;
    bor_mesh3_vertex_t m;
    bor_list_t list;
    int border_edges;
} bor_aligned(16) bor_packed;
typedef struct _bor_chull3_vert_t bor_chull3_vert_t;

struct _bor_chull3_edge_t {
    bor_mesh3_edge_t m;
    bor_list_t list;
    int swap;
    unsigned int onedge;
};
typedef struct _bor_chull3_edge_t bor_chull3_edge_t;

struct _bor_chull3_face_t {
    bor_mesh3_face_t m;
    bor_chull3_vert_t *v[3];
    bor_list_t list;
};
typedef struct _bor_chull3_face_t bor_chull3_face_t;

/** Predicate that returns true if f is visible from v */
_bor_inline int isVisible(const bor_chull3_t *h,
                          const bor_vec3_t *v,
                          const bor_chull3_face_t *f);
_bor_inline int isVisible2(const bor_chull3_t *h,
                           const bor_vec3_t *v,
                           const bor_vec3_t *f1, const bor_vec3_t *f2,
                           const bor_vec3_t *f3);
_bor_inline bor_real_t orient3d(const bor_chull3_t *h,
                                const bor_vec3_t *v,
                                const bor_chull3_face_t *f);
_bor_inline bor_real_t orient3d2(const bor_chull3_t *h,
                                 const bor_vec3_t *v,
                                 const bor_vec3_t *f1, const bor_vec3_t *f2,
                                 const bor_vec3_t *f3);
/** Predicate that returns true if f is coplanar with v */
_bor_inline int isCoplanar(const bor_chull3_t *h,
                           const bor_vec3_t *v,
                           const bor_chull3_face_t *f);

/** Create new vertex */
static bor_chull3_vert_t *vertNew(bor_chull3_t *h, const bor_vec3_t *v);
/** Delete vertex */
static void vertDel(bor_chull3_t *h, bor_chull3_vert_t *v);
static void vertDelMesh(bor_mesh3_vertex_t *, void *);

/** Create new edge */
static bor_chull3_edge_t *edgeNew(bor_chull3_t *h,
                                  bor_chull3_vert_t *v1, bor_chull3_vert_t *v2);
/** Delete edge */
static void edgeDel(bor_chull3_t *h, bor_chull3_edge_t *e);
static void edgeDelMesh(bor_mesh3_edge_t *, void *);
/** Fills v[] with vertices incidenting with edge */
static void edgeVertices(bor_chull3_edge_t *e, bor_chull3_vert_t **v);

/** Create new face */
static bor_chull3_face_t *faceNew(bor_chull3_t *h,
                                  bor_chull3_edge_t *e1, bor_chull3_edge_t *e2,
                                  bor_chull3_edge_t *e3);
/** Delete face */
static void faceDel(bor_chull3_t *h, bor_chull3_face_t *f);
static void faceDelMesh(bor_mesh3_face_t *, void *);
/** Set triplet of bounding vertices in specified order */
static void faceSetVertices(bor_chull3_t *h, bor_chull3_face_t *f,
                            bor_chull3_vert_t *v1, bor_chull3_vert_t *v2,
                            bor_chull3_vert_t *v3);
/** Fills e[] with edges incidenting with face */
static void faceEdges(bor_chull3_face_t *f, bor_chull3_edge_t **e);


/** Returns true if hull and given point are coplanar */
static int isCoplanarWithHull(bor_chull3_t *h, const bor_vec3_t *v);
/** Returns true if given point lies on current coplanar patch (hull) */
static int liesOnCoplanar(bor_chull3_t *h, const bor_vec3_t *v);
/** Adds coplanar point */
static void addCoplanar(bor_chull3_t *h, const bor_vec3_t *v);
/** Adds first non-coplanar point on hull */
static void firstNonCoplanar(bor_chull3_t *h, const bor_vec3_t *v);
/** Obtain border edges */
static void findBorderEdges(bor_chull3_t *h, const bor_vec3_t *v,
                            bor_list_t *edges);
/** Makes new cone connecting new point {v} with border edges */
static void makeCone(bor_chull3_t *h, bor_list_t *edges, const bor_vec3_t *v);

bor_chull3_t *borCHull3New(void)
{
    bor_chull3_t *h;

    h = BOR_ALLOC(bor_chull3_t);
    h->mesh = borMesh3New();
    h->coplanar = 1;

    //borPredInit(&h->pred);

    return h;
}


void borCHull3Del(bor_chull3_t *h)
{
    borMesh3Del2(h->mesh,
                 vertDelMesh, NULL,
                 edgeDelMesh, NULL,
                 faceDelMesh, NULL);

    BOR_FREE(h);
}

void borCHull3Add(bor_chull3_t *h, const bor_vec3_t *point)
{
    bor_list_t border_edges;

    if (h->coplanar && isCoplanarWithHull(h, point)){
        if (!liesOnCoplanar(h, point))
            addCoplanar(h, point);
        return;
    }else if (h->coplanar){
        firstNonCoplanar(h, point);
        h->coplanar = 0;
        return;
    }

    borListInit(&border_edges);
    findBorderEdges(h, point, &border_edges);
    if (borListEmpty(&border_edges))
        return;


    makeCone(h, &border_edges, point);
}

void borCHull3DumpSVT(bor_chull3_t *h, FILE *out, const char *name)
{
    borMesh3DumpSVT(h->mesh, out, name);
}


_bor_inline int isVisible(const bor_chull3_t *h,
                          const bor_vec3_t *v,
                          const bor_chull3_face_t *f)
{
    bor_real_t orient;
    orient = orient3d(h, v, f);
    return orient > BOR_ZERO;
}

_bor_inline int isVisible2(const bor_chull3_t *h,
                           const bor_vec3_t *v,
                           const bor_vec3_t *f1, const bor_vec3_t *f2,
                           const bor_vec3_t *f3)
{
    bor_real_t orient;
    orient = orient3d2(h, v, f1, f2, f3);
    return orient > BOR_ZERO;
}


_bor_inline bor_real_t orient3d(const bor_chull3_t *h,
                                const bor_vec3_t *v,
                                const bor_chull3_face_t *f)
{
    return borVec3Volume6(&f->v[0]->v, &f->v[1]->v, &f->v[2]->v, v);
    //return borPredOrient3d(&h->pred, &f->v[0]->v, &f->v[1]->v, &f->v[2]->v, v);
}

_bor_inline bor_real_t orient3d2(const bor_chull3_t *h,
                                 const bor_vec3_t *v,
                                 const bor_vec3_t *f1, const bor_vec3_t *f2,
                                 const bor_vec3_t *f3)
{
    return borVec3Volume6(f1, f2, f3, v);
    //return borPredOrient3d(&h->pred, f1, f2, f3, v);
}

_bor_inline int isCoplanar(const bor_chull3_t *h,
                           const bor_vec3_t *v,
                           const bor_chull3_face_t *f)
{
    bor_real_t orient;
    orient = orient3d(h, v, f);

    return borIsZero(orient);
}


static bor_chull3_vert_t *vertNew(bor_chull3_t *h, const bor_vec3_t *v)
{
    bor_chull3_vert_t *vert;

    vert = BOR_ALLOC_ALIGN(bor_chull3_vert_t, 16);

    // copy coordinates
    borVec3Copy(&vert->v, v);

    // init mesh structure
    borMesh3VertexSetCoords(&vert->m, &vert->v);

    // add vertex to mesh
    borMesh3AddVertex(h->mesh, &vert->m);

    vert->border_edges = 0;

    return vert;
}

static void vertDel(bor_chull3_t *h, bor_chull3_vert_t *v)
{
    borMesh3RemoveVertex(h->mesh, &v->m);
    BOR_FREE(v);
}
static void vertDelMesh(bor_mesh3_vertex_t *_v, void *_)
{
    bor_chull3_vert_t *v;

    v = bor_container_of(_v, bor_chull3_vert_t, m);
    BOR_FREE(v);
}


static bor_chull3_edge_t *edgeNew(bor_chull3_t *h,
                                  bor_chull3_vert_t *v1, bor_chull3_vert_t *v2)
{
    bor_chull3_edge_t *e;

    e = BOR_ALLOC(bor_chull3_edge_t);
    borMesh3AddEdge(h->mesh, &e->m, &v1->m, &v2->m);
    e->swap = 0;
    e->onedge = 0;

    return e;
}

static void edgeDel(bor_chull3_t *h, bor_chull3_edge_t *e)
{
    borMesh3RemoveEdge(h->mesh, &e->m);
    BOR_FREE(e);
}

static void edgeDelMesh(bor_mesh3_edge_t *_e, void *_)
{
    bor_chull3_edge_t *e;

    e = bor_container_of(_e, bor_chull3_edge_t, m);
    BOR_FREE(e);
}

static void edgeVertices(bor_chull3_edge_t *e, bor_chull3_vert_t **v)
{
    bor_mesh3_vertex_t *mv;
    int i;

    for (i = 0; i < 2; i++){
        mv = borMesh3EdgeVertex(&e->m, i);
        v[i] = bor_container_of(mv, bor_chull3_vert_t, m);
    }
}



static bor_chull3_face_t *faceNew(bor_chull3_t *h,
                                  bor_chull3_edge_t *e1, bor_chull3_edge_t *e2,
                                  bor_chull3_edge_t *e3)
{
    bor_chull3_face_t *f;

    f = BOR_ALLOC(bor_chull3_face_t);
    if (borMesh3AddFace(h->mesh, &f->m, &e1->m, &e2->m, &e3->m) != 0){
        DBG("Can't add face, %d", (int)borMesh3VerticesLen(h->mesh));
        // borCHull3DumpSVT(h, stdout, "Can't face");
    }

    f->v[0] = f->v[1] = f->v[2] = NULL;

    return f;
}

static void faceDel(bor_chull3_t *h, bor_chull3_face_t *f)
{
    borMesh3RemoveFace(h->mesh, &f->m);
    BOR_FREE(f);
}

static void faceDelMesh(bor_mesh3_face_t *_f, void *_)
{
    bor_chull3_face_t *f;

    f = bor_container_of(_f, bor_chull3_face_t, m);
    BOR_FREE(f);
}

static void faceSetVertices(bor_chull3_t *h, bor_chull3_face_t *f,
                            bor_chull3_vert_t *v1, bor_chull3_vert_t *v2,
                            bor_chull3_vert_t *v3)
{
    f->v[0] = v1;
    f->v[1] = v2;
    f->v[2] = v3;
}

static void faceEdges(bor_chull3_face_t *f, bor_chull3_edge_t **e)
{
    bor_mesh3_edge_t *me;
    int i;

    for (i = 0; i < 3; i++){
        me = borMesh3FaceEdge(&f->m, i);
        e[i] = bor_container_of(me, bor_chull3_edge_t, m);
    }
}


static int isCoplanarWithHull(bor_chull3_t *h, const bor_vec3_t *v)
{
    bor_list_t *list, *item;
    bor_mesh3_face_t *mf;
    bor_chull3_face_t *f;

    if (borMesh3VerticesLen(h->mesh) < 3)
        return 1;

    list = borMesh3Faces(h->mesh);
    BOR_LIST_FOR_EACH(list, item){
        mf = BOR_LIST_ENTRY(item, bor_mesh3_face_t, list);
        f  = bor_container_of(mf, bor_chull3_face_t, m);

        if (!isCoplanar(h, v, f))
            return 0;
    }

    return 1;
}

static int liesOnCoplanar(bor_chull3_t *h, const bor_vec3_t *v)
{
    bor_list_t *list, *item;
    bor_mesh3_face_t *mf;
    bor_chull3_face_t *f;

    if (borMesh3VerticesLen(h->mesh) < 3)
        return 0;

    list = borMesh3Faces(h->mesh);
    BOR_LIST_FOR_EACH(list, item){
        mf = BOR_LIST_ENTRY(item, bor_mesh3_face_t, list);
        f  = bor_container_of(mf, bor_chull3_face_t, m);

        if (!borVec3PointInTri(v, &f->v[0]->v, &f->v[1]->v, &f->v[2]->v))
            return 0;
    }

    return 1;
}

static void addCoplanarGrow(bor_chull3_t *h, bor_chull3_vert_t *v,
                            bor_chull3_edge_t *e, const bor_vec3_t *topv,
                            int inv)
{
    bor_list_t *list, *item;
    bor_mesh3_vertex_t *mv1, *mv2;
    bor_mesh3_edge_t *me;
    bor_chull3_vert_t *v2, *v3;
    bor_chull3_edge_t *e2, *e3;
    bor_chull3_face_t *f;

    while (e){
        mv1 = borMesh3EdgeOtherVertex(&e->m, &v->m);
        list = borMesh3VertexEdges(mv1);
        BOR_LIST_FOR_EACH(list, item){
            me = borMesh3EdgeFromVertexList(item);

            if (me != &e->m && borMesh3EdgeFacesLen(me) == 1){
                mv2 = borMesh3EdgeOtherVertex(me, mv1);

                if (isVisible2(h, topv, &v->v, mv1->v, mv2->v)){
                    v2 = bor_container_of(mv1, bor_chull3_vert_t, m);
                    v3 = bor_container_of(mv2, bor_chull3_vert_t, m);
                    e2 = bor_container_of(me, bor_chull3_edge_t, m);

                    e3 = edgeNew(h, v, v3);

                    f = faceNew(h, e, e2, e3);

                    // orient face
                    if (inv){
                        faceSetVertices(h, f, v, v3, v2);
                    }else{
                        faceSetVertices(h, f, v, v2, v3);
                    }

                    e = e3;
                }else{
                    e = NULL;
                }

                break;
            }
        }
    }
}

static void addCoplanar3(bor_chull3_t *h, const bor_vec3_t *v)
{
    bor_list_t *list, *item;
    bor_mesh3_vertex_t *mv;
    bor_chull3_vert_t *v1, *v2, *vert;
    bor_chull3_edge_t *e1, *e2, *e3;
    bor_chull3_face_t *f;
    bor_real_t dist1, dist2, dist3;

    // first check if new vertex isn't collinear with the two already
    // in mesh
    list = borMesh3Vertices(h->mesh);
    item = borListNext(list);
    mv   = BOR_LIST_ENTRY(item, bor_mesh3_vertex_t, list);
    v1   = bor_container_of(mv, bor_chull3_vert_t, m);
    item = borListPrev(list);
    mv   = BOR_LIST_ENTRY(item, bor_mesh3_vertex_t, list);
    v2   = bor_container_of(mv, bor_chull3_vert_t, m);

    if (borVec3Collinear(v, &v1->v, &v2->v)){
        // vertices are collinear, find out pair with longest distance
        // and keep that pair
        dist1 = borVec3Dist2(v, &v1->v);
        dist2 = borVec3Dist2(v, &v2->v);
        dist3 = borVec3Dist2(&v1->v, &v2->v);
        if (dist1 > dist2){
            if (dist1 > dist3){
                vertDel(h, v2);
                vertNew(h, v);
            }
        }else if (dist2 > dist3){
            vertDel(h, v1);
            vertNew(h, v);
        }
    }else{
        // vertices aren't collinear, we can create double triangle
        // 1. create new vertex
        vert = vertNew(h, v);

        // 2. create edges connecting vertices
        e1 = edgeNew(h, v1, v2);
        e2 = edgeNew(h, vert, v1);
        e3 = edgeNew(h, vert, v2);

        // 3. create face
        f = faceNew(h, e1, e2, e3);
        faceSetVertices(h, f, vert, v1, v2);
    }
}

static void addCoplanar4(bor_chull3_t *h, const bor_vec3_t *point)
{
    bor_list_t *list, *item;
    bor_chull3_vert_t *v[4];
    bor_chull3_edge_t *e[3];
    bor_chull3_face_t *f, *f2;
    bor_mesh3_face_t *mf;
    bor_mesh3_edge_t *me;
    bor_real_t dist1, dist2;
    bor_vec3_t cross, p01, p02, topv;
    int inv;


    // we already have at least triangle
    // find nearest edge and make triangle

    // find nearest edge to connect triangle
    e[0] = NULL;
    v[0] = v[1] = NULL;
    dist2 = BOR_REAL_MAX;
    list = borMesh3Edges(h->mesh);
    BOR_LIST_FOR_EACH(list, item){
        me = BOR_LIST_ENTRY(item, bor_mesh3_edge_t, list);
        if (borMesh3EdgeFacesLen(me) != 1)
            continue;

        e[1] = bor_container_of(me, bor_chull3_edge_t, m);
        edgeVertices(e[1], v + 2);

        dist1 = borVec3PointSegmentDist2(point, &v[2]->v, &v[3]->v, NULL);
        if (borEq(dist1, dist2)){
            if (borVec3TriArea2(point, &v[2]->v, &v[3]->v)
                    > borVec3TriArea2(point, &v[0]->v, &v[1]->v)){
                dist2 = dist1;
                e[0] = e[1];
                v[0] = v[2];
                v[1] = v[3];
            }
        }else if (dist1 < dist2){
            dist2 = dist1;
            e[0] = e[1];
            v[0] = v[2];
            v[1] = v[3];
        }
    }

    // 1. check area of created triangle
    if (borIsZero(borVec3TriArea2(point, &v[0]->v, &v[1]->v))){
        return;
    }

    // 2. create new vertex
    v[2] = vertNew(h, point);

    // 3. create edges connecting vertices
    e[1] = edgeNew(h, v[2], v[0]);
    e[2] = edgeNew(h, v[2], v[1]);

    // 4. create face
    f = faceNew(h, e[0], e[1], e[2]);

    // 5. get other face that newly created one
    mf = borMesh3EdgeOtherFace(&e[0]->m, &f->m);
    f2 = bor_container_of(mf, bor_chull3_face_t, m);

    // 6. orient vertices in new face
    faceSetVertices(h, f, v[2], v[0], v[1]);
    inv = 1;
    if ((f2->v[0] == v[0] && f2->v[1] == v[1])
            || (f2->v[1] == v[0] && f2->v[2] == v[1])
            || (f2->v[2] == v[0] && f2->v[0] == v[1])){
        faceSetVertices(h, f, v[2], v[1], v[0]);
        inv = 0;
    }

    borVec3Sub2(&p01, &v[0]->v, &v[2]->v);
    borVec3Sub2(&p02, &v[1]->v, &v[2]->v);
    borVec3Cross(&cross, &p01, &p02);
    borVec3Normalize(&cross);

    borVec3Add2(&topv, &v[2]->v, &cross);
    addCoplanarGrow(h, v[2], e[1], &topv, inv);

    borVec3Scale(&cross, -BOR_ONE);
    borVec3Add2(&topv, &v[2]->v, &cross);
    addCoplanarGrow(h, v[2], e[2], &topv, !inv);
}


static void addCoplanar(bor_chull3_t *h, const bor_vec3_t *v)
{
    if (borMesh3VerticesLen(h->mesh) < 2){
        vertNew(h, v);
    }else if (borMesh3VerticesLen(h->mesh) < 3){
        addCoplanar3(h, v);
    }else{
        addCoplanar4(h, v);
    }
}

static void firstNonCoplanar(bor_chull3_t *h, const bor_vec3_t *point)
{
    bor_list_t *list, *item;
    bor_list_t border_edges;
    bor_mesh3_face_t *mf;
    bor_chull3_face_t *f;
    bor_chull3_edge_t *e[3];
    bor_chull3_vert_t *v[2];
    int reorient, i;

    // first find out if we need to reorient faces
    reorient = 0;
    list = borMesh3Faces(h->mesh);
    BOR_LIST_FOR_EACH(list, item){
        mf = BOR_LIST_ENTRY(item, bor_mesh3_face_t, list);
        f  = bor_container_of(mf, bor_chull3_face_t, m);

        if (isVisible(h, point, f)){
            reorient = 1;
            break;
        }
    }

    if (reorient){
        list = borMesh3Faces(h->mesh);
        BOR_LIST_FOR_EACH(list, item){
            mf = BOR_LIST_ENTRY(item, bor_mesh3_face_t, list);
            f  = bor_container_of(mf, bor_chull3_face_t, m);

            faceSetVertices(h, f, f->v[0], f->v[2], f->v[1]);
        }
    }

    borListInit(&border_edges);
    list = borMesh3Faces(h->mesh);
    BOR_LIST_FOR_EACH(list, item){
        mf = BOR_LIST_ENTRY(item, bor_mesh3_face_t, list);
        f  = bor_container_of(mf, bor_chull3_face_t, m);

        // obtain border edges
        faceEdges(f, e);

        // append edges that incidents with non-visible face to list of
        // border faces
        for (i = 0; i < 3; i++){
            if (borMesh3EdgeFacesLen(&e[i]->m) == 1){
                // orient edge correctly
                edgeVertices(e[i], v);

                e[i]->swap = 0;
                if ((v[0] == f->v[0] && v[1] == f->v[1])
                        || (v[0] == f->v[1] && v[1] == f->v[2])
                        || (v[0] == f->v[2] && v[1] == f->v[0])){
                    e[i]->swap = 1;
                }

                // append it to list of edges
                borListAppend(&border_edges, &e[i]->list);
            }
        }
    }

    makeCone(h, &border_edges, point);
}

static void updateBorderEdges(bor_chull3_t *h, bor_chull3_face_t *f,
                              bor_list_t *edges, bor_list_t *wrong_vertices)
{
    bor_chull3_edge_t *e[3];
    bor_chull3_vert_t *v[2];
    int i;

    faceEdges(f, e);

    // add edges to list of border edges
    for (i = 0; i < 3; i++){
        if (!e[i]->onedge){
            edgeVertices(e[i], v);

            e[i]->onedge = 1;
            borListAppend(edges, &e[i]->list);

            // set hint for edge orientation
            e[i]->swap = 0;
            if ((v[1] == f->v[0] && v[0] == f->v[1])
                    || (v[1] == f->v[1] && v[0] == f->v[2])
                    || (v[1] == f->v[2] && v[0] == f->v[0])){
                e[i]->swap = 1;
            }

            // increase counter for detecting vertices where is created
            // incorrect patch
            v[0]->border_edges++;
            v[1]->border_edges++;

            if (v[0]->border_edges == 3)
                borListAppend(wrong_vertices, &v[0]->list);
            if (v[1]->border_edges == 3)
                borListAppend(wrong_vertices, &v[1]->list);
        }
    }

    // delete face
    faceDel(h, f);

    // delete edges and vertices that were left alone 
    for (i = 0; i < 3; i++){
        if (borMesh3EdgeFacesLen(&e[i]->m) == 0){
            edgeVertices(e[i], v);

            borListDel(&e[i]->list);
            edgeDel(h, e[i]);

            v[0]->border_edges--;
            v[1]->border_edges--;
            if (v[0]->border_edges == 2)
                borListDel(&v[0]->list);
            if (v[1]->border_edges == 2)
                borListDel(&v[1]->list);

            if (borMesh3VertexEdgesLen(&v[0]->m) == 0)
                vertDel(h, v[0]);
            if (borMesh3VertexEdgesLen(&v[1]->m) == 0)
                vertDel(h, v[1]);
        }
    }
}

static void correctBorderEdges(bor_chull3_t *h, const bor_vec3_t *point,
                               bor_chull3_vert_t *v,
                               bor_list_t *edges,
                               bor_list_t *wrong_vertices)
{
    bor_list_t *list, *item;
    bor_mesh3_edge_t *me;
    bor_mesh3_face_t *mf;
    bor_real_t orient, best_orient;
    bor_chull3_face_t *f, *best_f;
    int i;

    best_orient = -BOR_REAL_MAX;
    best_f = NULL;

    list = borMesh3VertexEdges(&v->m);
    BOR_LIST_FOR_EACH(list, item){
        me = borMesh3EdgeFromVertexList(item);

        for (i = 0; i < 2; i++){
            mf = borMesh3EdgeFace(me, i);
            if (mf && (!best_f || &best_f->m != mf)){
                f = bor_container_of(mf, bor_chull3_face_t, m);

                orient = orient3d(h, point, f);
                if (orient > best_orient){
                    best_orient = orient;
                    best_f = f;
                }
            }
        }
    }

    updateBorderEdges(h, best_f, edges, wrong_vertices);
}

static void findBorderEdges(bor_chull3_t *h, const bor_vec3_t *point,
                            bor_list_t *edges)
{
    bor_list_t *list, *item, *itemtmp, wrong_vertices, faces;
    bor_mesh3_face_t *mf;
    bor_chull3_face_t *f;
    bor_chull3_vert_t *v;

    borListInit(&wrong_vertices);
    borListInit(&faces);

    list = borMesh3Faces(h->mesh);
    BOR_LIST_FOR_EACH_SAFE(list, item, itemtmp){
        mf = BOR_LIST_ENTRY(item, bor_mesh3_face_t, list);
        f  = bor_container_of(mf, bor_chull3_face_t, m);

        if (isVisible(h, point, f)){
            borListAppend(&faces, &f->list);
        }else{
            // skip this point if it is duplicate with any vertex
            if (borVec3Eq(point, &f->v[0]->v)
                    || borVec3Eq(point, &f->v[1]->v)
                    || borVec3Eq(point, &f->v[2]->v)){
                borListInit(&faces);
                break;
            }
        }
    }

    while (!borListEmpty(&faces)){
        item = borListNext(&faces);
        borListDel(item);
        f = BOR_LIST_ENTRY(item, bor_chull3_face_t, list);

        updateBorderEdges(h, f, edges, &wrong_vertices);
    }

    while (!borListEmpty(&wrong_vertices)){
        item = borListNext(&wrong_vertices);
        v = BOR_LIST_ENTRY(item, bor_chull3_vert_t, list);

        correctBorderEdges(h, point, v, edges, &wrong_vertices);
    }
}


static void makeCone(bor_chull3_t *h, bor_list_t *edges, const bor_vec3_t *point)
{
    bor_chull3_edge_t *e1, *e2, *e3;
    bor_chull3_vert_t *v[3];
    bor_chull3_face_t *f;
    bor_mesh3_edge_t *me;
    bor_list_t *item;

    v[0] = vertNew(h, point);

    BOR_LIST_FOR_EACH(edges, item){
        e1 = BOR_LIST_ENTRY(item, bor_chull3_edge_t, list);
        e1->onedge = 0;

        edgeVertices(e1, v + 1);
        v[1]->border_edges = 0;
        v[2]->border_edges = 0;

        me = borMesh3VertexCommonEdge(&v[0]->m, &v[1]->m);
        if (me == NULL){
            e2 = edgeNew(h, v[0], v[1]);
        }else{
            e2 = bor_container_of(me, bor_chull3_edge_t, m);
        }

        me = borMesh3VertexCommonEdge(&v[0]->m, &v[2]->m);
        if (me == NULL){
            e3 = edgeNew(h, v[0], v[2]);
        }else{
            e3 = bor_container_of(me, bor_chull3_edge_t, m);
        }

        // create new face
        f = faceNew(h, e1, e2, e3);

        // orient correctly vertices
        if (e1->swap){
            faceSetVertices(h, f, v[0], v[2], v[1]);
        }else{
            faceSetVertices(h, f, v[0], v[1], v[2]);
        }
    }
}
