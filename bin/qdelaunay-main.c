#include <boruvka/qhull.h>
#include <boruvka/timer.h>

int main(int argc, char *argv[])
{
    bor_timer_t timer;
    bor_qdelaunay_t *q;
    bor_qhull_mesh3_t *mesh;
    bor_pc_t *pc;
    size_t len;

    if (argc != 2){
        fprintf(stderr, "Usage: %s infile >out.svt\n", argv[0]);
        return -1;
    }

    pc = borPCNew(3);
    len = borPCAddFromFile(pc, argv[1]);
    fprintf(stderr, "Read %d points from %s\n", (int)len, argv[1]);

    q = borQDelaunayNew();

    borTimerStart(&timer);
    mesh = borQDelaunayMesh3(q, pc);
    if (!mesh){
        fprintf(stderr, "No mesh!\n");
    }
    borTimerStopAndPrintElapsed(&timer, stderr, " Generated mesh %d vertices, %d edges, %d faces\n",
                                borMesh3VerticesLen(borQHullMesh3(mesh)),
                                borMesh3EdgesLen(borQHullMesh3(mesh)),
                                borMesh3FacesLen(borQHullMesh3(mesh)));


    borQDelaunayDel(q);

    borMesh3DumpSVT(borQHullMesh3(mesh), stdout, NULL);
    borQHullMesh3Del(mesh);
    borPCDel(pc);

    return 0;
}
