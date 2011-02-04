#include <fermat/mesh/qhull.h>
#include <fermat/timer.h>

int main(int argc, char *argv[])
{
    fer_timer_t timer;
    fer_qdelaunay_t *q;
    fer_qhull_mesh3_t *mesh;
    fer_pc_t *pc;
    size_t len;

    if (argc != 2){
        fprintf(stderr, "Usage: %s infile >out.svt\n", argv[0]);
        return -1;
    }

    pc = ferPCNew();
    len = ferPCAddFromFile(pc, argv[1]);
    fprintf(stderr, "Read %d points from %s\n", len, argv[1]);

    q = ferQDelaunayNew();

    ferTimerStart(&timer);
    mesh = ferQDelaunayMesh3(q, pc);
    if (!mesh){
        fprintf(stderr, "No mesh!\n");
    }
    ferTimerStopAndPrintElapsed(&timer, stderr, " Generated mesh %d vertices, %d edges, %d faces\n",
                                ferMesh3VerticesLen(ferQHullMesh3(mesh)),
                                ferMesh3EdgesLen(ferQHullMesh3(mesh)),
                                ferMesh3FacesLen(ferQHullMesh3(mesh)));


    ferQDelaunayDel(q);

    ferMesh3DumpSVT(ferQHullMesh3(mesh), stdout, NULL);
    ferQHullMesh3Del(mesh);
    ferPCDel(pc);

    return 0;
}
