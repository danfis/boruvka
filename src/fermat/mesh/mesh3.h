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

#ifndef __FER_MESH3_H__
#define __FER_MESH3_H__

#include <stdio.h>
#include <fermat/core.h>
#include <fermat/vec3.h>
#include <fermat/list.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_mesh3_vertex_t;
struct _fer_mesh3_edge_t;
struct _fer_mesh3_face_t;


/**
 * Mesh3 - Representation of 3D mesh
 * ----------------------------------
 *
 * This is implementation of simple 2-minfold mesh representation.
 *
 * A mesh consists of vertices, edges and faces. In this implementation
 * each element (vertex, edge and face) is stored explicitly (no implicit
 * faces, edges or vertices). In each vertex is stored list of all
 * incidenting edges, in each edge is stored start and end vertex and
 * (maximally) two incideting faces, each face holds triplet of bounding
 * edges. It can be illustrated:
 * ----
 *      vertex <--> edge <--> face.
 * ----
 *
 * Vertex, edge and face corresponds with structs
 * fer_mesh3_{vertex,edge,face}_t respectively. Structs don't have to be
 * initialized before use - functions ferMesh3Add{Vertex,Edge,Face}()
 * initialize them when adding to mesh. Structs can be allocated on both
 * stack or heap and doesn't need any alignment. Vertices are designed hold
 * only pointer to *user-defined* fer_vec3_t vector representing
 * coordinates. Mesh3 takes care only of connection between vertices, edges
 * and faces, the rest is user's responsibility.
 *
 * Most likely, user will define its own structures and fer_mesh3_*
 * structs just embeds into to, for example:
 * ----
 *   struct my_vec_t {
 *       fer_vec3_t vec;       // Coordinates of vertex
 *       fer_mesh3_vertex_t v; // Vertex
 *   };
 *   ....
 *   struct my_vec_t v;
 *   // set vertex coordinates
 *   ferMesh3VertexSetCoordinates(&v.v, &v.vec);
 *   ...
 * ----
 *
 * NOTE: Don't access any struct's member directly instead use provided
 * inline functions.
 */
struct _fer_mesh3_t {
    fer_list_t verts; /*!< List of vertices */
    size_t verts_len; /*!< Number of vertices in list */
    fer_list_t edges; /*!< List of edges */
    size_t edges_len; /*!< Number of edges in list */
    fer_list_t faces; /*!< List of faces */
    size_t faces_len; /*!< Number of faces in list */
};
typedef struct _fer_mesh3_t fer_mesh3_t;


struct _fer_mesh3_vertex_t {
    const fer_vec3_t *v; /*!< User provided coordinates of vertex */
    fer_list_t list;     /*!< Connection into list of all vertices */

    fer_list_t edges; /*!< List of all incidenting edges */
    size_t edges_len; /*!< Number of edges in list */

    int _id; /*!< This is currently used only for ferMesh3DumpSVT() */
};
typedef struct _fer_mesh3_vertex_t fer_mesh3_vertex_t;


struct _fer_mesh3_edge_t {
    fer_mesh3_vertex_t *v[2];       /*!< Start and end point of edge */
    struct _fer_mesh3_face_t *f[2]; /*!< Incidenting faces */

    fer_list_t list;       /*!< Connection into list of all edges */
    fer_list_m_t vlist[2]; /*!< Connection into list of edges incidenting
                                with vertex.
                                .vlist[0] correspond with vertex .v[0] and
                                .vlist[1] with .v[1] */
};
typedef struct _fer_mesh3_edge_t fer_mesh3_edge_t;


struct _fer_mesh3_face_t {
    fer_mesh3_edge_t *e[3]; /*!< Incidenting edges */
    fer_list_t list;        /*!< Connection into list of all faces */
};
typedef struct _fer_mesh3_face_t fer_mesh3_face_t;


/**
 * Vertex
 * -------
 *
 * Vertex holds list of incidenting edges and *user-defined* vec3
 * coordinates. See above for example how to provide vec3 to vertex.
 *
 * Vertex don't have to be initialized because ferMesh3AddVertex()
 * initializes it when it's added to mesh.
 * Note that since there is no initialization function it is not safe to
 * use any ferMesh3Vertex*() function until it's added to mesh.
 * Note also that nothing ever touches fer_vec3_t vector stored (as
 * pointer) in vertex. It is safe anytime (even if it is not added to mesh)
 * to get or set coords - ferMesh3Vertex{,Set}Coords{,W}().
 */

/**
 * Allocates and initializes new vertex.
 */
fer_mesh3_vertex_t *ferMesh3VertexNew(void);

/**
 * Deletes vertex.
 */
void ferMesh3VertexDel(fer_mesh3_vertex_t *v);


/**
 * Sets pointer to user-defined vec3 coordinates.
 */
_fer_inline void ferMesh3VertexSetCoords(fer_mesh3_vertex_t *v,
                                         const fer_vec3_t *coords);

/**
 * Returns pointer to vector representing coordinates of vertex.
 */
_fer_inline const fer_vec3_t *ferMesh3VertexCoords(fer_mesh3_vertex_t *v);

/**
 * Returns writeable pointer to vec3 coordinates.
 */
_fer_inline fer_vec3_t *ferMesh3VertexCoordsW(fer_mesh3_vertex_t *v);

/**
 * Returns number of edges incidenting with given vertex.
 */
_fer_inline size_t ferMesh3VertexEdgesLen(const fer_mesh3_vertex_t *v);

/**
 * Returns head of list of edges incidenting with vertex.
 */
_fer_inline fer_list_t *ferMesh3VertexEdges(fer_mesh3_vertex_t *v);

/**
 * Returns true if vertex incidents with given edge.
 */
_fer_inline int ferMesh3VertexHasEdge(const fer_mesh3_vertex_t *v,
                                      const fer_mesh3_edge_t *e);



/**
 * Edge
 * -----
 * 
 * Edge doesn't have to be initialized explicitly - ferMesh3AddEdge()
 * initialize it when added to mesh. Note that since there is no
 * initialization function it is not safe to use any ferMesh3Edge*()
 * function until it's added to mesh.
 */

/**
 * Allocates and initializes edge.
 */
fer_mesh3_edge_t *ferMesh3EdgeNew(void);

/**
 * Deletes edge.
 */
void ferMesh3EdgeDel(fer_mesh3_edge_t *e);

/**
 * Returns start or end point of edge.
 * Parameter i can be either 0 or 1 (no check is performed).
 */
_fer_inline fer_mesh3_vertex_t *ferMesh3EdgeVertex(fer_mesh3_edge_t *e, size_t i);

/**
 * Returns incidenting face.
 * In 2-manifold mesh, edge can have maximally two incidenting faces, so i
 * can be 0 or 1.
 * No checking of i is performed!
 */
_fer_inline fer_mesh3_face_t *ferMesh3EdgeFace(fer_mesh3_edge_t *e, size_t i);

/**
 * Returns true if edge incidents with given vertex.
 */
_fer_inline int ferMesh3EdgeHasVertex(const fer_mesh3_edge_t *e,
                                      const fer_mesh3_vertex_t *v);

/**
 * Returns true if edge incidents with given face.
 */
_fer_inline int ferMesh3EdgeHasFace(const fer_mesh3_edge_t *e,
                                    const fer_mesh3_face_t *f);

/**
 * Returns number of incidenting faces.
 */
_fer_inline size_t ferMesh3EdgeFacesLen(const fer_mesh3_edge_t *e);

/**
 * Returns true if given triplet of edges form triangle.
 */
int ferMesh3EdgeTriCheck(const fer_mesh3_edge_t *e1,
                         const fer_mesh3_edge_t *e2,
                         const fer_mesh3_edge_t *e3);



/**
 * Face
 * -----
 *
 * Same as with edges - faces don't have to be initialized.
 */

/**
 * Allocates and initializes new face.
 */
fer_mesh3_face_t *ferMesh3FaceNew(void);

/**
 * Deletes face.
 */
void ferMesh3FaceDel(fer_mesh3_face_t *f);

/**
 * Returns incidenting edge.
 * Parameter i can be 0 or 1 or 2 since face has always exactly three
 * incidenting edges.
 * No check of parameters is performed.
 */
_fer_inline fer_mesh3_edge_t *ferMesh3FaceEdge(fer_mesh3_face_t *f, size_t i);

/**
 * Returns true if face incidents with given edge.
 */
_fer_inline int ferMesh3FaceHasEdge(const fer_mesh3_face_t *f,
                                    const fer_mesh3_edge_t *e);

/**
 * Returns true if face incidents with given vertex.
 */
_fer_inline int ferMesh3FaceHasVertex(const fer_mesh3_face_t *f,
                                      const fer_mesh3_vertex_t *v);

/**
 * Retuns three incidenting vertices (via vs argument).
 * Note that ns must be preallocated array of size 3.
 */
_fer_inline void ferMesh3FaceVertices(fer_mesh3_face_t *f,
                                      fer_mesh3_vertex_t **vs);



/**
 * Mesh
 * -----
 *
 * Mesh must be allocated on heap - no static declaration is allowed - use
 * ferMesh3New() function for allocation and ferMesh3Del() for
 * deallocation. If you need to deallocate also vertices, edges or faces
 * along with deallocation of whole mesh, use ferMesh3Del2() function that
 * allows you to provide callback for deallocation of elements.
 *
 * For iteration over vertices use this approach:
 * ----
 *   // note: mesh hold pointer to fer_mesh3_t
 *   fer_list_t *item;
 *   fer_mesh3_vertex_t *v;
 *   ferListForEach(ferMesh3Vertices(mesh), item) {
 *       v = ferListEntry(item, fer_mesh3_vertex_t, list);
 *       // here you have vertex stored in v
 *       ...
 *   }
 * ----
 * Similar approach can be used for edges and faces, only different type is
 * used in ferListEntry():
 * ----
 *   ferListEntry(item, fer_mesh3_{edge,face}_t, list);
 * ----
 *
 * TODO: Maybe some macros could be provided for iteration over vertices,
 *       edges and faces.
 *
 */

/**
 * Creates new empty mesh.
 */
fer_mesh3_t *ferMesh3New(void);

/**
 * Deletes mesh.
 * Warning: No vertices, edges or faces are deleted because they were
 * allocated outside a mesh! See ferMesh3Del2() if you want more
 * sofisticated destructor.
 */
void ferMesh3Del(fer_mesh3_t *m);

/**
 * Deletes mesh. This destructor can used for deleting all vertices, edges
 * and faces contained in mesh.
 *
 * Before freeing a mesh, destructor iterates over all vertices. Each
 * vertex is first disconnected from mesh and then delvertex is called with
 * second argument vdata. Similarly are iterated edges and faces.
 */
void ferMesh3Del2(fer_mesh3_t *m,
                  void (*delvertex)(fer_mesh3_vertex_t *, void *), void *vdata,
                  void (*deledge)(fer_mesh3_edge_t *, void *), void *edata,
                  void (*delface)(fer_mesh3_face_t *, void *), void *fdata);

/**
 * Returns number of vertices stored in mesh.
 */
_fer_inline size_t ferMesh3VerticesLen(const fer_mesh3_t *m);

/**
 * Returns number of edges stored in mesh.
 */
_fer_inline size_t ferMesh3EdgesLen(const fer_mesh3_t *m);

/**
 * Returns number of faces stored in mesh.
 */
_fer_inline size_t ferMesh3FacesLen(const fer_mesh3_t *m);

/**
 * Returns list of vertices.
 */
_fer_inline fer_list_t *ferMesh3Vertices(fer_mesh3_t *m);

/**
 * Returns list of edges.
 */
_fer_inline fer_list_t *ferMesh3Edges(fer_mesh3_t *m);

/**
 * Returns list of faces.
 */
_fer_inline fer_list_t *ferMesh3Faces(fer_mesh3_t *m);


/**
 * Adds vertex into mesh.
 */
void ferMesh3AddVertex(fer_mesh3_t *m, fer_mesh3_vertex_t *v);

/**
 * Removes vertex into mesh.
 * If vertex is connected with any edge, vertex can't be removed and -1 is
 * returned. On success 0 is returned.
 */
int ferMesh3RemoveVertex(fer_mesh3_t *m, fer_mesh3_vertex_t *v);

/**
 * Adds edge into mesh. Start and end points (vertices) must be provided.
 */
void ferMesh3AddEdge(fer_mesh3_t *m, fer_mesh3_edge_t *e,
                     fer_mesh3_vertex_t *start, fer_mesh3_vertex_t *end);

/**
 * Removes edge from mesh.
 * If edge incidents with any face, edge can't be removed and -1 is
 * returned. On success 0 is returned.
 */
int ferMesh3RemoveEdge(fer_mesh3_t *m, fer_mesh3_edge_t *e);

/**
 * Adds face into mesh. Three incidenting faces must be provided.
 * If any of given edge already incidents with two faces, face can't be
 * added to mesh and -1 is returned. On success is returned 0.
 *
 * The three provided edges should form triangle (by start and end points),
 * note that check if it is so is not performed! - see ferMesh3EdgeTriCheck()
 */
int ferMesh3AddFace(fer_mesh3_t *m, fer_mesh3_face_t *f,
                    fer_mesh3_edge_t *e1, fer_mesh3_edge_t *e2,
                    fer_mesh3_edge_t *e3);

/**
 * Removes face from mesh.
 * Note that this function can never fail.
 */
void ferMesh3RemoveFace(fer_mesh3_t *m, fer_mesh3_face_t *f);

/**
 * Dumps mesh as one object in SVT format.
 * See http://svt.danfis.cz for more info.
 */
void ferMesh3DumpSVT(fer_mesh3_t *m, FILE *out, const char *name);

/**
 * Dumps mesh as list of triangles.
 * One triangle per line (ax ay az bx by ...)
 */
void ferMesh3DumpTriangles(fer_mesh3_t *m, FILE *out);

/**
 * Dumps mesh in povray format.
 */
void ferMesh3DumpPovray(fer_mesh3_t *m, FILE *out);



/**** INLINES ****/
_fer_inline void ferMesh3VertexSetCoords(fer_mesh3_vertex_t *v,
                                         const fer_vec3_t *coords)
{
    v->v = coords;
}

_fer_inline const fer_vec3_t *ferMesh3VertexCoords(fer_mesh3_vertex_t *v)
{
    return v->v;
}

_fer_inline fer_vec3_t *ferMesh3VertexCoordsW(fer_mesh3_vertex_t *v)
{
    return (fer_vec3_t *)v->v;
}

_fer_inline size_t ferMesh3VertexEdgesLen(const fer_mesh3_vertex_t *v)
{
    return v->edges_len;
}

_fer_inline fer_list_t *ferMesh3VertexEdges(fer_mesh3_vertex_t *v)
{
    return &v->edges;
}

_fer_inline int ferMesh3VertexHasEdge(const fer_mesh3_vertex_t *v,
                                      const fer_mesh3_edge_t *e)
{
    fer_list_t *item;
    fer_list_m_t *mitem;
    fer_mesh3_edge_t *edge;

    ferListForEach(&v->edges, item){
        mitem = ferListMFromList(item);
        edge = ferListEntry(item, fer_mesh3_edge_t, vlist[mitem->mark]);
        if (edge == e)
            return 1;
    }

    return 0;
}





_fer_inline fer_mesh3_vertex_t *ferMesh3EdgeVertex(fer_mesh3_edge_t *e, size_t i)
{
    return e->v[i];
}

_fer_inline fer_mesh3_face_t *ferMesh3EdgeFace(fer_mesh3_edge_t *e, size_t i)
{
    return e->f[i];
}

_fer_inline int ferMesh3EdgeHasVertex(const fer_mesh3_edge_t *e,
                                      const fer_mesh3_vertex_t *v)
{
    return e->v[0] == v || e->v[1] == v;
}

_fer_inline int ferMesh3EdgeHasFace(const fer_mesh3_edge_t *e,
                                    const fer_mesh3_face_t *f)
{
    return e->f[0] == f || e->f[1] == f;
}

_fer_inline size_t ferMesh3EdgeFacesLen(const fer_mesh3_edge_t *e)
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




_fer_inline fer_mesh3_edge_t *ferMesh3FaceEdge(fer_mesh3_face_t *f, size_t i)
{
    return f->e[i];
}

_fer_inline int ferMesh3FaceHasEdge(const fer_mesh3_face_t *f,
                                    const fer_mesh3_edge_t *e)
{
    return f->e[0] == e
            || f->e[1] == e
            || f->e[2] == e;
}

_fer_inline int ferMesh3FaceHasVertex(const fer_mesh3_face_t *f,
                                      const fer_mesh3_vertex_t *v)
{
    fer_mesh3_vertex_t *vs[3];
    ferMesh3FaceVertices((fer_mesh3_face_t *)f, vs);
    return vs[0] == v || vs[1] == v || vs[2] == v;
}

_fer_inline void ferMesh3FaceVertices(fer_mesh3_face_t *f,
                                      fer_mesh3_vertex_t **vs)
{
    fer_mesh3_vertex_t *n;

    vs[0] = ferMesh3EdgeVertex(f->e[0], 0);
    vs[1] = ferMesh3EdgeVertex(f->e[0], 1);

    n = ferMesh3EdgeVertex(f->e[1], 0);
    if (n == vs[0] || n == vs[1]){
        vs[2] = ferMesh3EdgeVertex(f->e[1], 1);
    }else{
        vs[2] = ferMesh3EdgeVertex(f->e[1], 0);
    }
}




_fer_inline size_t ferMesh3VerticesLen(const fer_mesh3_t *m)
{
    return m->verts_len;
}

_fer_inline size_t ferMesh3EdgesLen(const fer_mesh3_t *m)
{
    return m->edges_len;
}

_fer_inline size_t ferMesh3FacesLen(const fer_mesh3_t *m)
{
    return m->faces_len;
}

_fer_inline fer_list_t *ferMesh3Vertices(fer_mesh3_t *m)
{
    return &m->verts;
}

_fer_inline fer_list_t *ferMesh3Edges(fer_mesh3_t *m)
{
    return &m->edges;
}

_fer_inline fer_list_t *ferMesh3Faces(fer_mesh3_t *m)
{
    return &m->faces;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_MESH3_H__ */

