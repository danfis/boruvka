#include <cu/cu.h>
#include <boruvka/mesh3.h>

FILE *pov, *tri;

TEST(testMesh3SetUp)
{
    pov = fopen("reg/tmp.TSMesh3-pov.out", "w");
    tri = fopen("reg/tmp.TSMesh3-tri.out", "w");
}

TEST(testMesh3TearDown)
{
    fclose(pov);
    fclose(tri);
}

TEST(testMesh){
    bor_mesh3_t *mesh;
    bor_vec3_t vcoords[4];
    bor_mesh3_vertex_t v[4];
    bor_mesh3_edge_t e[5];
    bor_mesh3_face_t f[2], f2;
    size_t i;

    mesh = borMesh3New();

    for (i = 0; i < 4; i++)
        borMesh3VertexSetCoords(&v[i], &vcoords[i]);
    borVec3Set(borMesh3VertexCoordsW(&v[0]), 0., 0., 0.);
    borVec3Set(borMesh3VertexCoordsW(&v[1]), 1., 0., 0.);
    borVec3Set(borMesh3VertexCoordsW(&v[2]), 0., 1., 0.);
    borVec3Set(borMesh3VertexCoordsW(&v[3]), 1., 1., 1.);

    borMesh3AddVertex(mesh, &v[0]);
    borMesh3AddVertex(mesh, &v[1]);
    borMesh3AddVertex(mesh, &v[2]);
    borMesh3AddVertex(mesh, &v[3]);
    assertEquals(borMesh3RemoveVertex(mesh, &v[2]), 0);
    borMesh3AddVertex(mesh, &v[2]);
    assertEquals(borMesh3VerticesLen(mesh), 4);

    borMesh3AddEdge(mesh, &e[0], &v[0], &v[1]);
    borMesh3AddEdge(mesh, &e[1], &v[0], &v[2]);
    borMesh3AddEdge(mesh, &e[2], &v[1], &v[2]);
    borMesh3AddEdge(mesh, &e[3], &v[1], &v[3]);
    borMesh3AddEdge(mesh, &e[4], &v[2], &v[3]);
    assertEquals(borMesh3RemoveEdge(mesh, &e[4]), 0);
    borMesh3AddEdge(mesh, &e[4], &v[2], &v[3]);
    assertEquals(borMesh3EdgesLen(mesh), 5);

    assertEquals(borMesh3VertexEdgesLen(&v[2]), 3);
    assertEquals(borMesh3VertexEdgesLen(&v[1]), 3);
    assertEquals(borMesh3VertexEdgesLen(&v[0]), 2);
    assertTrue(borMesh3VertexHasEdge(&v[0], &e[0]));
    assertTrue(borMesh3VertexHasEdge(&v[0], &e[1]));
    assertFalse(borMesh3VertexHasEdge(&v[0], &e[2]));
    assertTrue(borMesh3VertexHasEdge(&v[1], &e[0]));
    assertTrue(borMesh3VertexHasEdge(&v[1], &e[2]));
    assertTrue(borMesh3VertexHasEdge(&v[1], &e[3]));
    assertTrue(borMesh3VertexHasEdge(&v[2], &e[1]));
    assertTrue(borMesh3VertexHasEdge(&v[2], &e[2]));

    assertEquals(borMesh3VertexCommonEdge(&v[0], &v[1]), &e[0]);
    assertEquals(borMesh3VertexCommonEdge(&v[2], &v[1]), &e[2]);

    assertTrue(borMesh3EdgeTriCheck(&e[0], &e[1], &e[2]));
    assertTrue(borMesh3EdgeTriCheck(&e[2], &e[3], &e[4]));
    assertEquals(borMesh3AddFace(mesh, &f[0], &e[0], &e[1], &e[2]), 0);
    assertEquals(borMesh3AddFace(mesh, &f[1], &e[2], &e[3], &e[4]), 0);
    assertNotEquals(borMesh3AddFace(mesh, &f2, &e[2], &e[3], &e[4]), 0);
    assertEquals(borMesh3FacesLen(mesh), 2);

    assertTrue(borMesh3EdgeHasVertex(&e[0], &v[0]));
    assertTrue(borMesh3EdgeHasVertex(&e[0], &v[1]));
    assertTrue(borMesh3EdgeHasFace(&e[0], &f[0]));
    assertFalse(borMesh3EdgeHasFace(&e[0], &f[1]));
    assertTrue(borMesh3EdgeHasFace(&e[2], &f[0]));
    assertTrue(borMesh3EdgeHasFace(&e[2], &f[1]));
    assertEquals(borMesh3EdgeFacesLen(&e[2]), 2);
    assertEquals(borMesh3EdgeFacesLen(&e[0]), 1);
    assertTrue(borMesh3FaceHasEdge(&f[0], &e[2]));

    borMesh3DumpSVT(mesh, stdout, "Mesh 1");
    borMesh3DumpPovray(mesh, pov);
    borMesh3DumpTriangles(mesh, tri);

    assertNotEquals(borMesh3RemoveEdge(mesh, &e[2]), 0);
    assertNotEquals(borMesh3RemoveEdge(mesh, &e[0]), 0);
    assertEquals(borMesh3VerticesLen(mesh), 4);
    assertEquals(borMesh3EdgesLen(mesh), 5);
    assertEquals(borMesh3FacesLen(mesh), 2);


    assertNotEquals(borMesh3RemoveVertex(mesh, &v[2]), 0);
    assertNotEquals(borMesh3RemoveVertex(mesh, &v[0]), 0);
    assertEquals(borMesh3VerticesLen(mesh), 4);
    assertEquals(borMesh3EdgesLen(mesh), 5);
    assertEquals(borMesh3FacesLen(mesh), 2);

    borMesh3RemoveFace(mesh, &f[0]);

    borMesh3DumpSVT(mesh, stdout, "Mesh 1.2");
    borMesh3DumpPovray(mesh, pov);
    borMesh3DumpTriangles(mesh, tri);

    assertEquals(borMesh3VerticesLen(mesh), 4);
    assertEquals(borMesh3EdgesLen(mesh), 5);
    assertEquals(borMesh3FacesLen(mesh), 1);
    borMesh3RemoveFace(mesh, &f[1]);
    assertEquals(borMesh3FacesLen(mesh), 0);

    borMesh3DumpSVT(mesh, stdout, "Mesh 1.3");
    borMesh3DumpPovray(mesh, pov);
    borMesh3DumpTriangles(mesh, tri);

    assertEquals(borMesh3RemoveEdge(mesh, &e[0]), 0);
    assertEquals(borMesh3VerticesLen(mesh), 4);
    assertEquals(borMesh3EdgesLen(mesh), 4);
    assertEquals(borMesh3FacesLen(mesh), 0);
    assertEquals(borMesh3RemoveEdge(mesh, &e[1]), 0);
    assertEquals(borMesh3RemoveEdge(mesh, &e[2]), 0);
    assertEquals(borMesh3RemoveVertex(mesh, &v[0]), 0);
    assertEquals(borMesh3VerticesLen(mesh), 3);
    assertEquals(borMesh3EdgesLen(mesh), 2);
    assertEquals(borMesh3FacesLen(mesh), 0);
    assertEquals(borMesh3RemoveEdge(mesh, &e[3]), 0);
    assertEquals(borMesh3RemoveEdge(mesh, &e[4]), 0);
    assertEquals(borMesh3VerticesLen(mesh), 3);
    assertEquals(borMesh3EdgesLen(mesh), 0);
    assertEquals(borMesh3FacesLen(mesh), 0);

    borMesh3DumpSVT(mesh, stdout, "Mesh 1 three points");
    borMesh3Del(mesh);
}


static void testMesh2DelV(bor_mesh3_vertex_t *v, void *data)
{
    borVec3Del(borMesh3VertexCoordsW(v));
    borMesh3VertexDel(v);
    assertEquals(*(long *)data, 123);
}
static void testMesh2DelE(bor_mesh3_edge_t *e, void *data)
{
    borMesh3EdgeDel(e);
    assertEquals(*(long *)data, 345);
}
static void testMesh2DelF(bor_mesh3_face_t *f, void *data)
{
    borMesh3FaceDel(f);
    assertEquals(*(long *)data, 456);
}

TEST(testMesh2)
{
    bor_mesh3_t *mesh;
    bor_vec3_t *vcoords[4];
    bor_mesh3_vertex_t *v[4];
    bor_mesh3_edge_t *e[5];
    bor_mesh3_face_t *f[2], f2;
    size_t i;
    long vdata = 123, edata = 345, fdata = 456;

    mesh = borMesh3New();

    for (i = 0; i < 4; i++){
        vcoords[i] = borVec3New(0., 0., 0.);
        v[i] = borMesh3VertexNew();
        borMesh3VertexSetCoords(v[i], vcoords[i]);
    }
    for (i = 0; i < 5; i++)
        e[i] = borMesh3EdgeNew();
    for (i = 0; i < 2; i++)
        f[i] = borMesh3FaceNew();


    borVec3Set(borMesh3VertexCoordsW(v[0]), 0., 0., 0.);
    borVec3Set(borMesh3VertexCoordsW(v[1]), 1., 0., 0.);
    borVec3Set(borMesh3VertexCoordsW(v[2]), 0., 1., 0.);
    borVec3Set(borMesh3VertexCoordsW(v[3]), 1., 1., 1.);

    borMesh3AddVertex(mesh, v[0]);
    borMesh3AddVertex(mesh, v[1]);
    borMesh3AddVertex(mesh, v[2]);
    borMesh3AddVertex(mesh, v[3]);
    assertEquals(borMesh3RemoveVertex(mesh, v[2]), 0);
    borMesh3AddVertex(mesh, v[2]);
    assertEquals(borMesh3VerticesLen(mesh), 4);

    borMesh3AddEdge(mesh, e[0], v[0], v[1]);
    borMesh3AddEdge(mesh, e[1], v[0], v[2]);
    borMesh3AddEdge(mesh, e[2], v[1], v[2]);
    borMesh3AddEdge(mesh, e[3], v[1], v[3]);
    borMesh3AddEdge(mesh, e[4], v[2], v[3]);
    assertEquals(borMesh3RemoveEdge(mesh, e[4]), 0);
    borMesh3AddEdge(mesh, e[4], v[2], v[3]);
    assertEquals(borMesh3EdgesLen(mesh), 5);

    assertTrue(borMesh3EdgeTriCheck(e[0], e[1], e[2]));
    assertTrue(borMesh3EdgeTriCheck(e[2], e[3], e[4]));
    assertEquals(borMesh3AddFace(mesh, f[0], e[0], e[1], e[2]), 0);
    assertEquals(borMesh3AddFace(mesh, f[1], e[2], e[3], e[4]), 0);
    assertNotEquals(borMesh3AddFace(mesh, &f2, e[2], e[3], e[4]), 0);
    assertEquals(borMesh3FacesLen(mesh), 2);

    borMesh3DumpSVT(mesh, stdout, "Mesh 2");


    assertNotEquals(borMesh3RemoveEdge(mesh, e[2]), 0);
    assertNotEquals(borMesh3RemoveEdge(mesh, e[0]), 0);
    assertEquals(borMesh3VerticesLen(mesh), 4);
    assertEquals(borMesh3EdgesLen(mesh), 5);
    assertEquals(borMesh3FacesLen(mesh), 2);

    assertNotEquals(borMesh3RemoveVertex(mesh, v[2]), 0);
    assertNotEquals(borMesh3RemoveVertex(mesh, v[0]), 0);
    assertEquals(borMesh3VerticesLen(mesh), 4);
    assertEquals(borMesh3EdgesLen(mesh), 5);
    assertEquals(borMesh3FacesLen(mesh), 2);

    borMesh3RemoveFace(mesh, f[0]);
    assertEquals(borMesh3VerticesLen(mesh), 4);
    assertEquals(borMesh3EdgesLen(mesh), 5);
    assertEquals(borMesh3FacesLen(mesh), 1);

    borMesh3Del2(mesh, testMesh2DelV, &vdata,
                       testMesh2DelE, &edata,
                       testMesh2DelF, &fdata);
    borMesh3FaceDel(f[0]);
}

#if 0
TEST(testMeshDumpInv){
    mesh_t *mesh;
    node_t *n1, *n2, *n3, *n4, *n5;
    edge_t *e1, *e2, *e3, *e4, *e5, *e6, *e7;
    face_t *f1, *f2, *f3;

    mesh = meshCreate();

    n1 = (node_t *)meshCreateNewNode(mesh, 0, 0, 0);
    n2 = (node_t *)meshCreateNewNode(mesh, 1, 0, 0);
    n3 = (node_t *)meshCreateNewNode(mesh, 1, 1, 1);
    n4 = (node_t *)meshCreateNewNode(mesh, 0, 1, 1);
    n5 = (node_t *)meshCreateNewNode(mesh, 0.5, 1.5, 0.5);

    e1 = edgeCreate(n1, n2);
    serviceEdgeInit(e1);
    e2 = edgeCreate(n2, n3);
    serviceEdgeInit(e2);
    e3 = edgeCreate(n1, n3);
    serviceEdgeInit(e3);
    e4 = edgeCreate(n3, n4);
    serviceEdgeInit(e4);
    e5 = edgeCreate(n4, n1);
    serviceEdgeInit(e5);
    e6 = edgeCreate(n1, n5);
    serviceEdgeInit(e6);
    e7 = edgeCreate(n2, n5);
    serviceEdgeInit(e7);

    f1 = faceCreate(e1, e2, e3);
    serviceFaceInit(f1);
    f2 = faceCreate(e3, e4, e5);
    serviceFaceInit(f2);
    f3 = faceCreate(e1, e6, e7);
    serviceFaceInit(f3);

    assertEquals(meshNumNodes(mesh), 5);

    meshDumpInv(mesh, stdout);
    fprintf(stdout, "\n====================================\n");

    meshDumpInv(mesh, stdout);
    fprintf(stdout, "\n====================================\n");

    meshClean(mesh);
    assertEquals(meshRemove(mesh), 0);
}

void testMeshTravNode(node_t *node, void *ignore)
{
    const vector_t *coords = nodeGetCoords(node);
    fprintf(stdout, "node: id(%ld) - %f %f %f\n",
            serviceNodeItem(node, mesh_traverse)->visited,
            coords->x,
            coords->y,
            coords->z);
}
void testMeshTravEdge(edge_t *edge, void *ignore)
{
    fprintf(stdout, "edge: id(%ld) - node(%ld, %ld)\n",
            serviceEdgeItem(edge, mesh_traverse)->visited,
            serviceNodeItem(edge->node1, mesh_traverse)->visited,
            serviceNodeItem(edge->node2, mesh_traverse)->visited);
}
void testMeshTravFace(face_t *face, void *ignore)
{
    fprintf(stdout, "face: id(%ld) - edge(%ld, %ld, %ld)\n",
            serviceFaceItem(face, mesh_traverse)->visited,
            serviceEdgeItem(face->edge1, mesh_traverse)->visited,
            serviceEdgeItem(face->edge2, mesh_traverse)->visited,
            serviceEdgeItem(face->edge3, mesh_traverse)->visited);
}

TEST(testMeshTraverse)
{
    mesh_t *mesh;
    node_t *n1, *n2, *n3, *n4;
    edge_t *e1, *e2, *e3, *e4, *e5;
    face_t *f1, *f2;

    mesh = meshCreate();

    n1 = (node_t *)meshCreateNewNode(mesh, 0, 0, 0);
    n2 = (node_t *)meshCreateNewNode(mesh, 1, 0, 0);
    n3 = (node_t *)meshCreateNewNode(mesh, 1, 1, 1);
    n4 = (node_t *)meshCreateNewNode(mesh, 0, 1, 1);

    e1 = edgeCreate(n1, n2);
    serviceEdgeInit(e1);
    e2 = edgeCreate(n2, n3);
    serviceEdgeInit(e2);
    e3 = edgeCreate(n1, n3);
    serviceEdgeInit(e3);
    e4 = edgeCreate(n3, n4);
    serviceEdgeInit(e4);
    e5 = edgeCreate(n4, n1);
    serviceEdgeInit(e5);

    f1 = faceCreate(e1, e2, e3);
    serviceFaceInit(f1);
    f2 = faceCreate(e3, e4, e5);
    serviceFaceInit(f2);

    assertEquals(meshNumNodes(mesh), 4);

    // Traversing:
    meshTraverse(mesh,
                 testMeshTravNode, NULL,
                 testMeshTravEdge, NULL,
                 testMeshTravFace, NULL,
                 MESH_TRAVERSE_DEPTH_FIRST);
    meshTraverseClean(mesh);
    fprintf(stdout, "\n====================================\n");

    meshTraverse(mesh,
                 testMeshTravNode, NULL,
                 testMeshTravEdge, NULL,
                 testMeshTravFace, NULL,
                 MESH_TRAVERSE_DEPTH_FIRST);
    meshTraverseClean(mesh);
    fprintf(stdout, "\n====================================\n");

    meshClean(mesh);
    assertEquals(meshRemove(mesh), 0);
}


TEST(testMeshThreeNearsetNodes)
{
    mesh_t *mesh;
    mesh_node_t *n1, *n2, *n3, *n4;
    mesh_node_t *first, *second, *third;
    vector_t vertex;

    mesh = meshCreate();

    n1 = meshCreateNewNode(mesh, 0, 0, 0);
    n2 = meshCreateNewNode(mesh, 1, 0, 0);
    n3 = meshCreateNewNode(mesh, 1, 1, 1);
    n4 = meshCreateNewNode(mesh, 0, 1, 1);

    assertEquals(meshNumNodes(mesh), 4);

    vectorInitByCoords(&vertex, 0, 0, 0);
    assertEquals(meshThreeNearestNodes(mesh, &vertex,
                    &first, &second, &third), 0);
    assertEquals(first, n1);
    assertEquals(second, n2);
    assertEquals(third, n4);


    vectorInitByCoords(&vertex, 0.9, 1, 0.9);
    assertEquals(meshThreeNearestNodes(mesh, &vertex,
                    &first, &second, &third), 0);
    assertEquals(first, n3);
    assertEquals(second, n4);
    assertEquals(third, n2);


    // Only two nodes in mesh:
    assertEquals(meshRemoveNode(mesh, n4), 0);
    assertEquals(meshRemoveNode(mesh, n3), 0);

    vectorInitByCoords(&vertex, 1, 0, 0);
    assertEquals(meshThreeNearestNodes(mesh, &vertex,
                    &first, &second, &third), 2);
    assertEquals(first, n2);
    assertEquals(second, n1);

    
    // Only one node in mesh:
    assertEquals(meshRemoveNode(mesh, n2), 0);
    assertEquals(meshThreeNearestNodes(mesh, &vertex,
                    &first, &second, &third), 1);
    assertEquals(first, n1);


    assertEquals(meshRemoveNode(mesh, n1), 0);
    assertEquals(meshRemove(mesh), 0);
}

TEST(testMeshClean)
{
    mesh_t *mesh;
    node_t *n1, *n2, *n3, *n4, *n5;
    edge_t *e1, *e2, *e3, *e4, *e5, *e6, *e7;
    face_t *f1, *f2, *f3;

    mesh = meshCreate();

    n1 = (node_t *)meshCreateNewNode(mesh, 0, 0, 0);
    n2 = (node_t *)meshCreateNewNode(mesh, 1, 0, 0);
    n3 = (node_t *)meshCreateNewNode(mesh, 1, 1, 1);
    n4 = (node_t *)meshCreateNewNode(mesh, 0, 1, 1);
    n5 = (node_t *)meshCreateNewNode(mesh, 0.5, 1.5, 0.5);

    e1 = edgeCreate(n1, n2);
    serviceEdgeInit(e1);
    e2 = edgeCreate(n2, n3);
    serviceEdgeInit(e2);
    e3 = edgeCreate(n1, n3);
    serviceEdgeInit(e3);
    e4 = edgeCreate(n3, n4);
    serviceEdgeInit(e4);
    e5 = edgeCreate(n4, n1);
    serviceEdgeInit(e5);
    e6 = edgeCreate(n1, n5);
    serviceEdgeInit(e6);
    e7 = edgeCreate(n2, n5);
    serviceEdgeInit(e7);

    f1 = faceCreate(e1, e2, e3);
    serviceFaceInit(f1);
    f2 = faceCreate(e3, e4, e5);
    serviceFaceInit(f2);
    f3 = faceCreate(e1, e6, e7);
    serviceFaceInit(f3);

    assertEquals(meshNumNodes(mesh), 5);

    meshClean(mesh);
    assertEquals(meshRemove(mesh), 0);
}

TEST_SUITE(testSuiteMesh){
    TEST_ADD(testMesh),
    TEST_ADD(testMeshDumpInv),
    TEST_ADD(testMeshTraverse),
    TEST_ADD(testMeshThreeNearsetNodes),
    TEST_ADD(testMeshClean),
    TEST_SUITE_CLOSURE
};
#endif
