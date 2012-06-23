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

#ifndef __BOR_MESH3_H__
#define __BOR_MESH3_H__

#include <stdio.h>
#include <boruvka/core.h>
#include <boruvka/vec3.h>
#include <boruvka/list.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _bor_mesh3_vertex_t;
struct _bor_mesh3_edge_t;
struct _bor_mesh3_face_t;


/**
 * Mesh3 - Representation of 3D mesh
 * ==================================
 *
 * This is implementation of simple 2-minfold mesh representation.
 *
 * A mesh consists of vertices, edges and faces. In this implementation
 * each element (vertex, edge and face) is stored explicitly (no implicit
 * faces, edges or vertices). In each vertex is stored list of all
 * incidenting edges, in each edge is stored start and end vertex and
 * (maximally) two incideting faces, each face holds triplet of bounding
 * edges. It can be illustrated:
 * ~~~~
 *     vertex <--> edge <--> face.
 * ~~~~
 *
 * Vertex, edge and face corresponds with structs
 * bor_mesh3_{vertex,edge,face}_t respectively. Structs don't have to be
 * initialized before use - functions borMesh3Add{Vertex,Edge,Face}()
 * initialize them when adding to mesh. Structs can be allocated on both
 * stack or heap and doesn't need any alignment. Vertices are designed hold
 * only pointer to *user-defined* bor_vec3_t vector representing
 * coordinates. Mesh3 takes care only of connection between vertices, edges
 * and faces, the rest is user's responsibility.
 *
 * Most likely, user will define its own structures and bor_mesh3_*
 * structs just embeds into to, for example:
 * ~~~~~
 *   struct my_vec_t {
 *       bor_vec3_t vec;       // Coordinates of vertex
 *       bor_mesh3_vertex_t v; // Vertex
 *   };
 *   ....
 *   struct my_vec_t v;
 *   // set vertex coordinates
 *   borMesh3VertexSetCoordinates(&v.v, &v.vec);
 *   ...
 * ~~~~~
 *
 * NOTE: Don't access any struct's member directly instead use provided
 * inline functions.
 */

struct _bor_mesh3_t {
    bor_list_t verts; /*!< List of vertices */
    size_t verts_len; /*!< Number of vertices in list */
    bor_list_t edges; /*!< List of edges */
    size_t edges_len; /*!< Number of edges in list */
    bor_list_t faces; /*!< List of faces */
    size_t faces_len; /*!< Number of faces in list */
};
typedef struct _bor_mesh3_t bor_mesh3_t;


struct _bor_mesh3_vertex_t {
    const bor_vec3_t *v; /*!< User provided coordinates of vertex */
    bor_list_t list;     /*!< Connection into list of all vertices */

    bor_list_t edges; /*!< List of all incidenting edges */
    size_t edges_len; /*!< Number of edges in list */

    int _id; /*!< This is currently used only for borMesh3DumpSVT() */
};
typedef struct _bor_mesh3_vertex_t bor_mesh3_vertex_t;


struct _bor_mesh3_edge_t {
    bor_mesh3_vertex_t *v[2];       /*!< Start and end point of edge */
    struct _bor_mesh3_face_t *f[2]; /*!< Incidenting faces */

    bor_list_t list;       /*!< Connection into list of all edges */
    bor_list_m_t vlist[2]; /*!< Connection into list of edges incidenting
                                with vertex.
                                .vlist[0] correspond with vertex .v[0] and
                                .vlist[1] with .v[1] */
};
typedef struct _bor_mesh3_edge_t bor_mesh3_edge_t;


struct _bor_mesh3_face_t {
    bor_mesh3_edge_t *e[3]; /*!< Incidenting edges */
    bor_list_t list;        /*!< Connection into list of all faces */
};
typedef struct _bor_mesh3_face_t bor_mesh3_face_t;


/**
 * Vertex
 * -------
 *
 * Vertex holds list of incidenting edges and *user-defined* vec3
 * coordinates. See above for example how to provide vec3 to vertex.
 *
 * Vertex don't have to be initialized because borMesh3AddVertex()
 * initializes it when it's added to mesh.
 * Note that since there is no initialization function it is not safe to
 * use any borMesh3Vertex*() function until it's added to mesh.
 * Note also that nothing ever touches bor_vec3_t vector stored (as
 * pointer) in vertex. It is safe anytime (even if it is not added to mesh)
 * to get or set coords - borMesh3Vertex{,Set}Coords{,W}().
 *
 * See bor_mesh3_vertex_t.
 */

/**
 * Allocates and initializes new vertex.
 */
bor_mesh3_vertex_t *borMesh3VertexNew(void);

/**
 * Deletes vertex.
 */
void borMesh3VertexDel(bor_mesh3_vertex_t *v);


/**
 * Sets pointer to user-defined vec3 coordinates.
 */
_bor_inline void borMesh3VertexSetCoords(bor_mesh3_vertex_t *v,
                                         const bor_vec3_t *coords);

/**
 * Returns pointer to vector representing coordinates of vertex.
 */
_bor_inline const bor_vec3_t *borMesh3VertexCoords(bor_mesh3_vertex_t *v);

/**
 * Returns writeable pointer to vec3 coordinates.
 */
_bor_inline bor_vec3_t *borMesh3VertexCoordsW(bor_mesh3_vertex_t *v);

/**
 * Returns number of edges incidenting with given vertex.
 */
_bor_inline size_t borMesh3VertexEdgesLen(const bor_mesh3_vertex_t *v);

/**
 * Returns head of list of edges incidenting with vertex.
 */
_bor_inline bor_list_t *borMesh3VertexEdges(bor_mesh3_vertex_t *v);

/**
 * Returns true if vertex incidents with given edge.
 */
_bor_inline int borMesh3VertexHasEdge(const bor_mesh3_vertex_t *v,
                                      const bor_mesh3_edge_t *e);

/**
 * Returns edge (first) connecting given pair of vertices.
 */
bor_mesh3_edge_t *borMesh3VertexCommonEdge(const bor_mesh3_vertex_t *v1,
                                           const bor_mesh3_vertex_t *v2);


/**
 * Edge
 * -----
 * 
 * Edge doesn't have to be initialized explicitly - borMesh3AddEdge()
 * initialize it when added to mesh. Note that since there is no
 * initialization function it is not safe to use any borMesh3Edge*()
 * function until it's added to mesh.
 *
 * See bor_mesh3_edge_t.
 */

/**
 * Allocates and initializes edge.
 */
bor_mesh3_edge_t *borMesh3EdgeNew(void);

/**
 * Deletes edge.
 */
void borMesh3EdgeDel(bor_mesh3_edge_t *e);

/**
 * Returns start or end point of edge.
 * Parameter i can be either 0 or 1 (no check is performed).
 */
_bor_inline bor_mesh3_vertex_t *borMesh3EdgeVertex(bor_mesh3_edge_t *e, size_t i);

/**
 * Returns incidenting face.
 * In 2-manifold mesh, edge can have maximally two incidenting faces, so i
 * can be 0 or 1.
 * No checking of i is performed!
 */
_bor_inline bor_mesh3_face_t *borMesh3EdgeFace(bor_mesh3_edge_t *e, size_t i);

/**
 * Returns the other face than given.
 */
_bor_inline bor_mesh3_face_t *borMesh3EdgeOtherFace(bor_mesh3_edge_t *e,
                                                    bor_mesh3_face_t *f);

/**
 * Returns true if edge incidents with given vertex.
 */
_bor_inline int borMesh3EdgeHasVertex(const bor_mesh3_edge_t *e,
                                      const bor_mesh3_vertex_t *v);

/**
 * Returns true if edge incidents with given face.
 */
_bor_inline int borMesh3EdgeHasFace(const bor_mesh3_edge_t *e,
                                    const bor_mesh3_face_t *f);

/**
 * Returns number of incidenting faces.
 */
_bor_inline size_t borMesh3EdgeFacesLen(const bor_mesh3_edge_t *e);


/**
 * Returns the other (start/end) vertex than provided.
 */
_bor_inline bor_mesh3_vertex_t *borMesh3EdgeOtherVertex(bor_mesh3_edge_t *e,
                                                        const bor_mesh3_vertex_t *v);

/**
 * Returns true if given triplet of edges form triangle.
 */
int borMesh3EdgeTriCheck(const bor_mesh3_edge_t *e1,
                         const bor_mesh3_edge_t *e2,
                         const bor_mesh3_edge_t *e3);

/**
 * Return pointer of edge struct based on list item (pointer to vlist[0|1].
 */
_bor_inline bor_mesh3_edge_t *borMesh3EdgeFromVertexList(bor_list_t *l);


/**
 * Swaps end vertices of edge.
 */
_bor_inline void borMesh3EdgeSwapVertices(bor_mesh3_edge_t *e);


/**
 * Face
 * -----
 *
 * Same as with edges - faces don't have to be initialized.
 *
 * See bor_mesh3_face_t.
 */

/**
 * Allocates and initializes new face.
 */
bor_mesh3_face_t *borMesh3FaceNew(void);

/**
 * Deletes face.
 */
void borMesh3FaceDel(bor_mesh3_face_t *f);

/**
 * Returns incidenting edge.
 * Parameter i can be 0 or 1 or 2 since face has always exactly three
 * incidenting edges.
 * No check of parameters is performed.
 */
_bor_inline bor_mesh3_edge_t *borMesh3FaceEdge(bor_mesh3_face_t *f, size_t i);

/**
 * Returns true if face incidents with given edge.
 */
_bor_inline int borMesh3FaceHasEdge(const bor_mesh3_face_t *f,
                                    const bor_mesh3_edge_t *e);

/**
 * Returns true if face incidents with given vertex.
 */
_bor_inline int borMesh3FaceHasVertex(const bor_mesh3_face_t *f,
                                      const bor_mesh3_vertex_t *v);

/**
 * Retuns three incidenting vertices (via vs argument).
 * Note that ns must be preallocated array of size 3.
 */
_bor_inline void borMesh3FaceVertices(bor_mesh3_face_t *f,
                                      bor_mesh3_vertex_t **vs);

/**
 * Returns twice an area of a face.
 */
_bor_inline bor_real_t borMesh3FaceArea2(const bor_mesh3_face_t *f);

/**
 * Returns the last vertex bounding the face.
 */
_bor_inline bor_mesh3_vertex_t *borMesh3FaceOtherVertex(bor_mesh3_face_t *f,
                                                        bor_mesh3_vertex_t *v1,
                                                        bor_mesh3_vertex_t *v2);

/**
 * Mesh
 * -----
 *
 * Mesh must be allocated on heap - no static declaration is allowed - use
 * borMesh3New() function for allocation and borMesh3Del() for
 * deallocation. If you need to deallocate also vertices, edges or faces
 * along with deallocation of whole mesh, use borMesh3Del2() function that
 * allows you to provide callback for deallocation of elements.
 *
 * For iteration over vertices use this approach:
 * ~~~~
 *   // note: mesh hold pointer to bor_mesh3_t
 *   bor_list_t *item;
 *   bor_mesh3_vertex_t *v;
 *   BOR_LIST_FOR_EACH(borMesh3Vertices(mesh), item) {
 *       v = BOR_LIST_ENTRY(item, bor_mesh3_vertex_t, list);
 *       // here you have vertex stored in v
 *       ...
 *   }
 * ~~~~
 *
 * Similar approach can be used for edges and faces, only different type is
 * used in BOR_LIST_ENTRY():
 * ~~~~
 *   BOR_LIST_ENTRY(item, bor_mesh3_{edge,face}_t, list);
 * ~~~~
 *
 * TODO: Maybe some macros could be provided for iteration over vertices,
 *       edges and faces.
 *
 *
 * See bor_mesh3_t.
 */

/**
 * Creates new empty mesh.
 */
bor_mesh3_t *borMesh3New(void);

/**
 * Deletes mesh.
 * Warning: No vertices, edges or faces are deleted because they were
 * allocated outside a mesh! See borMesh3Del2() if you want more
 * sofisticated destructor.
 */
void borMesh3Del(bor_mesh3_t *m);

/**
 * Deletes mesh. This destructor can used for deleting all vertices, edges
 * and faces contained in mesh.
 *
 * Before freeing a mesh, destructor iterates over all vertices. Each
 * vertex is first disconnected from mesh and then delvertex is called with
 * second argument vdata. Similarly are iterated edges and faces.
 */
void borMesh3Del2(bor_mesh3_t *m,
                  void (*delvertex)(bor_mesh3_vertex_t *, void *), void *vdata,
                  void (*deledge)(bor_mesh3_edge_t *, void *), void *edata,
                  void (*delface)(bor_mesh3_face_t *, void *), void *fdata);

/**
 * Returns number of vertices stored in mesh.
 */
_bor_inline size_t borMesh3VerticesLen(const bor_mesh3_t *m);

/**
 * Returns number of edges stored in mesh.
 */
_bor_inline size_t borMesh3EdgesLen(const bor_mesh3_t *m);

/**
 * Returns number of faces stored in mesh.
 */
_bor_inline size_t borMesh3FacesLen(const bor_mesh3_t *m);

/**
 * Returns list of vertices.
 */
_bor_inline bor_list_t *borMesh3Vertices(bor_mesh3_t *m);

/**
 * Returns list of edges.
 */
_bor_inline bor_list_t *borMesh3Edges(bor_mesh3_t *m);

/**
 * Returns list of faces.
 */
_bor_inline bor_list_t *borMesh3Faces(bor_mesh3_t *m);


/**
 * Adds vertex into mesh.
 */
void borMesh3AddVertex(bor_mesh3_t *m, bor_mesh3_vertex_t *v);

/**
 * Removes vertex into mesh.
 * If vertex is connected with any edge, vertex can't be removed and -1 is
 * returned. On success 0 is returned.
 */
int borMesh3RemoveVertex(bor_mesh3_t *m, bor_mesh3_vertex_t *v);

/**
 * Adds edge into mesh. Start and end points (vertices) must be provided.
 */
void borMesh3AddEdge(bor_mesh3_t *m, bor_mesh3_edge_t *e,
                     bor_mesh3_vertex_t *start, bor_mesh3_vertex_t *end);

/**
 * Removes edge from mesh.
 * If edge incidents with any face, edge can't be removed and -1 is
 * returned. On success 0 is returned.
 */
int borMesh3RemoveEdge(bor_mesh3_t *m, bor_mesh3_edge_t *e);

/**
 * Adds face into mesh. Three incidenting faces must be provided.
 * If any of given edge already incidents with two faces, face can't be
 * added to mesh and -1 is returned. On success is returned 0.
 *
 * The three provided edges should form triangle (by start and end points),
 * note that check if it is so is not performed! - see borMesh3EdgeTriCheck()
 */
int borMesh3AddFace(bor_mesh3_t *m, bor_mesh3_face_t *f,
                    bor_mesh3_edge_t *e1, bor_mesh3_edge_t *e2,
                    bor_mesh3_edge_t *e3);

/**
 * Removes face from mesh.
 * Note that this function can never fail.
 */
void borMesh3RemoveFace(bor_mesh3_t *m, bor_mesh3_face_t *f);

/**
 * Dumps mesh as one object in SVT format.
 * See http://svt.danfis.cz for more info.
 */
void borMesh3DumpSVT(bor_mesh3_t *m, FILE *out, const char *name);

/**
 * Dumps mesh as list of triangles.
 * One triangle per line (ax ay az bx by ...)
 */
void borMesh3DumpTriangles(bor_mesh3_t *m, FILE *out);

/**
 * Dumps mesh in povray format.
 */
void borMesh3DumpPovray(bor_mesh3_t *m, FILE *out);



/**** INLINES ****/
_bor_inline void borMesh3VertexSetCoords(bor_mesh3_vertex_t *v,
                                         const bor_vec3_t *coords)
{
    v->v = coords;
}

_bor_inline const bor_vec3_t *borMesh3VertexCoords(bor_mesh3_vertex_t *v)
{
    return v->v;
}

_bor_inline bor_vec3_t *borMesh3VertexCoordsW(bor_mesh3_vertex_t *v)
{
    return (bor_vec3_t *)v->v;
}

_bor_inline size_t borMesh3VertexEdgesLen(const bor_mesh3_vertex_t *v)
{
    return v->edges_len;
}

_bor_inline bor_list_t *borMesh3VertexEdges(bor_mesh3_vertex_t *v)
{
    return &v->edges;
}

_bor_inline int borMesh3VertexHasEdge(const bor_mesh3_vertex_t *v,
                                      const bor_mesh3_edge_t *e)
{
    bor_list_t *item;
    bor_list_m_t *mitem;
    bor_mesh3_edge_t *edge;

    BOR_LIST_FOR_EACH(&v->edges, item){
        mitem = borListMFromList(item);
        edge = BOR_LIST_M_ENTRY(item, bor_mesh3_edge_t, vlist, mitem->mark);
        if (edge == e)
            return 1;
    }

    return 0;
}





_bor_inline bor_mesh3_vertex_t *borMesh3EdgeVertex(bor_mesh3_edge_t *e, size_t i)
{
    return e->v[i];
}

_bor_inline bor_mesh3_face_t *borMesh3EdgeFace(bor_mesh3_edge_t *e, size_t i)
{
    return e->f[i];
}

_bor_inline bor_mesh3_face_t *borMesh3EdgeOtherFace(bor_mesh3_edge_t *e,
                                                    bor_mesh3_face_t *f)
{
    if (e->f[0] == f)
        return e->f[1];
    return e->f[0];
}

_bor_inline int borMesh3EdgeHasVertex(const bor_mesh3_edge_t *e,
                                      const bor_mesh3_vertex_t *v)
{
    return e->v[0] == v || e->v[1] == v;
}

_bor_inline int borMesh3EdgeHasFace(const bor_mesh3_edge_t *e,
                                    const bor_mesh3_face_t *f)
{
    return e->f[0] == f || e->f[1] == f;
}

_bor_inline size_t borMesh3EdgeFacesLen(const bor_mesh3_edge_t *e)
{
    if (e->f[0] == NULL){
        if (e->f[1] == NULL)
            return 0;
        return 1;
    }else if (e->f[1] == NULL){
        return 1;
    }else{
        return 2;
    }
}

_bor_inline bor_mesh3_vertex_t *borMesh3EdgeOtherVertex(bor_mesh3_edge_t *e,
                                                        const bor_mesh3_vertex_t *v)
{
    if (e->v[0] == v)
        return e->v[1];
    return e->v[0];
}

_bor_inline bor_mesh3_edge_t *borMesh3EdgeFromVertexList(bor_list_t *l)
{
    bor_list_m_t *m;
    bor_mesh3_edge_t *e;

    m = borListMFromList(l);
    e = BOR_LIST_M_ENTRY(l, bor_mesh3_edge_t, vlist, m->mark);

    return e;
}

_bor_inline void borMesh3EdgeSwapVertices(bor_mesh3_edge_t *e)
{
    bor_mesh3_vertex_t *tmp;

    tmp = e->v[0];
    e->v[0] = e->v[1];
    e->v[1] = tmp;
}



_bor_inline bor_mesh3_edge_t *borMesh3FaceEdge(bor_mesh3_face_t *f, size_t i)
{
    return f->e[i];
}

_bor_inline int borMesh3FaceHasEdge(const bor_mesh3_face_t *f,
                                    const bor_mesh3_edge_t *e)
{
    return f->e[0] == e
            || f->e[1] == e
            || f->e[2] == e;
}

_bor_inline int borMesh3FaceHasVertex(const bor_mesh3_face_t *f,
                                      const bor_mesh3_vertex_t *v)
{
    bor_mesh3_vertex_t *vs[3];
    borMesh3FaceVertices((bor_mesh3_face_t *)f, vs);
    return vs[0] == v || vs[1] == v || vs[2] == v;
}

_bor_inline void borMesh3FaceVertices(bor_mesh3_face_t *f,
                                      bor_mesh3_vertex_t **vs)
{
    bor_mesh3_vertex_t *n;

    vs[0] = borMesh3EdgeVertex(f->e[0], 0);
    vs[1] = borMesh3EdgeVertex(f->e[0], 1);

    n = borMesh3EdgeVertex(f->e[1], 0);
    if (n == vs[0] || n == vs[1]){
        vs[2] = borMesh3EdgeVertex(f->e[1], 1);
    }else{
        vs[2] = borMesh3EdgeVertex(f->e[1], 0);
    }
}

_bor_inline bor_real_t borMesh3FaceArea2(const bor_mesh3_face_t *f)
{
    bor_mesh3_vertex_t *vs[3];
    borMesh3FaceVertices((bor_mesh3_face_t *)f, vs);
    return borVec3TriArea2(vs[0]->v, vs[1]->v, vs[2]->v);
}

_bor_inline bor_mesh3_vertex_t *borMesh3FaceOtherVertex(bor_mesh3_face_t *f,
                                                        bor_mesh3_vertex_t *v1,
                                                        bor_mesh3_vertex_t *v2)
{
    bor_mesh3_vertex_t *vs[3];

    borMesh3FaceVertices((bor_mesh3_face_t *)f, vs);

    if (vs[0] != v1 && vs[0] != v2)
        return vs[0];
    if (vs[1] != v1 && vs[1] != v2)
        return vs[1];
    return vs[2];
}



_bor_inline size_t borMesh3VerticesLen(const bor_mesh3_t *m)
{
    return m->verts_len;
}

_bor_inline size_t borMesh3EdgesLen(const bor_mesh3_t *m)
{
    return m->edges_len;
}

_bor_inline size_t borMesh3FacesLen(const bor_mesh3_t *m)
{
    return m->faces_len;
}

_bor_inline bor_list_t *borMesh3Vertices(bor_mesh3_t *m)
{
    return &m->verts;
}

_bor_inline bor_list_t *borMesh3Edges(bor_mesh3_t *m)
{
    return &m->edges;
}

_bor_inline bor_list_t *borMesh3Faces(bor_mesh3_t *m)
{
    return &m->faces;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_MESH3_H__ */

