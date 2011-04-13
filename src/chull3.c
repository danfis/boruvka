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
#include <fermat/predicates.h>
#include <fermat/rand.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>


struct _fer_chull3_vert_t {
    fer_vec3_t v;
    fer_mesh3_vertex_t m;
    fer_list_t list;
} fer_aligned(16) fer_packed;
typedef struct _fer_chull3_vert_t fer_chull3_vert_t;

struct _fer_chull3_edge_t {
    fer_mesh3_edge_t m;
    fer_list_t list;
};
typedef struct _fer_chull3_edge_t fer_chull3_edge_t;

struct _fer_chull3_face_t {
    fer_mesh3_face_t m;
    int visible;
    fer_chull3_vert_t *v[3];
    fer_list_t list;
};
typedef struct _fer_chull3_face_t fer_chull3_face_t;

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


/** Adds point to list of vertices and we have 3 non-collinear vertices
 *  already 'double triangle' is created, i.e. two triangles formed by same
 *  three vertices but triangles are oriented in opposite direction. */
static void addInit(fer_chull3_t *h, const fer_vec3_t *v);
/** Fill list with faces visible from v */
static void findVisibleFaces(fer_chull3_t *h, const fer_vec3_t *v,
                             fer_list_t *list);
/** Creates hole in mesh (i.e., visible faces are deleted) and list {edges}
 *  is filled with edges on border of hole */
static void createHole(fer_chull3_t *h, fer_list_t *visible,
                       fer_list_t *edges);
/** Makes new cone connecting new point {v} with border edges */
static void makeCone(fer_chull3_t *h, fer_list_t *edges, const fer_vec3_t *v);

fer_chull3_t *ferCHull3New(void)
{
    fer_chull3_t *h;

    h = FER_ALLOC(fer_chull3_t);
    h->mesh = ferMesh3New();
    return h;
}


void ferCHull3Del(fer_chull3_t *h)
{
    ferMesh3Del2(h->mesh,
                 vertDelMesh, NULL,
                 edgeDelMesh, NULL,
                 faceDelMesh, NULL);

    free(h);
}

void ferCHull3Add(fer_chull3_t *h, const fer_vec3_t *point)
{
    fer_list_t visible;
    fer_list_t border_edges;

    if (ferMesh3VerticesLen(h->mesh) < 3){
        addInit(h, point);
        return;
    }

    ferListInit(&visible);
    findVisibleFaces(h, point, &visible);
    if (ferListEmpty(&visible))
        return;


    ferListInit(&border_edges);
    createHole(h, &visible, &border_edges);
    if (ferListEmpty(&border_edges))
        return;


    makeCone(h, &border_edges, point);
}

void ferCHull3DumpSVT(fer_chull3_t *h, FILE *out, const char *name)
{
    ferMesh3DumpSVT(h->mesh, out, name);
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

    return vert;
}

static void vertDel(fer_chull3_t *h, fer_chull3_vert_t *v)
{
    ferMesh3RemoveVertex(h->mesh, &v->m);
    free(v);
}
static void vertDelMesh(fer_mesh3_vertex_t *_v, void *_)
{
    fer_chull3_vert_t *v;

    v = fer_container_of(_v, fer_chull3_vert_t, m);
    free(v);
}


static fer_chull3_edge_t *edgeNew(fer_chull3_t *h,
                                  fer_chull3_vert_t *v1, fer_chull3_vert_t *v2)
{
    fer_chull3_edge_t *e;

    e = FER_ALLOC(fer_chull3_edge_t);
    ferMesh3AddEdge(h->mesh, &e->m, &v1->m, &v2->m);

    return e;
}

static void edgeDel(fer_chull3_t *h, fer_chull3_edge_t *e)
{
    ferMesh3RemoveEdge(h->mesh, &e->m);
    free(e);
}

static void edgeDelMesh(fer_mesh3_edge_t *_e, void *_)
{
    fer_chull3_edge_t *e;

    e = fer_container_of(_e, fer_chull3_edge_t, m);
    free(e);
}

static fer_chull3_face_t *faceNew(fer_chull3_t *h,
                                  fer_chull3_edge_t *e1, fer_chull3_edge_t *e2,
                                  fer_chull3_edge_t *e3)
{
    fer_chull3_face_t *f;

    f = FER_ALLOC(fer_chull3_face_t);
    ferMesh3AddFace(h->mesh, &f->m, &e1->m, &e2->m, &e3->m);
    f->v[0] = f->v[1] = f->v[2] = NULL;
    return f;
}

static void faceDel(fer_chull3_t *h, fer_chull3_face_t *f)
{
    ferMesh3RemoveFace(h->mesh, &f->m);
    free(f);
}

static void faceDelMesh(fer_mesh3_face_t *_f, void *_)
{
    fer_chull3_face_t *f;

    f = fer_container_of(_f, fer_chull3_face_t, m);
    free(f);
}

static void faceSetVertices(fer_chull3_t *h, fer_chull3_face_t *f,
                            fer_chull3_vert_t *v1, fer_chull3_vert_t *v2,
                            fer_chull3_vert_t *v3)
{
    f->v[0] = v1;
    f->v[1] = v2;
    f->v[2] = v3;
}



static void addInit(fer_chull3_t *h, const fer_vec3_t *v)
{
    fer_chull3_vert_t *vert;
    fer_chull3_vert_t *v1, *v2;
    fer_chull3_edge_t *e1, *e2, *e3;
    fer_chull3_face_t *f1, *f2;
    fer_mesh3_vertex_t *mv;
    fer_list_t *list, *item;
    fer_real_t dist1, dist2, dist3;

    if (ferMesh3VerticesLen(h->mesh) < 2){
        vert = vertNew(h, v);
    }else{
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

            // 3. create two faces
            f1 = faceNew(h, e1, e2, e3);
            f2 = faceNew(h, e1, e2, e3);

            // 4. orient the faces
            faceSetVertices(h, f1, vert, v1, v2);
            faceSetVertices(h, f2, vert, v2, v1);
        }
    }
}

static void findVisibleFacesCoplanar(fer_chull3_t *h, const fer_vec3_t *v,
                                     fer_list_t *visible)
{
    fer_vec3_t w;
    fer_rand_t r;

    ferRandInit(&r);

    ferVec3Copy(&w, v);
    ferVec3AddConst(&w, ferRand(&r, -0.01, 0.01));

    findVisibleFaces(h, &w, visible);
}

static void findVisibleFaces(fer_chull3_t *h, const fer_vec3_t *v,
                             fer_list_t *visible)
{
    fer_list_t *list, *item;
    fer_mesh3_face_t *mf;
    fer_chull3_face_t *f;
    fer_real_t orient;
    size_t overall, coplanar;

    //DBG2("");

    overall = coplanar = 0;

    list = ferMesh3Faces(h->mesh);
    FER_LIST_FOR_EACH(list, item){
        mf = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
        f  = fer_container_of(mf, fer_chull3_face_t, m);
        //DBG("%lx, v[0,1,2] %lx %lx %lx", (long)f, (long)f->v[0], (long)f->v[1], (long)f->v[2]);

        f->visible = 0;

        // get orientation of vertex v
        // TODO: Use exact arithmetic?
        orient = ferPredOrient3dFast(&f->v[0]->v, &f->v[1]->v, &f->v[2]->v, v);

        //DBG("orient: %f", orient);

        if (ferIsZero(orient)){
            // points are coplanar
            coplanar++;
        }else if (orient > FER_ZERO){
            f->visible = 1;
            ferListAppend(visible, &f->list);
        }

        overall++;
    }

    if (overall == coplanar){
        DBG2("overall == coplanar");
        findVisibleFacesCoplanar(h, v, visible);
    }
}

static void createHole(fer_chull3_t *h, fer_list_t *visible,
                       fer_list_t *edges)
{
    fer_chull3_face_t *f, *f2;
    fer_chull3_edge_t *e[3];
    fer_chull3_vert_t *v0, *v1;
    fer_mesh3_edge_t *me;
    fer_mesh3_face_t *mf;
    fer_mesh3_vertex_t *mv;
    fer_list_t *item;
    int i;

    //DBG2("");

    while (!ferListEmpty(visible)){
        item = ferListNext(visible);
        ferListDel(item);

        f = FER_LIST_ENTRY(item, fer_chull3_face_t, list);

        // obtain border edges
        for (i = 0; i < 3; i++){
            me   = ferMesh3FaceEdge(&f->m, i);
            e[i] = fer_container_of(me, fer_chull3_edge_t, m);
        }

        // append edges that incidents with non-visible face to list of
        // border faces
        for (i = 0; i < 3; i++){
            if (ferMesh3EdgeFacesLen(&e[i]->m) == 2){
                if ((mf = ferMesh3EdgeFace(&e[i]->m, 0)) == &f->m)
                    mf = ferMesh3EdgeFace(&e[i]->m, 1);
                f2 = fer_container_of(mf, fer_chull3_face_t, m);

                if (!f2->visible){
                    // orient edge correctly
                    mv = ferMesh3EdgeVertex(&e[i]->m, 0);
                    v0 = fer_container_of(mv, fer_chull3_vert_t, m);
                    mv = ferMesh3EdgeVertex(&e[i]->m, 1);
                    v1 = fer_container_of(mv, fer_chull3_vert_t, m);

                    if (v0 == f2->v[0]){
                        if (v1 == f2->v[1]){
                            ferMesh3EdgeSwapVertices(&e[i]->m);
                        }
                    }else if (v0 == f2->v[1]){
                        if (v1 == f2->v[2]){
                            ferMesh3EdgeSwapVertices(&e[i]->m);
                        }
                    }else if (v0 == f2->v[2]){
                        if (v1 == f2->v[0]){
                            ferMesh3EdgeSwapVertices(&e[i]->m);
                        }
                    }

                    // append it to list of edges
                    ferListAppend(edges, &e[i]->list);
                }
            }
        }

        // delete face
        faceDel(h, f);

        // delete those edges that were left alone
        for (i = 0; i < 3; i++){
            if (ferMesh3EdgeFacesLen(&e[i]->m) == 0){
                mv = ferMesh3EdgeVertex(&e[i]->m, 0);
                v0 = fer_container_of(mv, fer_chull3_vert_t, m);
                mv = ferMesh3EdgeVertex(&e[i]->m, 1);
                v1 = fer_container_of(mv, fer_chull3_vert_t, m);

                edgeDel(h, e[i]);

                if (ferMesh3VertexEdgesLen(&v0->m) == 0)
                    vertDel(h, v0);
                if (ferMesh3VertexEdgesLen(&v1->m) == 0)
                    vertDel(h, v1);
            }
        }
    }
}

static void makeCone(fer_chull3_t *h, fer_list_t *edges, const fer_vec3_t *v)
{
    fer_chull3_edge_t *e1, *e2, *e3;
    fer_chull3_vert_t *v1, *v2, *v3;
    fer_chull3_face_t *f;
    fer_mesh3_vertex_t *mv;
    fer_mesh3_edge_t *me;
    fer_list_t *item;

    //DBG2("");

    v1 = vertNew(h, v);

    FER_LIST_FOR_EACH(edges, item){
        e1 = FER_LIST_ENTRY(item, fer_chull3_edge_t, list);

        mv = ferMesh3EdgeVertex(&e1->m, 0);
        v2 = fer_container_of(mv, fer_chull3_vert_t, m);
        mv = ferMesh3EdgeVertex(&e1->m, 1);
        v3 = fer_container_of(mv, fer_chull3_vert_t, m);

        me = ferMesh3VertexCommonEdge(&v1->m, &v2->m);
        if (me == NULL){
            e2 = edgeNew(h, v1, v2);
        }else{
            e2 = fer_container_of(me, fer_chull3_edge_t, m);
        }

        me = ferMesh3VertexCommonEdge(&v1->m, &v3->m);
        if (me == NULL){
            e3 = edgeNew(h, v1, v3);
        }else{
            e3 = fer_container_of(me, fer_chull3_edge_t, m);
        }

        // create new face
        f = faceNew(h, e1, e2, e3);

        // orient correctly vertices
        faceSetVertices(h, f, v1, v2, v3);
    }
}
