/***awInputPoint(g);
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

#include <fermat/surface/gsrm.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

struct _node_t {
    // TODO: must be changed to pointer (SSE)
    fer_vec3_t *v; /*!< Position of node (weight vector) */

    fer_real_t err_counter;  /*!< Error counter */
    size_t err_counter_mark; /*!< Mark used for accumulated error counter */

    int simpl_ban; /*!< true if node is banned from simplification (deletion) */

    fer_mesh3_vertex_t vert; /*!< Vertex in mesh */
    fer_cubes3_el_t cubes;   /*!< Struct for NN search */
};
typedef struct _node_t node_t;

struct _edge_t {
    int age;      /*!< Age of edge */
    int obsolete; /*!< Obsolete flag (TODO) */

    fer_mesh3_edge_t edge; /*!< Edge in mesh */
};
typedef struct _edge_t edge_t;

struct _face_t {
    fer_mesh3_face_t face; /*!< Face in mesh */
};
typedef struct _face_t face_t;

struct _fer_gsrm_cache_t {
    fer_vec3_t *is;     /*!< Input signal */
    node_t *nearest[2]; /*!< Two nearest nodes */
    node_t **common_neighb; /*!< Array of common neighbors */
    size_t common_neighb_size;
    size_t common_neighb_len;

    size_t err_counter_mark; /*!< TODO */
    fer_real_t err_counter_scale; /*!< TODO */
};
typedef struct _fer_gsrm_cache_t fer_gsrm_cache_t;

/** Allocates and deallocates cache */
static fer_gsrm_cache_t *cacheNew(void);
static void cacheDel(fer_gsrm_cache_t *c);

/** Creates new node */
static node_t *nodeNew(fer_gsrm_t *g, const fer_vec3_t *v);
/** Deletes node */
static void nodeDel(fer_gsrm_t *g, node_t *n);
static void nodeDel2(fer_mesh3_vertex_t *v, void *data);

/** TODO */
static void nodeErrCounterApply(fer_gsrm_t *g, node_t *n);
static fer_real_t nodeErrCounter(fer_gsrm_t *g, node_t *n);
static fer_real_t nodeErrCounterScale(fer_gsrm_t *g, node_t *n, fer_real_t s);
static fer_real_t nodeErrCounterReset(fer_gsrm_t *g, node_t *n);
/** Increases error counter (TODO) */
static void nodeErrCounterInc(fer_gsrm_t *g, node_t *n, const fer_vec3_t *v);
/** Reset all error counters */
static void nodeErrCounterResetAll(fer_gsrm_t *g);
/** Scale all error counters */
static void nodeErrCounterScaleAll(fer_gsrm_t *g);


static edge_t *edgeNew(fer_gsrm_t *g, node_t *n1, node_t *n2);
static void edgeDel(fer_gsrm_t *g, edge_t *e);
static void edgeDel2(fer_mesh3_edge_t *v, void *data);

/** TODO */
static face_t *faceNew(fer_gsrm_t *g, edge_t *e, node_t *n);
static void faceDel(fer_gsrm_t *g, face_t *e);
static void faceDel2(fer_mesh3_face_t *v, void *data);


/* Initializes mesh with three random numbers from input */
static void meshInit(fer_gsrm_t *g);
static void drawInputPoint(fer_gsrm_t *g);
/** Performes Extended Competitive Hebbian Learning */
static void echl(fer_gsrm_t *g);
static void echlConnectNodes(fer_gsrm_t *g);
static void echlMove(fer_gsrm_t *g);
static void echlUpdate(fer_gsrm_t *g);
/** Creates new node */
static void createNewNode(fer_gsrm_t *g);
/** Decreases error counter */
static void decreaseErrCounter(fer_gsrm_t *g);


fer_gsrm_t *ferGSRMNew(void)
{
    fer_gsrm_t *g;

    g = FER_ALLOC(fer_gsrm_t);

    // init params:
    g->param.lambda = 200;
    g->param.eb = 0.05;
    g->param.en = 0.0006;
    g->param.alpha = 0.95;
    g->param.beta = 0.9995;
    g->param.age_max = 200;
    g->param.max_nodes = 5000;
    g->param.num_cubes = 5000;

    g->param.min_dangle = M_PI_4;
    g->param.max_angle = M_PI_2 * 1.5;
    g->param.angle_merge_edges = M_PI * 0.9;

    g->param.simpl_dist_treshold = 0.;
    g->param.simpl_max_node_dec = -1.;
    g->param.simpl_max_face_dec = -1.;

    // initialize point cloude (input signals)
    g->is = ferPCNew();

    // init 3D mesh
    g->mesh = ferMesh3New();

    // init cubes for NN search to NULL, actual allocation will be made
    // after we know what area do we need to cover
    g->cubes = NULL;

    g->c = NULL;

    return g;
}

void ferGSRMDel(fer_gsrm_t *g)
{
    if (g->c)
        cacheDel(g->c);

    if (g->is)
        ferPCDel(g->is);

    if (g->mesh)
        ferMesh3Del2(g->mesh, nodeDel2, (void *)g,
                              edgeDel2, (void *)g,
                              faceDel2, (void *)g);

    if (g->cubes)
        ferCubes3Del(g->cubes);

    free(g);
}

size_t ferGSRMAddInputSignals(fer_gsrm_t *g, const char *fn)
{
    return ferPCAddFromFile(g->is, fn);
}

int ferGSRMRun(fer_gsrm_t *g)
{
    const fer_real_t *aabb;
    size_t step;

    // check if there are some input signals
    if (ferPCLen(g->is) <= 3){
        DBG2("No input signals!");
        return -1;
    }

    DBG2("Init cache");
    // initialize cache
    if (!g->c)
        g->c = cacheNew();

    DBG2("Init NN search");
    // initialize NN search structure
    if (!g->cubes){
        aabb = ferPCAABB(g->is);
        g->cubes = ferCubes3New(aabb, g->param.num_cubes);
    }

    DBG2("PC");
    // first shuffle of all input signals
    ferPCPermutate(g->is);
    // and initialize is's iterator
    ferPCItInit(&g->isit, g->is);

    // initialize mesh
    meshInit(g);

    step = 1;
    while (ferMesh3VerticesLen(g->mesh) < g->param.max_nodes){
        fprintf(stderr, "%06d, %08d\r", step, ferMesh3VerticesLen(g->mesh));

        drawInputPoint(g);

        echl(g);

        if (step >= g->param.lambda){
            createNewNode(g);
            step = 0;
            //ferMesh3DumpSVT(g->mesh, stdout, "1");
        }

        decreaseErrCounter(g);

        step++;
    }

    // TODO
    return -1;
}




static fer_gsrm_cache_t *cacheNew(void)
{
    fer_gsrm_cache_t *c;

    c = FER_ALLOC(fer_gsrm_cache_t);
    c->nearest[0] = c->nearest[1] = NULL;

    c->common_neighb_size = 3;
    c->common_neighb = FER_ALLOC_ARR(node_t *, c->common_neighb_size);
    c->common_neighb_len = 0;

    c->err_counter_mark = 0;
    c->err_counter_scale = FER_ONE;

    return c;
}

static void cacheDel(fer_gsrm_cache_t *c)
{
    free(c->common_neighb);
    free(c);
}



static node_t *nodeNew(fer_gsrm_t *g, const fer_vec3_t *v)
{
    node_t *n;

    n = FER_ALLOC(node_t);
    n->v = ferVec3Clone(v);

    // initialize mesh's vertex struct with weight vector
    ferMesh3VertexSetCoords(&n->vert, n->v);

    // initialize cubes struct with its own weight vector
    ferCubes3ElInit(&n->cubes, n->v);

    // add node into mesh
    ferMesh3AddVertex(g->mesh, &n->vert);
    // and add node into cubes
    ferCubes3Add(g->cubes, &n->cubes);

    // set error counter (and mark)
    n->err_counter = FER_ZERO;
    n->err_counter_mark = g->c->err_counter_mark;

    //DBG("n: %lx, vert: %lx (%g %g %g)", (long)n, (long)&n->vert,
    //    ferVec3X(&n->v), ferVec3Y(&n->v), ferVec3Z(&n->v));

    return n;
}

static void nodeDel(fer_gsrm_t *g, node_t *n)
{
    fer_list_t *list, *item, *item_tmp;
    fer_mesh3_edge_t *edge;
    edge_t *e;
    int res;

    // remove node from mesh
    if (fer_unlikely(ferMesh3VertexEdgesLen(&n->vert) > 0)){
        // remove edges first
        list = ferMesh3VertexEdges(&n->vert);
        ferListForEachSafe(list, item, item_tmp){
            edge = ferMesh3EdgeFromVertexList(item);
            e = fer_container_of(edge, edge_t, edge);

            edgeDel(g, e);
        }
    }

    // then vertex
    res = ferMesh3RemoveVertex(g->mesh, &n->vert);
    if (fer_unlikely(res != 0)){
        DBG2("Node couldn't be removed from mesh - this shouldn't happen!");
        exit(-1);
    }

    ferVec3Del(n->v);

    // remove node from cubes
    ferCubes3Remove(g->cubes, &n->cubes);

    //DBG("n: %lx, vert: %lx", (long)n, (long)&n->vert);
    // Note: no need of deallocation of .vert and .cubes
    free(n);
}

static void nodeDel2(fer_mesh3_vertex_t *v, void *data)
{
    fer_gsrm_t *g = (fer_gsrm_t *)data;
    node_t *n;
    n = fer_container_of(v, node_t, vert);

    ferVec3Del(n->v);

    // remove node from cubes
    ferCubes3Remove(g->cubes, &n->cubes);

    free(n);
}

static void nodeErrCounterApply(fer_gsrm_t *g, node_t *n)
{
    size_t mark, left;
    fer_real_t err;

    mark = n->err_counter_mark;
    left = g->c->err_counter_mark - mark;

    if (fer_likely(left > 0)){
        if (fer_likely(left == g->c->err_counter_mark)){
            // most of nodes in mesh are not touched while ECHL phase - so
            // scale factor can be cumulated and can be used directly
            // without pow() operation
            n->err_counter *= g->c->err_counter_scale;
        }else{
            err = FER_POW(g->param.beta, (fer_real_t)left);
            n->err_counter *= err;
        }
    }
    //DBG("err_counter: %g", n->err_counter);

    n->err_counter_mark = g->c->err_counter_mark;
}

static fer_real_t nodeErrCounter(fer_gsrm_t *g, node_t *n)
{
    nodeErrCounterApply(g, n);
    return n->err_counter;
}

static fer_real_t nodeErrCounterScale(fer_gsrm_t *g, node_t *n, fer_real_t s)
{
    nodeErrCounterApply(g, n);
    n->err_counter *= s;
    return n->err_counter;
}

static fer_real_t nodeErrCounterReset(fer_gsrm_t *g, node_t *n)
{
    nodeErrCounterApply(g, n);
    n->err_counter_mark = 0;
    return n->err_counter;
}

static void nodeErrCounterInc(fer_gsrm_t *g, node_t *n, const fer_vec3_t *v)
{
    fer_real_t dist;

    dist = ferVec3Dist2(n->v, v);

    nodeErrCounterApply(g, n);
    n->err_counter += dist;
}

static void nodeErrCounterResetAll(fer_gsrm_t *g)
{
    g->c->err_counter_mark = 0;
    g->c->err_counter_scale = FER_ONE;
}

static void nodeErrCounterScaleAll(fer_gsrm_t *g)
{
    g->c->err_counter_scale *= g->param.beta;
    g->c->err_counter_mark++;
}




static edge_t *edgeNew(fer_gsrm_t *g, node_t *n1, node_t *n2)
{
    edge_t *e;

    e = FER_ALLOC(edge_t);
    e->age = 0;
    e->obsolete = 0;

    ferMesh3AddEdge(g->mesh, &e->edge, &n1->vert, &n2->vert);

    //DBG("e: %lx, edge: %lx", (long)e, (long)&e->edge);

    return e;
}

static void edgeDel(fer_gsrm_t *g, edge_t *e)
{
    fer_mesh3_face_t *face;
    int res;

    // first remove incidenting faces
    while ((face = ferMesh3EdgeFace(&e->edge, 0)) != NULL){
        faceDel(g, fer_container_of(face, face_t, face));
    }

    // then remove edge itself
    res = ferMesh3RemoveEdge(g->mesh, &e->edge);
    if (fer_unlikely(res != 0)){
        DBG2("Can't remove edge - this shouldn'h happen!");
        exit(-1);
    }

    //DBG("e: %lx, edge: %lx", (long)e, (long)&e->edge);
    free(e);
}

static void edgeDel2(fer_mesh3_edge_t *edge, void *data)
{
    edge_t *e;
    e = fer_container_of(edge, edge_t, edge);
    free(e);
}




static face_t *faceNew(fer_gsrm_t *g, edge_t *e, node_t *n)
{
    face_t *f;
    fer_mesh3_edge_t *e2, *e3;
    int res;

    e2 = ferMesh3VertexCommonEdge(ferMesh3EdgeVertex(&e->edge, 0), &n->vert);
    e3 = ferMesh3VertexCommonEdge(ferMesh3EdgeVertex(&e->edge, 1), &n->vert);
    if (fer_unlikely(!e2 || !e3)){
        DBG2("Can't create face because *the* three nodes are not connected "
             " - this shouldn't happen!");
        return NULL;
    }

    f = FER_ALLOC(face_t);

    //DBG("e: %lx, e2: %lx, e3: %lx", (long)e, (long)e2, (long)e3);
    //DBG("f: %lx, face: %lx", (long)f, (long)&f->face);
    res = ferMesh3AddFace(g->mesh, &f->face, &e->edge, e2, e3);
    if (fer_unlikely(res != 0)){
        free(f);
        return NULL;
    }

    // TODO: check if face already exists

    return f;
}

static void faceDel(fer_gsrm_t *g, face_t *f)
{
    ferMesh3RemoveFace(g->mesh, &f->face);
    //DBG("f: %lx, face: %lx", (long)f, (long)&f->face);
    free(f);
}

static void faceDel2(fer_mesh3_face_t *face, void *data)
{
    face_t *f;
    f = fer_container_of(face, face_t, face);
    free(f);
}


static void meshInit(fer_gsrm_t *g)
{
    fer_vec3_t *v;
    node_t *n;
    size_t i;

    for (i = 0; i < 3; i++){
        // obtain input signal
        v = ferPCItGet(&g->isit);

        // create new node
        n = nodeNew(g, v);

        // move to next point
        ferPCItNext(&g->isit);
    }
}

static void drawInputPoint(fer_gsrm_t *g)
{
    if (ferPCItEnd(&g->isit)){
        // if iterator is at the end permutate point cloud again
        ferPCPermutate(g->is);
        // and re-initialize iterator
        ferPCItInit(&g->isit, g->is);
    }
    g->c->is = ferPCItGet(&g->isit);
    //DBG_VEC3(g->c->is, "IS: ");
    ferPCItNext(&g->isit);
}

static void echl(fer_gsrm_t *g)
{
    fer_cubes3_el_t *el[2];

    //DBG2("1. ");
    //DBG("%lx", (long)g->c->is);
    // 1. Find two nearest nodes
    ferCubes3Nearest(g->cubes, g->c->is, 2, el);
    g->c->nearest[0] = fer_container_of(el[0], node_t, cubes);
    g->c->nearest[1] = fer_container_of(el[1], node_t, cubes);
    /*
    DBG("nearest[0]: %lx %g %g %g", (long)g->c->nearest[0],
        ferVec3X(&g->c->nearest[0]->v),
        ferVec3Y(&g->c->nearest[0]->v),
        ferVec3Z(&g->c->nearest[0]->v));
    DBG("nearest[1]: %lx %g %g %g", (long)g->c->nearest[1],
        ferVec3X(&g->c->nearest[1]->v),
        ferVec3Y(&g->c->nearest[1]->v),
        ferVec3Z(&g->c->nearest[1]->v));
    */

    //DBG2("2. ");
    //DBG("%lx", (long)g->c->is);
    // 2. Updates winners error counter
    nodeErrCounterInc(g, g->c->nearest[0], g->c->is);

    //DBG2("3. ");
    // 3. Connect winning nodes
    echlConnectNodes(g);

    //DBG2("4. ");
    // 4. Move winning node and its neighbors towards input signal
    echlMove(g);

    //DBG2("5. ");
    // 5. Update all edges emitating from winning node
    echlUpdate(g);
}

static void echlCommonNeighbors(fer_gsrm_t *g, node_t *n1, node_t *n2)
{
    fer_list_t *list1, *list2;
    fer_list_t *item1, *item2;
    fer_mesh3_edge_t *edge1, *edge2;
    fer_mesh3_vertex_t *o1, *o2;
    node_t *n;
    size_t len;

    // allocate enough memory for common neighbors
    if (g->c->common_neighb_size < ferMesh3VertexEdgesLen(&n1->vert)
            && g->c->common_neighb_size < ferMesh3VertexEdgesLen(&n2->vert)){
        len = ferMesh3VertexEdgesLen(&n1->vert);
        len = FER_MIN(len, ferMesh3VertexEdgesLen(&n2->vert));

        g->c->common_neighb = FER_REALLOC_ARR(g->c->common_neighb, node_t *, len);
        g->c->common_neighb_size = len;
    }

    list1 = ferMesh3VertexEdges(&n1->vert);
    list2 = ferMesh3VertexEdges(&n2->vert);
    len = 0;
    ferListForEach(list1, item1){
        edge1 = ferMesh3EdgeFromVertexList(item1);
        //DBG("edge1: %lx", (long)edge1);
        o1 = ferMesh3EdgeVertex(edge1, 0);
        if (o1 == &n1->vert)
            o1 = ferMesh3EdgeVertex(edge1, 1);

        ferListForEach(list2, item2){
            edge2 = ferMesh3EdgeFromVertexList(item2);
            //DBG("edge2: %lx", (long)edge2);
            o2 = ferMesh3EdgeVertex(edge2, 0);
            if (o2 == &n2->vert)
                o2 = ferMesh3EdgeVertex(edge2, 1);

            if (o1 == o2){
                n = fer_container_of(o1, node_t, vert);
                g->c->common_neighb[len] = n;
                len++;
            }
        }
    }

    g->c->common_neighb_len = len;
}

static void echlRemoveThales(fer_gsrm_t *g, edge_t *e, node_t *n1, node_t *n2)
{
    node_t *nb;
    size_t i, len;

    len = g->c->common_neighb_len;
    for (i=0; i < len; i++){
        nb = g->c->common_neighb[i];

        if (ferVec3Angle(n1->v, nb->v, n2->v) > M_PI_2){
            // remove edge
            edgeDel(g, e);
            return;
        }
    }
}

static void echlRemoveNeighborsEdges(fer_gsrm_t *g)
{
    size_t i, j, len;
    fer_mesh3_edge_t *edge;
    node_t **ns;
    edge_t *e;

    ns = g->c->common_neighb;
    len = g->c->common_neighb_len;
    if (len == 0)
        return;

    for (i = 0; i < len; i++){
        for (j = i + 1; j < len; j++){
            edge = ferMesh3VertexCommonEdge(&ns[i]->vert, &ns[j]->vert);
            if (edge != NULL){
                e = fer_container_of(edge, edge_t, edge);
                //DBG("Deleting edge %lx (%lx)", (long)e, (long)edge);
                edgeDel(g, e);
            }
        }
    }
}

static void echlCreateFaces(fer_gsrm_t *g, edge_t *e)
{
    size_t i, len;
    node_t **ns;

    len = FER_MIN(g->c->common_neighb_len, 2);
    ns = g->c->common_neighb;
    for (i = 0; i < len; i++){
        faceNew(g, e, ns[i]);
    }
}

static void echlConnectNodes(fer_gsrm_t *g)
{
    fer_mesh3_edge_t *edge;
    edge_t *e;
    node_t *n1, *n2;

    n1 = g->c->nearest[0];
    n2 = g->c->nearest[1];

    // get edge connecting n1 and n2
    e = NULL;
    edge = ferMesh3VertexCommonEdge(&n1->vert, &n2->vert);
    if (edge){
        e = fer_container_of(edge, edge_t, edge);
    }

    // get common neighbors
    echlCommonNeighbors(g, n1, n2);

    if (e != NULL){
        //DBG2("Nodes are connected");

        // set age of edge to zero
        e->age = 0;

        // Remove edge if opposite node lies inside thales sphere
        echlRemoveThales(g, e, n1, n2);
    }else{
        //DBG2("Nodes are NOT connected");

        // remove all edges that connect common neighbors
        echlRemoveNeighborsEdges(g);

        // create new edge between n1 and n2
        e = edgeNew(g, n1, n2);

        // create faces with common neighbors
        echlCreateFaces(g, e);
    }
}

_fer_inline void echlMoveNode(fer_gsrm_t *g, node_t *n, fer_real_t k)
{
    fer_vec3_t v;
    ferVec3Sub2(&v, g->c->is, n->v);
    ferVec3Scale(&v, k);
    //DBG_VEC3(g->c->is, "g->c->is: ");
    //DBG_VEC3(&n->v, "n->v: ");
    ferVec3Add(n->v, &v);
    //DBG_VEC3(&n->v, "n->v: ");
    //DBG2("");
}

static void echlMove(fer_gsrm_t *g)
{
    fer_list_t *list, *item;
    fer_mesh3_edge_t *edge;
    fer_mesh3_vertex_t *wvert, *vert;
    node_t *wn;

    wn = g->c->nearest[0];
    wvert = &wn->vert;

    // move winning node
    echlMoveNode(g, wn, g->param.eb);

    // move nodes connected with the winner
    list = ferMesh3VertexEdges(wvert);
    ferListForEach(list, item){
        edge = ferMesh3EdgeFromVertexList(item);
        //DBG("edge: %lx", (long)edge);
        vert = ferMesh3EdgeOtherVertex(edge, wvert);

        echlMoveNode(g, fer_container_of(vert, node_t, vert), g->param.en);
    }
}

static void echlUpdate(fer_gsrm_t *g)
{
    node_t *wn;
    edge_t *e;
    fer_list_t *list, *item, *tmp_item;
    fer_mesh3_edge_t *edge;
    fer_mesh3_vertex_t *vert;

    wn = g->c->nearest[0];

    list = ferMesh3VertexEdges(&wn->vert);
    ferListForEachSafe(list, item, tmp_item){
        edge = ferMesh3EdgeFromVertexList(item);
        e = fer_container_of(edge, edge_t, edge);

        // increment age of edge
        e->age++;

        // if age of edge is above treshold remove edge and nodes which
        // remain unconnected
        if (e->age > g->param.age_max){
            // get other node than winning one
            vert = ferMesh3EdgeOtherVertex(edge, &wn->vert);

            // delete edge
            edgeDel(g, e);

            // check if n is connected in mesh, if not delete it
            if (ferMesh3VertexEdgesLen(vert) == 0){
                nodeDel(g, fer_container_of(vert, node_t, vert));
            }
        }
    }

    // check if winning node remains connected
    if (ferMesh3VertexEdgesLen(&wn->vert) == 0){
        nodeDel(g, wn);
    }
}



static node_t *nodeWithHighestErrCounter(fer_gsrm_t *g)
{
    fer_list_t *list, *item;
    fer_real_t max_err, err;
    node_t *max_n, *n;
    fer_mesh3_vertex_t *vert;

    max_err = FER_REAL_MIN;
    list = ferMesh3Vertices(g->mesh);
    ferListForEach(list, item){
        vert = ferListEntry(item, fer_mesh3_vertex_t, list);
        n    = fer_container_of(vert, node_t, vert);
        err  = nodeErrCounterReset(g, n);

        if (err > max_err){
            //DBG("err: %g, %lx", err, (long)n);
            max_err = err;
            max_n   = n;
        }
    }

    nodeErrCounterResetAll(g);

    return max_n;
}

static node_t *nodesNeighborWithHighestErrCounter(fer_gsrm_t *g, node_t *sq)
{
    fer_list_t *list, *item;
    fer_mesh3_edge_t *edge;
    fer_mesh3_vertex_t *other_vert;
    fer_real_t err, max_err;
    node_t *n, *max_n;

    max_err = FER_REAL_MIN;
    list = ferMesh3VertexEdges(&sq->vert);
    ferListForEach(list, item){
        edge = ferMesh3EdgeFromVertexList(item);
        other_vert = ferMesh3EdgeOtherVertex(edge, &sq->vert);
        n = fer_container_of(other_vert, node_t, vert);
        err = nodeErrCounter(g, n);
        if (err > max_err){
            max_err = err;
            max_n   = n;
        }
    }

    return n;
}

static node_t *createNewNode2(fer_gsrm_t *g, node_t *sq, node_t *sf)
{
    node_t *sr;
    fer_vec3_t v;

    ferVec3Add2(&v, sq->v, sf->v);
    ferVec3Scale(&v, FER_REAL(0.5));

    sr = nodeNew(g, &v);

    return sr;
}

static void createNewNode(fer_gsrm_t *g)
{
    node_t *sq, *sf, *sr;
    fer_mesh3_edge_t *edge;
    fer_real_t err;


    // get node with highest error counter and its neighbor with highest
    // error counter
    sq = nodeWithHighestErrCounter(g);
    sf = nodesNeighborWithHighestErrCounter(g, sq);
    if (!sq || !sf){
        DBG2("Can't create new node, because sq has no neighbors");
        return;
    }

    //DBG("sq: %lx, sf: %lx", (long)sq, (long)sf);

    // delete common edge of sq and sf
    edge = ferMesh3VertexCommonEdge(&sq->vert, &sf->vert);
    if (edge){
        edgeDel(g, fer_container_of(edge, edge_t, edge));
    }

    // create new node
    sr = createNewNode2(g, sq, sf);

    // set up error counters of sq, sf and sr
    err  = nodeErrCounterScale(g, sq, g->param.alpha);
    err += nodeErrCounterScale(g, sf, g->param.alpha);
    err *= FER_REAL(0.5);
    sr->err_counter = err;

    // create edges sq-sr and sf-sr
    edgeNew(g, sq, sr);
    edgeNew(g, sf, sr);
}



static void decreaseErrCounter(fer_gsrm_t *g)
{
    nodeErrCounterScaleAll(g);
}
