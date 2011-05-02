/***
 * libccd
 * ---------------------------------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *
 *  This file is part of libccd.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __FER_POLYTOPE_H__
#define __FER_POLYTOPE_H__

#include <fermat/ccd-support.h>
#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define FER_PT_VERTEX 1
#define FER_PT_EDGE   2
#define FER_PT_FACE   3


#define __FER_PT_EL \
    int type;           /*! type of element */ \
    fer_real_t dist;        /*! distance from origin */ \
    fer_vec3_t witness; /*! witness point of projection of origin */ \
    fer_list_t list;    /*! list of elements of same type */

/**
 * General polytope element.
 * Could be vertex, edge or triangle.
 */
struct _fer_pt_el_t {
    __FER_PT_EL
};
typedef struct _fer_pt_el_t fer_pt_el_t;

struct _fer_pt_edge_t;
struct _fer_pt_face_t;

/**
 * Polytope's vertex.
 */
struct _fer_pt_vertex_t {
    __FER_PT_EL

    int id;
    fer_support_t v;
    fer_list_t edges; //!< List of edges
};
typedef struct _fer_pt_vertex_t fer_pt_vertex_t;

/**
 * Polytope's edge.
 */
struct _fer_pt_edge_t {
    __FER_PT_EL

    fer_pt_vertex_t *vertex[2]; //!< Reference to vertices
    struct _fer_pt_face_t *faces[2]; //!< Reference to faces

    fer_list_t vertex_list[2]; //!< List items in vertices' lists
};
typedef struct _fer_pt_edge_t fer_pt_edge_t;

/**
 * Polytope's triangle faces.
 */
struct _fer_pt_face_t {
    __FER_PT_EL

    fer_pt_edge_t *edge[3]; //!< Reference to surrounding edges
};
typedef struct _fer_pt_face_t fer_pt_face_t;


/**
 * Struct containing polytope.
 */
struct _fer_pt_t {
    fer_list_t vertices; //!< List of vertices
    fer_list_t edges; //!< List of edges
    fer_list_t faces; //!< List of faces

    fer_pt_el_t *nearest;
    fer_real_t nearest_dist;
    int nearest_type;
};
typedef struct _fer_pt_t fer_pt_t;


void ferPtInit(fer_pt_t *pt);
void ferPtDestroy(fer_pt_t *pt);

/**
 * Returns vertices surrounding given triangle face.
 */
_fer_inline void ferPtFaceVec3(const fer_pt_face_t *face,
                               fer_vec3_t **a,
                               fer_vec3_t **b,
                               fer_vec3_t **c);
_fer_inline void ferPtFaceVertices(const fer_pt_face_t *face,
                                   fer_pt_vertex_t **a,
                                   fer_pt_vertex_t **b,
                                   fer_pt_vertex_t **c);
_fer_inline void ferPtFaceEdges(const fer_pt_face_t *f,
                                fer_pt_edge_t **a,
                                fer_pt_edge_t **b,
                                fer_pt_edge_t **c);

_fer_inline void ferPtEdgeVec3(const fer_pt_edge_t *e,
                               fer_vec3_t **a,
                               fer_vec3_t **b);
_fer_inline void ferPtEdgeVertices(const fer_pt_edge_t *e,
                                   fer_pt_vertex_t **a,
                                   fer_pt_vertex_t **b);
_fer_inline void ferPtEdgeFaces(const fer_pt_edge_t *e,
                                fer_pt_face_t **f1,
                                fer_pt_face_t **f2);


/**
 * Adds vertex to polytope and returns pointer to newly created vertex.
 */
fer_pt_vertex_t *ferPtAddVertex(fer_pt_t *pt, const fer_support_t *v);
_fer_inline fer_pt_vertex_t *ferPtAddVertexCoords(fer_pt_t *pt,
                                                  fer_real_t x, fer_real_t y, fer_real_t z);

/**
 * Adds edge to polytope.
 */
fer_pt_edge_t *ferPtAddEdge(fer_pt_t *pt, fer_pt_vertex_t *v1,
                                          fer_pt_vertex_t *v2);

/**
 * Adds face to polytope.
 */
fer_pt_face_t *ferPtAddFace(fer_pt_t *pt, fer_pt_edge_t *e1,
                                          fer_pt_edge_t *e2,
                                          fer_pt_edge_t *e3);

/**
 * Deletes vertex from polytope.
 * Returns 0 on success, -1 otherwise.
 */
_fer_inline int ferPtDelVertex(fer_pt_t *pt, fer_pt_vertex_t *);
_fer_inline int ferPtDelEdge(fer_pt_t *pt, fer_pt_edge_t *);
_fer_inline int ferPtDelFace(fer_pt_t *pt, fer_pt_face_t *);


/**
 * Recompute distances from origin for all elements in pt.
 */
void ferPtRecomputeDistances(fer_pt_t *pt);

/**
 * Returns nearest element to origin.
 */
fer_pt_el_t *ferPtNearest(fer_pt_t *pt);


void ferPtDumpSVT(fer_pt_t *pt, const char *fn);
void ferPtDumpSVT2(fer_pt_t *pt, FILE *);


/**** INLINES ****/
_fer_inline fer_pt_vertex_t *ferPtAddVertexCoords(fer_pt_t *pt,
                                                  fer_real_t x, fer_real_t y, fer_real_t z)
{
    fer_support_t s;
    ferVec3Set(&s.v, x, y, z);
    return ferPtAddVertex(pt, &s);
}

_fer_inline int ferPtDelVertex(fer_pt_t *pt, fer_pt_vertex_t *v)
{
    // test if any edge is connected to this vertex
    if (!ferListEmpty(&v->edges))
        return -1;

    // delete vertex from main list
    ferListDel(&v->list);

    if ((void *)pt->nearest == (void *)v){
        pt->nearest = NULL;
    }

    free(v);
    return 0;
}

_fer_inline int ferPtDelEdge(fer_pt_t *pt, fer_pt_edge_t *e)
{
    // text if any face is connected to this edge (faces[] is always
    // aligned to lower indices)
    if (e->faces[0] != NULL)
        return -1;

    // disconnect edge from lists of edges in vertex struct
    ferListDel(&e->vertex_list[0]);
    ferListDel(&e->vertex_list[1]);

    // disconnect edge from main list
    ferListDel(&e->list);

    if ((void *)pt->nearest == (void *)e){
        pt->nearest = NULL;
    }

    free(e);
    return 0;
}

_fer_inline int ferPtDelFace(fer_pt_t *pt, fer_pt_face_t *f)
{
    fer_pt_edge_t *e;
    size_t i;

    // remove face from edges' recerence lists
    for (i = 0; i < 3; i++){
        e = f->edge[i];
        if (e->faces[0] == f){
            e->faces[0] = e->faces[1];
        }
        e->faces[1] = NULL;
    }

    // remove face from list of all faces
    ferListDel(&f->list);

    if ((void *)pt->nearest == (void *)f){
        pt->nearest = NULL;
    }

    free(f);
    return 0;
}

_fer_inline void ferPtFaceVec3(const fer_pt_face_t *face,
                               fer_vec3_t **a,
                               fer_vec3_t **b,
                               fer_vec3_t **c)
{
    *a = &face->edge[0]->vertex[0]->v.v;
    *b = &face->edge[0]->vertex[1]->v.v;

    if (face->edge[1]->vertex[0] != face->edge[0]->vertex[0]
            && face->edge[1]->vertex[0] != face->edge[0]->vertex[1]){
        *c = &face->edge[1]->vertex[0]->v.v;
    }else{
        *c = &face->edge[1]->vertex[1]->v.v;
    }
}

_fer_inline void ferPtFaceVertices(const fer_pt_face_t *face,
                                   fer_pt_vertex_t **a,
                                   fer_pt_vertex_t **b,
                                   fer_pt_vertex_t **c)
{
    *a = face->edge[0]->vertex[0];
    *b = face->edge[0]->vertex[1];

    if (face->edge[1]->vertex[0] != face->edge[0]->vertex[0]
            && face->edge[1]->vertex[0] != face->edge[0]->vertex[1]){
        *c = face->edge[1]->vertex[0];
    }else{
        *c = face->edge[1]->vertex[1];
    }
}

_fer_inline void ferPtFaceEdges(const fer_pt_face_t *f,
                                fer_pt_edge_t **a,
                                fer_pt_edge_t **b,
                                fer_pt_edge_t **c)
{
    *a = f->edge[0];
    *b = f->edge[1];
    *c = f->edge[2];
}

_fer_inline void ferPtEdgeVec3(const fer_pt_edge_t *e,
                               fer_vec3_t **a,
                               fer_vec3_t **b)
{
    *a = &e->vertex[0]->v.v;
    *b = &e->vertex[1]->v.v;
}

_fer_inline void ferPtEdgeVertices(const fer_pt_edge_t *e,
                                   fer_pt_vertex_t **a,
                                   fer_pt_vertex_t **b)
{
    *a = e->vertex[0];
    *b = e->vertex[1];
}

_fer_inline void ferPtEdgeFaces(const fer_pt_edge_t *e,
                                fer_pt_face_t **f1,
                                fer_pt_face_t **f2)
{
    *f1 = e->faces[0];
    *f2 = e->faces[1];
}


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_POLYTOPE_H__ */
