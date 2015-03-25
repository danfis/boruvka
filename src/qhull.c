#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <boruvka/qhull.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>


/** Maximal length of line  */
#define BOR_QHULL_OUTPUT_LINE_MAX_LEN 1024

/** Creates new qhull mesh3 */
static bor_qhull_mesh3_t *borQHullMesh3New(size_t vertices);

/** Deletes dymanically allocated vertices, edges and faces */
static void mesh3DelVert(bor_mesh3_vertex_t *v, void *data);
static void mesh3DelEdge(bor_mesh3_edge_t *e, void *data);
static void mesh3DelFace(bor_mesh3_face_t *f, void *data);

/** Writes point cloud into fd in format qhull accepts.
 *  Returns 0 on success */
static int writePC33(bor_pc_t *pc, int fd);
/** Parses input from fd into Mesh3 */
static bor_qhull_mesh3_t *qdelaunayToMesh3(int fd);


void borQHullMesh3Del(bor_qhull_mesh3_t *m)
{
    if (m->mesh){
        borMesh3Del2(m->mesh, mesh3DelVert, NULL,
                              mesh3DelEdge, NULL,
                              mesh3DelFace, NULL);
    }

    if (m->vecs){
        borVec3ArrDel(m->vecs);
    }
    BOR_FREE(m);
}



bor_qdelaunay_t *borQDelaunayNew(void)
{
    bor_qdelaunay_t *q;

    q = BOR_ALLOC(bor_qdelaunay_t);

    q->bin_path = BOR_STRDUP(BOR_QDELAUNAY_BIN_PATH);

    return q;
}

void borQDelaunayDel(bor_qdelaunay_t *q)
{
    if (q->bin_path)
        BOR_FREE(q->bin_path);
    BOR_FREE(q);
}

void borQDelaunaySetPath(bor_qdelaunay_t *q, const char *path)
{
    if (q->bin_path)
        BOR_FREE(q->bin_path);
    q->bin_path = BOR_STRDUP(path);
}

bor_qhull_mesh3_t *borQDelaunayMesh3(bor_qdelaunay_t *q, const bor_pc_t *pc)
{
    int pipe_points[2];
    int pipe_result[2];
    int pid;
    bor_qhull_mesh3_t *mesh;

    // open pipes for communication between qdelaunay program and this
    // program
    if (pipe(pipe_points) != 0
            || pipe(pipe_result) != 0){
        ERR2("Can't open pipes.\n");
        return NULL;
    }

    // fork process - child process will run qdelaunay
    pid = fork();
    if (pid == 0){
        // child process

        // close write end of points pipe and read end of result pipe
        close(pipe_points[1]);
        close(pipe_result[0]);

        // duplicate read end of pipe_points to stdin
        if (dup2(pipe_points[0], 0) != 0){
            ERR2("Can't connect pipe to stdin!");
            exit(-1);
        }

        // and duplicate write end of pipe_result to stdout
        if (dup2(pipe_result[1], 1) != 1){
            perror("");
            ERR2("Can't connect pipe to stdout!");
            exit(-1);
        }

        execl(q->bin_path, q->bin_path, "o", NULL);

        perror("");
        ERR("execl(\"%s\", \"%s\", \"o\") failed!", q->bin_path, q->bin_path);
        exit(-1);

    }else if (pid < 0){
        ERR2("Fork failed!\n");
        close(pipe_points[0]);
        close(pipe_points[1]);
        close(pipe_result[0]);
        close(pipe_result[1]);
        return NULL;
    }

    // parent process

    // close read end of pipe_points
    close(pipe_points[0]);
    // and write end of pipe_result
    close(pipe_result[1]);

    // write points on qdelaunay stdin
    if (writePC33((bor_pc_t *)pc, pipe_points[1]) != 0){
        ERR2("Can't open write end of pipe using stdio.");
        close(pipe_points[1]);
    }

    // read result from qdelaunay
    mesh = qdelaunayToMesh3(pipe_result[0]);
    if (!mesh){
        ERR2("Can't open read end of pipe using stdio.");
        close(pipe_result[0]);
    }

    // wait for child process
    waitpid(pid, NULL, 0);

    return mesh;
}

static bor_qhull_mesh3_t *borQHullMesh3New(size_t vertices)
{
    bor_qhull_mesh3_t *m;

    m = BOR_ALLOC(bor_qhull_mesh3_t);

    m->mesh = borMesh3New();

    m->vecs = borVec3ArrNew(vertices);
    m->vecs_len = vertices;

    return m;
}

static void mesh3DelVert(bor_mesh3_vertex_t *v, void *data)
{
    BOR_FREE(v);
}

static void mesh3DelEdge(bor_mesh3_edge_t *e, void *data)
{
    BOR_FREE(e);
}

static void mesh3DelFace(bor_mesh3_face_t *f, void *data)
{
    BOR_FREE(f);
}

static int writePC33(bor_pc_t *pc, int fd)
{
    FILE *fout;
    bor_pc_it_t pcit;
    bor_vec3_t *v;

    fout = fdopen(fd, "w");
    if (!fout)
        return -1;

    // first print dimensions
    fprintf(fout, "3\n");

    // then write number of points
    fprintf(fout, "%d\n", (int)borPCLen(pc));

    // and write all points
    borPCItInit(&pcit, (bor_pc_t *)pc);
    while (!borPCItEnd(&pcit)){
        v = (bor_vec3_t *)borPCItGet(&pcit);
        fprintf(fout, "%g %g %g\n", borVec3X(v), borVec3Y(v), borVec3Z(v));

        borPCItNext(&pcit);
    }

    fclose(fout);

    return 0;
}

static bor_qhull_mesh3_t *qdelaunayToMesh3(int fd)
{
    FILE *fin;
    int vertices, faces, tmp;
    int i, j, k;
    double x, y, z, w;
    int id[4];
    bor_qhull_mesh3_t *qmesh;
    bor_mesh3_t *mesh;
    bor_mesh3_vertex_t **verts;
    bor_mesh3_vertex_t *vert;
    bor_mesh3_edge_t *edge;
    //bor_mesh3_face_t *face;

    fin = fdopen(fd, "r");
    if (!fin)
        return NULL;

    // first line is number of facets 
    if (fscanf(fin, "%d", &tmp) != 1){
        fclose(fin);
        return NULL;
    }

    // second line contains number of points, facets and ridges - the last
    // one can be ignored
    if (fscanf(fin, "%d %d %d", &vertices, &faces, &tmp) != 3){
        fclose(fin);
        return NULL;
    }

    // alloc mesh
    qmesh = borQHullMesh3New(vertices);
    mesh = borQHullMesh3(qmesh);

    // allocate index array for vertices
    verts = BOR_ALLOC_ARR(bor_mesh3_vertex_t *, vertices);

    // read points and create vertices
    for (i = 0; i < vertices; i++){
        if (fscanf(fin, "%lg %lg %lg %lg", &x, &y, &z, &w) != 4){
            break;
        }

        borVec3Set(&qmesh->vecs[i], x, y, z);

        vert = BOR_ALLOC(bor_mesh3_vertex_t);
        borMesh3VertexSetCoords(vert, &qmesh->vecs[i]);
        borMesh3AddVertex(mesh, vert);
        verts[i] = vert;

        //fprintf(stdout, "[%d] %lg %lg %lg\n", i, x, y, z);
    }

    // read tetrahedrons
    for (i = 0; i < faces; i++){
        if (fscanf(fin, "%d %d %d %d", &id[0], &id[1], &id[2], &id[3]) != 4){
            break;
        }

        // create edges
        for (j = 0; j < 3; j++){
            for (k = j + 1; k < 4; k++){
                // create new edge only if it is not already there
                edge = borMesh3VertexCommonEdge(verts[id[j]], verts[id[k]]);
                if (!edge){
                    edge = BOR_ALLOC(bor_mesh3_edge_t);
                    borMesh3AddEdge(mesh, edge, verts[id[j]], verts[id[k]]);
                }
            }
        }

        //fprintf(stdout, "[%d] %d %d %d %d\n", i, id[0], id[1], id[2], id[3]);
    }

    if (verts)
        BOR_FREE(verts);

    fclose(fin);
    return qmesh;
}
