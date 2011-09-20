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

#include <fermat/chull3.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>
#include <fermat/predicates.h>

struct _fer_chull3_vert_t {
    fer_vec3_t v;
    fer_mesh3_vertex_t m;
    fer_list_t list;
    int border_edges;
} fer_aligned(16) fer_packed;
typedef struct _fer_chull3_vert_t fer_chull3_vert_t;

struct _fer_chull3_edge_t {
    fer_mesh3_edge_t m;
    fer_list_t list;
    int swap;
    unsigned int onedge;
};
typedef struct _fer_chull3_edge_t fer_chull3_edge_t;

struct _fer_chull3_face_t {
    fer_mesh3_face_t m;
    fer_chull3_vert_t *v[3];
    fer_list_t list;
};
typedef struct _fer_chull3_face_t fer_chull3_face_t;

/** Predicate that returns true if f is visible from v */
_fer_inline int isVisible(const fer_chull3_t *h,
                          const fer_vec3_t *v,
                          const fer_chull3_face_t *f);
_fer_inline int isVisible2(const fer_chull3_t *h,
                           const fer_vec3_t *v,
                           const fer_vec3_t *f1, const fer_vec3_t *f2,
                           const fer_vec3_t *f3);
_fer_inline fer_real_t orient3d(const fer_chull3_t *h,
                                const fer_vec3_t *v,
                                const fer_chull3_face_t *f);
_fer_inline fer_real_t orient3d2(const fer_chull3_t *h,
                                 const fer_vec3_t *v,
                                 const fer_vec3_t *f1, const fer_vec3_t *f2,
                                 const fer_vec3_t *f3);
/** Predicate that returns true if f is coplanar with v */
_fer_inline int isCoplanar(const fer_chull3_t *h,
                           const fer_vec3_t *v,
                           const fer_chull3_face_t *f);

/** Create new vertex */
static fer_chull3_vert_t *vertNew(fer_chull3_t *h, const fer_vec3_t *v);
/** Delete vertex */
static void vertDel(fer_chull3_t *h, fer_chull3_vert_t *v);
static void vertDelMesh(fer_mesh3_vertex_t *, void *);

/** Create new edge */
static fer_chull3_edge_t *edgeNew(fer_chull3_t *h,
                                  fer_chull3_vert_t *v1, fer_chull3_vert_t *v2);
/** Delete edge */
static void edgeDel(fer_chull3_t *h, fer_chull3_edge_t *e);
static void edgeDelMesh(fer_mesh3_edge_t *, void *);
/** Fills v[] with vertices incidenting with edge */
static void edgeVertices(fer_chull3_edge_t *e, fer_chull3_vert_t **v);

/** Create new face */
static fer_chull3_face_t *faceNew(fer_chull3_t *h,
                                  fer_chull3_edge_t *e1, fer_chull3_edge_t *e2,
                                  fer_chull3_edge_t *e3);
/** Delete face */
static void faceDel(fer_chull3_t *h, fer_chull3_face_t *f);
static void faceDelMesh(fer_mesh3_face_t *, void *);
/** Set triplet of bounding vertices in specified order */
static void faceSetVertices(fer_chull3_t *h, fer_chull3_face_t *f,
                            fer_chull3_vert_t *v1, fer_chull3_vert_t *v2,
                            fer_chull3_vert_t *v3);
/** Fills e[] with edges incidenting with face */
static void faceEdges(fer_chull3_face_t *f, fer_chull3_edge_t **e);


/** Returns true if hull and given point are coplanar */
static int isCoplanarWithHull(fer_chull3_t *h, const fer_vec3_t *v);
/** Returns true if given point lies on current coplanar patch (hull) */
static int liesOnCoplanar(fer_chull3_t *h, const fer_vec3_t *v);
/** Adds coplanar point */
static void addCoplanar(fer_chull3_t *h, const fer_vec3_t *v);
/** Adds first non-coplanar point on hull */
static void firstNonCoplanar(fer_chull3_t *h, const fer_vec3_t *v);
/** Obtain border edges */
static void findBorderEdges(fer_chull3_t *h, const fer_vec3_t *v,
                            fer_list_t *edges);
/** Makes new cone connecting new point {v} with border edges */
static void makeCone(fer_chull3_t *h, fer_list_t *edges, const fer_vec3_t *v);

fer_chull3_t *ferCHull3New(void)
{
    fer_chull3_t *h;

    h = FER_ALLOC(fer_chull3_t);
    h->mesh = ferMesh3New();
    h->coplanar = 1;

    //ferPredInit(&h->pred);

    return h;
}


void ferCHull3Del(fer_chull3_t *h)
{
    ferMesh3Del2(h->mesh,
                 vertDelMesh, NULL,
                 edgeDelMesh, NULL,
                 faceDelMesh, NULL);

    FER_FREE(h);
}

void ferCHull3Add(fer_chull3_t *h, const fer_vec3_t *point)
{
    fer_list_t border_edges;

    if (h->coplanar && isCoplanarWithHull(h, point)){
        if (!liesOnCoplanar(h, point))
            addCoplanar(h, point);
        return;
    }else if (h->coplanar){
        firstNonCoplanar(h, point);
        h->coplanar = 0;
        return;
    }

    ferListInit(&border_edges);
    findBorderEdges(h, point, &border_edges);
    if (ferListEmpty(&border_edges))
        return;


    makeCone(h, &border_edges, point);
}

void ferCHull3DumpSVT(fer_chull3_t *h, FILE *out, const char *name)
{
    ferMesh3DumpSVT(h->mesh, out, name);
}


_fer_inline int isVisible(const fer_chull3_t *h,
                          const fer_vec3_t *v,
                          const fer_chull3_face_t *f)
{
    fer_real_t orient;
    orient = orient3d(h, v, f);
    return orient > FER_ZERO;
}

_fer_inline int isVisible2(const fer_chull3_t *h,
                           const fer_vec3_t *v,
                           const fer_vec3_t *f1, const fer_vec3_t *f2,
                           const fer_vec3_t *f3)
{
    fer_real_t orient;
    orient = orient3d2(h, v, f1, f2, f3);
    return orient > FER_ZERO;
}


_fer_inline fer_real_t orient3d(const fer_chull3_t *h,
                                const fer_vec3_t *v,
                                const fer_chull3_face_t *f)
{
    return ferVec3Volume6(&f->v[0]->v, &f->v[1]->v, &f->v[2]->v, v);
    //return ferPredOrient3d(&h->pred, &f->v[0]->v, &f->v[1]->v, &f->v[2]->v, v);
}

_fer_inline fer_real_t orient3d2(const fer_chull3_t *h,
                                 const fer_vec3_t *v,
                                 const fer_vec3_t *f1, const fer_vec3_t *f2,
                                 const fer_vec3_t *f3)
{
    return ferVec3Volume6(f1, f2, f3, v);
    //return ferPredOrient3d(&h->pred, f1, f2, f3, v);
}

_fer_inline int isCoplanar(const fer_chull3_t *h,
                           const fer_vec3_t *v,
                           const fer_chull3_face_t *f)
{
    fer_real_t orient;
    orient = orient3d(h, v, f);

    return ferIsZero(orient);
}


static fer_chull3_vert_t *vertNew(fer_chull3_t *h, const fer_vec3_t *v)
{
    fer_chull3_vert_t *vert;

    vert = FER_ALLOC_ALIGN(fer_chull3_vert_t, 16);

    // copy coordinates
    ferVec3Copy(&vert->v, v);

    // init mesh structure
    ferMesh3VertexSetCoords(&vert->m, &vert->v);

    // add vertex to mesh
    ferMesh3AddVertex(h->mesh, &vert->m);

    vert->border_edges = 0;

    return vert;
}

static void vertDel(fer_chull3_t *h, fer_chull3_vert_t *v)
{
    ferMesh3RemoveVertex(h->mesh, &v->m);
    FER_FREE(v);
}
static void vertDelMesh(fer_mesh3_vertex_t *_v, void *_)
{
    fer_chull3_vert_t *v;

    v = fer_container_of(_v, fer_chull3_vert_t, m);
    FER_FREE(v);
}


static fer_chull3_edge_t *edgeNew(fer_chull3_t *h,
                                  fer_chull3_vert_t *v1, fer_chull3_vert_t *v2)
{
    fer_chull3_edge_t *e;

    e = FER_ALLOC(fer_chull3_edge_t);
    ferMesh3AddEdge(h->mesh, &e->m, &v1->m, &v2->m);
    e->swap = 0;
    e->onedge = 0;

    return e;
}

static void edgeDel(fer_chull3_t *h, fer_chull3_edge_t *e)
{
    ferMesh3RemoveEdge(h->mesh, &e->m);
    FER_FREE(e);
}

static void edgeDelMesh(fer_mesh3_edge_t *_e, void *_)
{
    fer_chull3_edge_t *e;

    e = fer_container_of(_e, fer_chull3_edge_t, m);
    FER_FREE(e);
}

static void edgeVertices(fer_chull3_edge_t *e, fer_chull3_vert_t **v)
{
    fer_mesh3_vertex_t *mv;
    int i;

    for (i = 0; i < 2; i++){
        mv = ferMesh3EdgeVertex(&e->m, i);
        v[i] = fer_container_of(mv, fer_chull3_vert_t, m);
    }
}



static fer_chull3_face_t *faceNew(fer_chull3_t *h,
                                  fer_chull3_edge_t *e1, fer_chull3_edge_t *e2,
                                  fer_chull3_edge_t *e3)
{
    fer_chull3_face_t *f;

    f = FER_ALLOC(fer_chull3_face_t);
    if (ferMesh3AddFace(h->mesh, &f->m, &e1->m, &e2->m, &e3->m) != 0){
        DBG("Can't add face, %d", (int)ferMesh3VerticesLen(h->mesh));
        // ferCHull3DumpSVT(h, stdout, "Can't face");
    }

    f->v[0] = f->v[1] = f->v[2] = NULL;

    return f;
}

static void faceDel(fer_chull3_t *h, fer_chull3_face_t *f)
{
    ferMesh3RemoveFace(h->mesh, &f->m);
    FER_FREE(f);
}

static void faceDelMesh(fer_mesh3_face_t *_f, void *_)
{
    fer_chull3_face_t *f;

    f = fer_container_of(_f, fer_chull3_face_t, m);
    FER_FREE(f);
}

static void faceSetVertices(fer_chull3_t *h, fer_chull3_face_t *f,
                            fer_chull3_vert_t *v1, fer_chull3_vert_t *v2,
                            fer_chull3_vert_t *v3)
{
    f->v[0] = v1;
    f->v[1] = v2;
    f->v[2] = v3;
}

static void faceEdges(fer_chull3_face_t *f, fer_chull3_edge_t **e)
{
    fer_mesh3_edge_t *me;
    int i;

    for (i = 0; i < 3; i++){
        me = ferMesh3FaceEdge(&f->m, i);
        e[i] = fer_container_of(me, fer_chull3_edge_t, m);
    }
}


static int isCoplanarWithHull(fer_chull3_t *h, const fer_vec3_t *v)
{
    fer_list_t *list, *item;
    fer_mesh3_face_t *mf;
    fer_chull3_face_t *f;

    if (ferMesh3VerticesLen(h->mesh) < 3)
        return 1;

    list = ferMesh3Faces(h->mesh);
    FER_LIST_FOR_EACH(list, item){
        mf = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
        f  = fer_container_of(mf, fer_chull3_face_t, m);

        if (!isCoplanar(h, v, f))
            return 0;
    }

    return 1;
}

static int liesOnCoplanar(fer_chull3_t *h, const fer_vec3_t *v)
{
    fer_list_t *list, *item;
    fer_mesh3_face_t *mf;
    fer_chull3_face_t *f;

    if (ferMesh3VerticesLen(h->mesh) < 3)
        return 0;

    list = ferMesh3Faces(h->mesh);
    FER_LIST_FOR_EACH(list, item){
        mf = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
        f  = fer_container_of(mf, fer_chull3_face_t, m);

        if (!ferVec3PointInTri(v, &f->v[0]->v, &f->v[1]->v, &f->v[2]->v))
            return 0;
    }

    return 1;
}

static void addCoplanarGrow(fer_chull3_t *h, fer_chull3_vert_t *v,
                            fer_chull3_edge_t *e, const fer_vec3_t *topv,
                            int inv)
{
    fer_list_t *list, *item;
    fer_mesh3_vertex_t *mv1, *mv2;
    fer_mesh3_edge_t *me;
    fer_chull3_vert_t *v2, *v3;
    fer_chull3_edge_t *e2, *e3;
    fer_chull3_face_t *f;

    while (e){
        mv1 = ferMesh3EdgeOtherVertex(&e->m, &v->m);
        list = ferMesh3VertexEdges(mv1);
        FER_LIST_FOR_EACH(list, item){
            me = ferMesh3EdgeFromVertexList(item);

            if (me != &e->m && ferMesh3EdgeFacesLen(me) == 1){
                mv2 = ferMesh3EdgeOtherVertex(me, mv1);

                if (isVisible2(h, topv, &v->v, mv1->v, mv2->v)){
                    v2 = fer_container_of(mv1, fer_chull3_vert_t, m);
                    v3 = fer_container_of(mv2, fer_chull3_vert_t, m);
                    e2 = fer_container_of(me, fer_chull3_edge_t, m);

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

static void addCoplanar3(fer_chull3_t *h, const fer_vec3_t *v)
{
    fer_list_t *list, *item;
    fer_mesh3_vertex_t *mv;
    fer_chull3_vert_t *v1, *v2, *vert;
    fer_chull3_edge_t *e1, *e2, *e3;
    fer_chull3_face_t *f;
    fer_real_t dist1, dist2, dist3;

    // first check if new vertex isn't collinear with the two already
    // in mesh
    list = ferMesh3Vertices(h->mesh);
    item = ferListNext(list);
    mv   = FER_LIST_ENTRY(item, fer_mesh3_vertex_t, list);
    v1   = fer_container_of(mv, fer_chull3_vert_t, m);
    item = ferListPrev(list);
    mv   = FER_LIST_ENTRY(item, fer_mesh3_vertex_t, list);
    v2   = fer_container_of(mv, fer_chull3_vert_t, m);

    if (ferVec3Collinear(v, &v1->v, &v2->v)){
        // vertices are collinear, find out pair with longest distance
        // and keep that pair
        dist1 = ferVec3Dist2(v, &v1->v);
        dist2 = ferVec3Dist2(v, &v2->v);
        dist3 = ferVec3Dist2(&v1->v, &v2->v);
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

static void addCoplanar4(fer_chull3_t *h, const fer_vec3_t *point)
{
    fer_list_t *list, *item;
    fer_chull3_vert_t *v[4];
    fer_chull3_edge_t *e[3];
    fer_chull3_face_t *f, *f2;
    fer_mesh3_face_t *mf;
    fer_mesh3_edge_t *me;
    fer_real_t dist1, dist2;
    fer_vec3_t cross, p01, p02, topv;
    int inv;


    // we already have at least triangle
    // find nearest edge and make triangle

    // find nearest edge to connect triangle
    e[0] = NULL;
    v[0] = v[1] = NULL;
    dist2 = FER_REAL_MAX;
    list = ferMesh3Edges(h->mesh);
    FER_LIST_FOR_EACH(list, item){
        me = FER_LIST_ENTRY(item, fer_mesh3_edge_t, list);
        if (ferMesh3EdgeFacesLen(me) != 1)
            continue;

        e[1] = fer_container_of(me, fer_chull3_edge_t, m);
        edgeVertices(e[1], v + 2);

        dist1 = ferVec3PointSegmentDist2(point, &v[2]->v, &v[3]->v, NULL);
        if (ferEq(dist1, dist2)){
            if (ferVec3TriArea2(point, &v[2]->v, &v[3]->v)
                    > ferVec3TriArea2(point, &v[0]->v, &v[1]->v)){
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
    if (ferIsZero(ferVec3TriArea2(point, &v[0]->v, &v[1]->v))){
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
    mf = ferMesh3EdgeOtherFace(&e[0]->m, &f->m);
    f2 = fer_container_of(mf, fer_chull3_face_t, m);

    // 6. orient vertices in new face
    faceSetVertices(h, f, v[2], v[0], v[1]);
    inv = 1;
    if ((f2->v[0] == v[0] && f2->v[1] == v[1])
            || (f2->v[1] == v[0] && f2->v[2] == v[1])
            || (f2->v[2] == v[0] && f2->v[0] == v[1])){
        faceSetVertices(h, f, v[2], v[1], v[0]);
        inv = 0;
    }

    ferVec3Sub2(&p01, &v[0]->v, &v[2]->v);
    ferVec3Sub2(&p02, &v[1]->v, &v[2]->v);
    ferVec3Cross(&cross, &p01, &p02);
    ferVec3Normalize(&cross);

    ferVec3Add2(&topv, &v[2]->v, &cross);
    addCoplanarGrow(h, v[2], e[1], &topv, inv);

    ferVec3Scale(&cross, -FER_ONE);
    ferVec3Add2(&topv, &v[2]->v, &cross);
    addCoplanarGrow(h, v[2], e[2], &topv, !inv);
}


static void addCoplanar(fer_chull3_t *h, const fer_vec3_t *v)
{
    if (ferMesh3VerticesLen(h->mesh) < 2){
        vertNew(h, v);
    }else if (ferMesh3VerticesLen(h->mesh) < 3){
        addCoplanar3(h, v);
    }else{
        addCoplanar4(h, v);
    }
}

static void firstNonCoplanar(fer_chull3_t *h, const fer_vec3_t *point)
{
    fer_list_t *list, *item;
    fer_list_t border_edges;
    fer_mesh3_face_t *mf;
    fer_chull3_face_t *f;
    fer_chull3_edge_t *e[3];
    fer_chull3_vert_t *v[2];
    int reorient, i;

    // first find out if we need to reorient faces
    reorient = 0;
    list = ferMesh3Faces(h->mesh);
    FER_LIST_FOR_EACH(list, item){
        mf = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
        f  = fer_container_of(mf, fer_chull3_face_t, m);

        if (isVisible(h, point, f)){
            reorient = 1;
            break;
        }
    }

    if (reorient){
        list = ferMesh3Faces(h->mesh);
        FER_LIST_FOR_EACH(list, item){
            mf = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
            f  = fer_container_of(mf, fer_chull3_face_t, m);

            faceSetVertices(h, f, f->v[0], f->v[2], f->v[1]);
        }
    }

    ferListInit(&border_edges);
    list = ferMesh3Faces(h->mesh);
    FER_LIST_FOR_EACH(list, item){
        mf = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
        f  = fer_container_of(mf, fer_chull3_face_t, m);

        // obtain border edges
        faceEdges(f, e);

        // append edges that incidents with non-visible face to list of
        // border faces
        for (i = 0; i < 3; i++){
            if (ferMesh3EdgeFacesLen(&e[i]->m) == 1){
                // orient edge correctly
                edgeVertices(e[i], v);

                e[i]->swap = 0;
                if ((v[0] == f->v[0] && v[1] == f->v[1])
                        || (v[0] == f->v[1] && v[1] == f->v[2])
                        || (v[0] == f->v[2] && v[1] == f->v[0])){
                    e[i]->swap = 1;
                }

                // append it to list of edges
                ferListAppend(&border_edges, &e[i]->list);
            }
        }
    }

    makeCone(h, &border_edges, point);
}

static void updateBorderEdges(fer_chull3_t *h, fer_chull3_face_t *f,
                              fer_list_t *edges, fer_list_t *wrong_vertices)
{
    fer_chull3_edge_t *e[3];
    fer_chull3_vert_t *v[2];
    int i;

    faceEdges(f, e);

    // add edges to list of border edges
    for (i = 0; i < 3; i++){
        if (!e[i]->onedge){
            edgeVertices(e[i], v);

            e[i]->onedge = 1;
            ferListAppend(edges, &e[i]->list);

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
                ferListAppend(wrong_vertices, &v[0]->list);
            if (v[1]->border_edges == 3)
                ferListAppend(wrong_vertices, &v[1]->list);
        }
    }

    // delete face
    faceDel(h, f);

    // delete edges and vertices that were left alone 
    for (i = 0; i < 3; i++){
        if (ferMesh3EdgeFacesLen(&e[i]->m) == 0){
            edgeVertices(e[i], v);

            ferListDel(&e[i]->list);
            edgeDel(h, e[i]);

            v[0]->border_edges--;
            v[1]->border_edges--;
            if (v[0]->border_edges == 2)
                ferListDel(&v[0]->list);
            if (v[1]->border_edges == 2)
                ferListDel(&v[1]->list);

            if (ferMesh3VertexEdgesLen(&v[0]->m) == 0)
                vertDel(h, v[0]);
            if (ferMesh3VertexEdgesLen(&v[1]->m) == 0)
                vertDel(h, v[1]);
        }
    }
}

static void correctBorderEdges(fer_chull3_t *h, const fer_vec3_t *point,
                               fer_chull3_vert_t *v,
                               fer_list_t *edges,
                               fer_list_t *wrong_vertices)
{
    fer_list_t *list, *item;
    fer_mesh3_edge_t *me;
    fer_mesh3_face_t *mf;
    fer_real_t orient, best_orient;
    fer_chull3_face_t *f, *best_f;
    int i;

    best_orient = -FER_REAL_MAX;
    best_f = NULL;

    list = ferMesh3VertexEdges(&v->m);
    FER_LIST_FOR_EACH(list, item){
        me = ferMesh3EdgeFromVertexList(item);

        for (i = 0; i < 2; i++){
            mf = ferMesh3EdgeFace(me, i);
            if (mf && (!best_f || &best_f->m != mf)){
                f = fer_container_of(mf, fer_chull3_face_t, m);

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

static void findBorderEdges(fer_chull3_t *h, const fer_vec3_t *point,
                            fer_list_t *edges)
{
    fer_list_t *list, *item, *itemtmp, wrong_vertices, faces;
    fer_mesh3_face_t *mf;
    fer_chull3_face_t *f;
    fer_chull3_vert_t *v;

    ferListInit(&wrong_vertices);
    ferListInit(&faces);

    list = ferMesh3Faces(h->mesh);
    FER_LIST_FOR_EACH_SAFE(list, item, itemtmp){
        mf = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
        f  = fer_container_of(mf, fer_chull3_face_t, m);

        if (isVisible(h, point, f)){
            ferListAppend(&faces, &f->list);
        }else{
            // skip this point if it is duplicate with any vertex
            if (ferVec3Eq(point, &f->v[0]->v)
                    || ferVec3Eq(point, &f->v[1]->v)
                    || ferVec3Eq(point, &f->v[2]->v)){
                ferListInit(&faces);
                break;
            }
        }
    }

    while (!ferListEmpty(&faces)){
        item = ferListNext(&faces);
        ferListDel(item);
        f = FER_LIST_ENTRY(item, fer_chull3_face_t, list);

        updateBorderEdges(h, f, edges, &wrong_vertices);
    }

    while (!ferListEmpty(&wrong_vertices)){
        item = ferListNext(&wrong_vertices);
        v = FER_LIST_ENTRY(item, fer_chull3_vert_t, list);

        correctBorderEdges(h, point, v, edges, &wrong_vertices);
    }
}


static void makeCone(fer_chull3_t *h, fer_list_t *edges, const fer_vec3_t *point)
{
    fer_chull3_edge_t *e1, *e2, *e3;
    fer_chull3_vert_t *v[3];
    fer_chull3_face_t *f;
    fer_mesh3_edge_t *me;
    fer_list_t *item;

    v[0] = vertNew(h, point);

    FER_LIST_FOR_EACH(edges, item){
        e1 = FER_LIST_ENTRY(item, fer_chull3_edge_t, list);
        e1->onedge = 0;

        edgeVertices(e1, v + 1);
        v[1]->border_edges = 0;
        v[2]->border_edges = 0;

        me = ferMesh3VertexCommonEdge(&v[0]->m, &v[1]->m);
        if (me == NULL){
            e2 = edgeNew(h, v[0], v[1]);
        }else{
            e2 = fer_container_of(me, fer_chull3_edge_t, m);
        }

        me = ferMesh3VertexCommonEdge(&v[0]->m, &v[2]->m);
        if (me == NULL){
            e3 = edgeNew(h, v[0], v[2]);
        }else{
            e3 = fer_container_of(me, fer_chull3_edge_t, m);
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
