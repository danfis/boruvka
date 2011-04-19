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
} fer_aligned(16) fer_packed;
typedef struct _fer_chull3_vert_t fer_chull3_vert_t;

struct _fer_chull3_edge_t {
    fer_mesh3_edge_t m;
    fer_list_t list;
    int swap;
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


/** Returns true if hull and with given point is still coplanar */
static int isCoplanar(fer_chull3_t *h, const fer_vec3_t *v);
/** Returns true if given point lies on current coplanar patch */
static int liesOnCoplanar(fer_chull3_t *h, const fer_vec3_t *v);
/** Adds point to list of vertices and we have 3 non-collinear vertices
 *  already 'double triangle' is created, i.e. two triangles formed by same
 *  three vertices but triangles are oriented in opposite direction. */
/** TODO */
static void addCoplanar(fer_chull3_t *h, const fer_vec3_t *v);
/** Fill list with faces visible from v */
static void findVisibleFaces(fer_chull3_t *h, const fer_vec3_t *v,
                             fer_list_t *list);
/** Adds first non-coplanar point on hull */
static void firstNonCoplanar(fer_chull3_t *h, const fer_vec3_t *v);
/** Creates hole in mesh (i.e., visible faces are deleted) and list {edges}
 *  is filled with edges on border of hole */
static void createHole(fer_chull3_t *h, fer_list_t *visible,
                       fer_list_t *edges);
/** Makes new cone connecting new point {v} with border edges */
static void makeCone(fer_chull3_t *h, fer_list_t *edges, const fer_vec3_t *v);

fer_chull3_t *ferCHull3New(void)
{
    fer_chull3_t *h;

    ferPredInit();

    h = FER_ALLOC(fer_chull3_t);
    h->mesh = ferMesh3New();
    h->eps = FER_CHULL3_EPS;
    //TODO ferRandInit(&h->rand);
    h->coplanar = 1;
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


    fprintf(stderr, "POINT: ");
    ferVec3Print(point, stderr);
    fprintf(stderr, "\n");
    if (h->coplanar && isCoplanar(h, point)){
        if (!liesOnCoplanar(h, point))
            addCoplanar(h, point);
        return;
    }else if (h->coplanar){
        //DBG2("non pre");
        //ferCHull3DumpSVT(h, stdout, "non pre");
        firstNonCoplanar(h, point);
        //DBG2("non");
        //ferCHull3DumpSVT(h, stdout, "non");
        h->coplanar = 0;
        return;
    }

    DBG2("first non non");
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
    if (ferMesh3AddFace(h->mesh, &f->m, &e1->m, &e2->m, &e3->m) != 0){
        DBG("Can't add face, %d", (int)ferMesh3VerticesLen(h->mesh));
        ferCHull3DumpSVT(h, stdout, "Can't face");
        {
            printf("--\nName: face\nFace color: 0.8 0.8 0\nPoints:\n");
            ferVec3Print(&f->v[0]->v, stdout); printf("\n");
            ferVec3Print(&f->v[1]->v, stdout); printf("\n");
            ferVec3Print(&f->v[2]->v, stdout); printf("\n");
            printf("Faces:\n0 1 2\n--\n");
        }
        fflush(stdout);
    }

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



static int isCoplanar(fer_chull3_t *h, const fer_vec3_t *v)
{
    fer_list_t *list, *item;
    fer_mesh3_face_t *mf;
    fer_chull3_face_t *f;
    fer_real_t orient;

    if (ferMesh3VerticesLen(h->mesh) < 3)
        return 1;

    list = ferMesh3Faces(h->mesh);
    FER_LIST_FOR_EACH(list, item){
        mf = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
        f  = fer_container_of(mf, fer_chull3_face_t, m);

        orient = ferVec3Volume6(&f->v[0]->v, &f->v[1]->v, &f->v[2]->v, v);
        orient = orient / FER_REAL(6.);
        //DBG("orient: %.30f", orient);
        if (!ferIsZero(orient))
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
    fer_real_t orient;

    while (e){
        //DBG2("e");
        mv1 = ferMesh3EdgeOtherVertex(&e->m, &v->m);
        list = ferMesh3VertexEdges(mv1);
        FER_LIST_FOR_EACH(list, item){
            me = ferMesh3EdgeFromVertexList(item);

            if (me != &e->m && ferMesh3EdgeFacesLen(me) == 1){
                mv2 = ferMesh3EdgeOtherVertex(me, mv1);

                /*
                {
                    printf("---\nName: a\nPoint size: 10\nPoints:\n");
                    ferVec3Print(&v->v, stdout);printf("\n---\n");
                    printf("---\nPoint size: 10\nPoints:\n");
                    ferVec3Print(mv1->v, stdout);printf("\n---\n");
                    printf("---\nPoint size: 10\nPoints:\n");
                    ferVec3Print(mv2->v, stdout);printf("\n---\n");
                }
                */

                orient = ferVec3Volume6(&v->v, mv1->v, mv2->v, topv);
                orient = orient / FER_REAL(6.);
                //DBG("orient: %f", orient);
                if (!ferIsZero(orient) && orient > FER_ZERO){
                    v2 = fer_container_of(mv1, fer_chull3_vert_t, m);
                    v3 = fer_container_of(mv2, fer_chull3_vert_t, m);
                    e2 = fer_container_of(me, fer_chull3_edge_t, m);

                    e3 = edgeNew(h, v, v3);

                    //DBG2("faceNew");
                    f = faceNew(h, e, e2, e3);

                    // orient face
                    if (inv){
                        faceSetVertices(h, f, v, v3, v2);
                    }else{
                        faceSetVertices(h, f, v, v2, v3);
                    }

                    {
                        /*
                        printf("---\nPoint size: 10\nPoints:\n");
                        ferVec3Print(&v->v, stdout);printf("\n---\n");
                        printf("---\nPoint size: 10\nPoints:\n");
                        ferVec3Print(mv1->v, stdout);printf("\n---\n");
                        printf("---\nPoint size: 10\nPoints:\n");
                        ferVec3Print(mv2->v, stdout);printf("\n---\n");
                        */
                        /*
                        printf("----\nFace color:0.8 0 0\nPoints:\n");
                        ferVec3Print(&v->v, stdout); printf("\n");
                        ferVec3Print(&v2->v, stdout); printf("\n");
                        ferVec3Print(&v3->v, stdout); printf("\n");
                        printf("Faces:\n0 1 2\n---\n");
                        */
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

static void addCoplanar(fer_chull3_t *h, const fer_vec3_t *v)
{
    fer_chull3_vert_t *vert;
    fer_chull3_vert_t *v1, *v2;
    fer_chull3_edge_t *e1, *e2, *e3;
    fer_chull3_face_t *f, *f2;
    fer_mesh3_vertex_t *mv[4];
    fer_mesh3_edge_t *me;
    fer_mesh3_face_t *mf;
    fer_list_t *list, *item;
    fer_real_t dist1, dist2, dist3;
    fer_vec3_t cross, p01, p02, topv;
    int inv;

    if (ferMesh3VerticesLen(h->mesh) < 2){
        vert = vertNew(h, v);
    }else if (ferMesh3VerticesLen(h->mesh) < 3){
        // first check if new vertex isn't collinear with the two already
        // in mesh
        list  = ferMesh3Vertices(h->mesh);
        item  = ferListNext(list);
        mv[0] = FER_LIST_ENTRY(item, fer_mesh3_vertex_t, list);
        v1    = fer_container_of(mv[0], fer_chull3_vert_t, m);
        item  = ferListPrev(list);
        mv[0] = FER_LIST_ENTRY(item, fer_mesh3_vertex_t, list);
        v2    = fer_container_of(mv[0], fer_chull3_vert_t, m);

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
    }else{
        // we already have at least triangle
        // find nearest edge and make triangle
        fer_vec3_t q;

        //DBG2("");
        // find nearest edge to connect triangle
        e1 = NULL;
        list = ferMesh3Edges(h->mesh);
        dist2 = FER_REAL_MAX;
        FER_LIST_FOR_EACH(list, item){
            me = FER_LIST_ENTRY(item, fer_mesh3_edge_t, list);
            if (ferMesh3EdgeFacesLen(me) != 1)
                continue;

            mv[0] = ferMesh3EdgeVertex(me, 0);
            mv[1] = ferMesh3EdgeVertex(me, 1);

            dist1 = ferVec3PointSegmentDist2(v, mv[0]->v, mv[1]->v, &q);
            if (ferEq(dist1, dist2)){
                mv[2] = ferMesh3EdgeVertex(&e1->m, 0);
                mv[3] = ferMesh3EdgeVertex(&e1->m, 1);
                if (ferVec3TriArea2(v, mv[0]->v, mv[1]->v)
                        > ferVec3TriArea2(v, mv[2]->v, mv[3]->v)){
                    dist2 = dist1;
                    e1 = fer_container_of(me, fer_chull3_edge_t, m);
                }
            }else if (dist1 < dist2){
                dist2 = dist1;
                e1 = fer_container_of(me, fer_chull3_edge_t, m);
            }
        }

        // 0. check area of created triangle
        mv[0] = ferMesh3EdgeVertex(&e1->m, 0);
        mv[1] = ferMesh3EdgeVertex(&e1->m, 1);
        if (ferIsZero(ferVec3TriArea2(v, mv[0]->v, mv[1]->v))){
            return;
        }

        // 1. create new vertex
        vert = vertNew(h, v);

        // 2. create edges connecting vertices
        mv[0] = ferMesh3EdgeVertex(&e1->m, 0);
        mv[1] = ferMesh3EdgeVertex(&e1->m, 1);
        v1 = fer_container_of(mv[0], fer_chull3_vert_t, m);
        v2 = fer_container_of(mv[1], fer_chull3_vert_t, m);
        e2 = edgeNew(h, vert, v1);
        e3 = edgeNew(h, vert, v2);

        // 3. create face
        f = faceNew(h, e1, e2, e3);
        {
            printf("---\nName: addCoplanar\nFace color: 1 0 0\nPoints:\n");
            ferVec3Print(&vert->v, stdout); printf("\n");
            ferVec3Print(&v1->v, stdout); printf("\n");
            ferVec3Print(&v2->v, stdout); printf("\n");
            printf("Faces:\n0 1 2\n");
            printf("---\n");
        }
        fprintf(stderr, "faceNew: %lx area: %.30f, %.30f\n",
                (long)f, ferVec3TriArea2(&vert->v, &v1->v, &v2->v),
                FER_EPS);

        // 4. get other face that newly created one
        mf = ferMesh3EdgeOtherFace(&e1->m, &f->m);
        f2 = fer_container_of(mf, fer_chull3_face_t, m);

        // 5. orient vertices in new face
        ferMesh3FaceVertices(mf, mv);
        faceSetVertices(h, f, vert, v1, v2);
        inv = 1;
        if ((f2->v[0] == v1 && f2->v[1] == v2)
                || (f2->v[1] == v1 && f2->v[2] == v2)
                || (f2->v[2] == v1 && f2->v[0] == v2)){
            DBG2("inv");
            faceSetVertices(h, f, vert, v2, v1);
            inv = 0;
        }

        ferVec3Sub2(&p01, &v1->v, &vert->v);
        ferVec3Sub2(&p02, &v2->v, &vert->v);
        ferVec3Cross(&cross, &p01, &p02);
        ferVec3Normalize(&cross);
        //printf("---\nPoint size: 10\nPoints:\n");
        //ferVec3Print(&topv, stdout);printf("\n---\n");

        ferVec3Add2(&topv, &vert->v, &cross);
        addCoplanarGrow(h, vert, e2, &topv, inv);

        ferVec3Scale(&cross, -FER_ONE);
        ferVec3Add2(&topv, &vert->v, &cross);
        addCoplanarGrow(h, vert, e3, &topv, !inv);
    }
}

static void firstNonCoplanar(fer_chull3_t *h, const fer_vec3_t *point)
{
    fer_list_t *list, *item;
    fer_list_t border_edges;
    fer_mesh3_face_t *mf;
    fer_mesh3_edge_t *me;
    fer_mesh3_vertex_t *mv;
    fer_chull3_face_t *f;
    fer_chull3_edge_t *e[3];
    fer_chull3_vert_t *v0, *v1;
    fer_real_t orient;
    int reorient, i;

    // first find out if we need to reorient faces
    reorient = 0;
    list = ferMesh3Faces(h->mesh);
    FER_LIST_FOR_EACH(list, item){
        mf = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
        f  = fer_container_of(mf, fer_chull3_face_t, m);

        orient = ferVec3Volume6(&f->v[0]->v, &f->v[1]->v, &f->v[2]->v, point);
        orient = orient / FER_REAL(6.);
        //DBG("orient: %f", orient);
        if (!ferIsZero(orient) && orient > FER_ZERO){
            reorient = 1;
            break;
        }
    }


    DBG("reorient: %d", reorient);
    if (reorient){
        list = ferMesh3Faces(h->mesh);
        FER_LIST_FOR_EACH(list, item){
            mf = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
            f  = fer_container_of(mf, fer_chull3_face_t, m);

            faceSetVertices(h, f, f->v[0], f->v[2], f->v[1]);
            //orient = ferVec3Volume6(&f->v[0]->v, &f->v[1]->v, &f->v[2]->v, point);
            //DBG("orient: %f", orient);
        }
    }

    ferListInit(&border_edges);
    list = ferMesh3Faces(h->mesh);
    FER_LIST_FOR_EACH(list, item){
        mf = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
        f  = fer_container_of(mf, fer_chull3_face_t, m);

        // obtain border edges
        for (i = 0; i < 3; i++){
            me   = ferMesh3FaceEdge(&f->m, i);
            e[i] = fer_container_of(me, fer_chull3_edge_t, m);
        }

        // append edges that incidents with non-visible face to list of
        // border faces
        for (i = 0; i < 3; i++){
            //DBG("i: %d", i);
            if (ferMesh3EdgeFacesLen(&e[i]->m) == 1){
                // orient edge correctly
                mv = ferMesh3EdgeVertex(&e[i]->m, 0);
                v0 = fer_container_of(mv, fer_chull3_vert_t, m);
                mv = ferMesh3EdgeVertex(&e[i]->m, 1);
                v1 = fer_container_of(mv, fer_chull3_vert_t, m);

                e[i]->swap = 0;
                if ((v0 == f->v[0] && v1 == f->v[1])
                        || (v0 == f->v[1] && v1 == f->v[2])
                        || (v0 == f->v[2] && v1 == f->v[0])){
                    e[i]->swap = 1;
                }

                // append it to list of edges
                ferListAppend(&border_edges, &e[i]->list);
            }
        }
    }

    makeCone(h, &border_edges, point);
}

static int __c = 0;

static void findVisibleFacesCoplanar(fer_chull3_t *h, const fer_vec3_t *v,
                                     fer_list_t *visible)
{
/*
    fer_list_t *list, *item;
    fer_mesh3_face_t *mf;
    fer_mesh3_vertex_t *mv[3];
    fer_mesh3_edge_t *me;
    fer_chull3_face_t *f;
    fer_chull3_edge_t *e[2], *tmpe;
    fer_real_t edist[2];
    fer_chull3_edge_t *newe[2];
    fer_chull3_vertex_t *newv, *v[2];
    fer_vec3_t w;
    fer_real_t val, dist, fdist, edist;
    int notin, d, i;

    f = NULL;
    list = ferMesh3Faces(h->mesh);
    notin = 0;
    FER_LIST_FOR_EACH(list, item){
        mf = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
        ferMesh3FaceVertices(mf, mv);

        if (!ferVec3PointInTri(v, mv[0]->v, mv[1]->v, mv[2]->v)){
            notin = 1;
            break;
        }
    }

    if (!notin)
        return;


    list = ferMesh3Edges(h->mesh);
    edist[0] = edist[1] = FER_REAL_MAX;
    FER_LIST_FOR_EACH(list, item){
        me = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
        mv[0] = ferMesh3EdgeVertex(me, 0);
        mv[1] = ferMesh3EdgeVertex(me, 1);

        dist = ferVec3PointSegmentDist2(v, mv[0]->v, mv[1]->v, NULL);
        if (dist < edist[1]){
            edist[1] = dist;
            e[1] = fer_container_of(me, fer_chull3_edge_t, m);

            if (edist[1] < edist[0]){
                dist = edist[1];
                edist[1] = edist[0];
                edist[0] = dist;

                tmpe = e[1];
                e[1] = e[0];
                e[0] = tmpe;
            }
        }
    }

    newv = vertNew(h, v);

    mv[0] = ferMesh3EdgeVertex(e[0], 0);
    mv[1] = ferMesh3EdgeVertex(e[0], 1);
    v[0] = fer_container_of(mv[0], fer_chull3_vert_t, m);
    v[1] = fer_container_of(mv[1], fer_chull3_vert_t, m);
    newe[0] = edgeNew(h, newv, v[0]);
    newe[1] = edgeNew(h, newv, v[0]);
    f = faceNew(

            if (!f){
                f = fer_container_of(mf, fer_chull3_face_t, m);
                fdist = ferVec3PointTriDist2(v, mv[0]->v, mv[1]->v, mv[2]->v, NULL);
            }else{
                dist = ferVec3PointTriDist2(v, mv[0]->v, mv[1]->v, mv[2]->v, NULL);
                if (dist < fdist){
                    f = fer_container_of(mf, fer_chull3_face_t, m);
                    fdist = dist;
                }
            }
        }
    }

    if (f){
        e = NULL;
        edist = FER_REAL_MAX;
        for (i = 0; i < 3; i++){
            me = ferMesh3FaceEdge(&f->m, 0);
            mv[0] = ferMesh3EdgeVertex(me, 0);
            mv[1] = ferMesh3EdgeVertex(me, 1);
            dist = ferVec3PointSegmentDist2(v, mv[0]->v, mv[1]->v, NULL);
            if (dist < edist){
                edist = dist;
                e = fer_container_of(me, fer_chull3_edge_t, m);
            }
        }
        ferListAppend(visible, &f->list);
    }
*/

    /*
    ferVec3Copy(&w, v);
    d = ferRand(&h->rand, 0, 3);
    val = ferRand(&h->rand, -.1, .1);
    ferVec3SetCoord(&w, d, ferVec3Get(&w, d) + val);

    findVisibleFaces(h, &w, visible);
    */
}

static void findVisibleFaces(fer_chull3_t *h, const fer_vec3_t *v,
                             fer_list_t *visible)
{
    fer_list_t *list, *item, coplanar, faces;
    fer_mesh3_face_t *mf;
    fer_mesh3_edge_t *me;
    fer_chull3_face_t *f, *f2, *fstart;
    fer_real_t orient, orient_start;
    size_t num_overall, num_coplanar, i;

    __c++;
    DBG("__c: %d", __c);

    //if (__c >= 3600 && __c <= 3678){
        {
        char name[120];
        sprintf(name, "h - %d - %lx", __c, (long)h);
        ferCHull3DumpSVT(h, stdout, name);
        printf("---\nPoint color: 0.8 0 0\nPoint size: 10\nPoints:\n");
        ferVec3Print(v, stdout);
        printf("\n---\n");
    }


    ferListInit(&coplanar);
    num_overall = num_coplanar = 0;

    //DBG_VEC3(v, "v: ");

    // find first visible face and mark all other faces if they are visible
    // or coplanar
    fstart = NULL;
    orient_start = -1;
    list = ferMesh3Faces(h->mesh);
    FER_LIST_FOR_EACH(list, item){
        mf = FER_LIST_ENTRY(item, fer_mesh3_face_t, list);
        f  = fer_container_of(mf, fer_chull3_face_t, m);

        f->visible = 0;

        // mark all visible (and coplanar) faces as visible
        orient = ferVec3Volume6(&f->v[0]->v, &f->v[1]->v, &f->v[2]->v, v);
        DBG("orient1: %.30f", orient);
        orient = orient / FER_REAL(6.);
        DBG("orient2: %.30f", orient);
        orient = ferPredOrient3d(&f->v[0]->v, &f->v[1]->v, &f->v[2]->v, v);
        DBG("orient3: %.30f", orient);
        orient = ferPredOrient3dFast(&f->v[0]->v, &f->v[1]->v, &f->v[2]->v, v);
        DBG("orient4: %.30f", orient);
        if (!ferIsZero(orient) && orient > FER_ZERO){
            DBG2("visible");
            f->visible = 2;

            if (orient > orient_start){
                orient_start = orient;
                fstart = f;
            }

            {
            printf("---\nFace color: 0.8 0.8 0\nPoints:\n");
            ferVec3Print(&f->v[0]->v, stdout); printf("\n");
            ferVec3Print(&f->v[1]->v, stdout); printf("\n");
            ferVec3Print(&f->v[2]->v, stdout); printf("\n");
            printf("Faces:\n0 1 2\n---\n");
            }
        }
        /*

        if (ferIsZero(orient)){
            if (!ferVec3Eq(v, &f->v[0]->v)
                    || !ferVec3Eq(v, &f->v[1]->v)
                    || !ferVec3Eq(v, &f->v[2]->v)){
                num_coplanar++;
            }
        }else if (orient > h->eps){
            f->visible = 2;

            if (orient > orient_start){
                orient_start = orient;
                fstart = f;
            }
        }else if (orient > -h->eps && orient < h->eps){
            if (!ferVec3Eq(v, &f->v[0]->v)
                    || !ferVec3Eq(v, &f->v[1]->v)
                    || !ferVec3Eq(v, &f->v[2]->v)){
                num_coplanar++;
            }
        }

        num_overall++;
        */
    }

    /*
    //DBG("num_overall, num_coplanar %d %d", num_overall, num_coplanar);
    if (num_overall == num_coplanar){
        // degenerate case - all faces are coplanar with the point
        ferListInit(visible);
        findVisibleFacesCoplanar(h, v, visible);
        return;
    }
    */

    if (!fstart)
        return;

    // grow visible patch from the initial visible face
    ferListInit(&faces);
    ferListAppend(&faces, &fstart->list);
    fstart->visible = 1;

    while (!ferListEmpty(&faces)){
        item = ferListNext(&faces);
        ferListDel(item);
        f = FER_LIST_ENTRY(item, fer_chull3_face_t, list);

        ferListAppend(visible, &f->list);

        for (i = 0; i < 3; i++){
            me = ferMesh3FaceEdge(&f->m, i);
            mf = ferMesh3EdgeOtherFace(me, &f->m);

            // this shouldn't happen...
            if (!mf)
                continue;

            f2 = fer_container_of(mf, fer_chull3_face_t, m);
            if (f2->visible == 2){
                ferListAppend(&faces, &f2->list);
                f2->visible = 1;
            }
        }

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

                    e[i]->swap = 0;
                    if ((v0 == f2->v[0] && v1 == f2->v[1])
                            || (v0 == f2->v[1] && v1 == f2->v[2])
                            || (v0 == f2->v[2] && v1 == f2->v[0])){
                        e[i]->swap = 1;
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
        if (e1->swap){
            faceSetVertices(h, f, v1, v3, v2);
        }else{
            faceSetVertices(h, f, v1, v2, v3);
        }
    }
}
