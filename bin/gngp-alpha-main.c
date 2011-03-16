#include <fermat/gng-plan.h>
#include <fermat/trimesh.h>
#include <fermat/timer.h>
#include <fermat/rand-mt.h>

struct _params_t {
    size_t max_nodes;
    size_t find_path;
    size_t warm_start;
    fer_gngp_t *gng;
    fer_timer_t timer;

    fer_vec_t *start, *goal;

    fer_rand_mt_t *rand;
    fer_vec_t *is;

    unsigned long evals;

    fer_trimesh_t *obs, *robot;
    
    const fer_real_t *aabb;
};
typedef struct _params_t params_t;


static int terminate(void *data);
static void callback(void *data);
static const fer_vec_t *inputSignal(void *data);
static int eval(const fer_vec_t *w, void *data);
static void printPath(fer_list_t *path, FILE *out);

int main(int argc, char *argv[])
{
    params_t p;
    fer_gngp_params_t params;
    fer_gngp_ops_t ops;
    fer_real_t aabb[12] = { -30, 30, -30, 30, -30, 30,
                            -M_PI_2, M_PI_2,
                            -M_PI_2, M_PI_2,
                            -M_PI_2, M_PI_2 };

    if (argc < 4){
        fprintf(stderr, "Usage: %s max_nodes warm_start find_path_period\n", argv[0]);
        return -1;
    }

    ferGNGPParamsInit(&params);
    params.d = 6;
    params.lambda = 200;
    params.warm_start = atoi(argv[2]);
    params.cells.num_cells = 0;
    params.cells.max_dens  = 1;
    params.cells.expand_rate = 1.5;
    params.cells.aabb = aabb;

    ferGNGPOpsInit(&ops);
    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.eval         = eval;
    ops.callback  = callback;
    ops.callback_period = 500;
    ops.data = &p;

    p.evals = 0L;
    p.max_nodes = atoi(argv[1]);
    p.warm_start = params.warm_start;
    p.find_path = atoi(argv[3]);
    p.aabb = aabb;
    p.start = ferVecNew(6);
    p.goal = ferVecNew(6);
    p.is = ferVecNew(6);

    ferVecSet(p.start, 0, 10.);
    ferVecSet(p.start, 1, 15.);
    ferVecSet(p.start, 2, 3.);
    ferVecSet(p.start, 3, M_PI_2);
    ferVecSet(p.start, 4, 0.);
    ferVecSet(p.start, 5, 0.);

    ferVecSet(p.goal, 0, 10.);
    ferVecSet(p.goal, 1, 15.);
    ferVecSet(p.goal, 2, 23.);
    ferVecSet(p.goal, 3, M_PI_2);
    ferVecSet(p.goal, 4, 0.);
    ferVecSet(p.goal, 5, 0.);

    p.obs   = ferTriMeshNew();
    p.robot = ferTriMeshNew();
    ferTriMeshLoad(p.obs, "alpha-1.5.mesh");
    ferTriMeshLoad(p.robot, "alpha-1.5.mesh");

    p.rand = ferRandMTNewAuto();

    p.gng = ferGNGPNew(&ops, &params);

    ferTimerStart(&p.timer);
    callback(&p);
    ferGNGPRun(p.gng);
    callback(&p);
    fprintf(stderr, "\n");
    fprintf(stderr, "Evals: %lu\n", p.evals);

    ferGNGPDel(p.gng);



    /*
    {
    fer_vec3_t axis, pos;
    fer_quat_t rot, rot2;
    ferVec3Set(&axis, 1, 0, 0);
    ferQuatSetAngleAxis(&rot, M_PI_2, &axis);
    ferTriMeshSetRot(p.robot, &rot);

    ferVec3Set(&pos, 0., 0., .);
    ferTriMeshSetPos(p.robot, &pos);

    fprintf(stderr, "%d\n", ferTriMeshCollide(p.obs, p.robot));
    ferTriMeshDumpSVT(p.obs, stdout, "OBS");
    ferTriMeshDumpSVT(p.robot, stdout, "Robot");
    }
    */



    ferTriMeshDel(p.obs);
    ferTriMeshDel(p.robot);
    ferRandMTDel(p.rand);
    ferVecDel(p.is);
    ferVecDel(p.start);
    ferVecDel(p.goal);

    return 0;
}


static int terminate(void *data)
{
    params_t *p = (params_t *)data;
    int res;
    fer_list_t path;

    if (ferGNGPNodesLen(p->gng) > p->warm_start
            && ferGNGPNodesLen(p->gng) % p->find_path == 0){
        ferListInit(&path);
        ferVecPrint(6, p->start, stderr);
        fprintf(stderr, "\n");
        res = ferGNGPFindPath(p->gng, p->start, p->goal, &path);
        if (res == 0){
            fprintf(stderr, "\n");
            fprintf(stderr, "Path found. Nodes: %d\n",
                    (int)ferGNGPNodesLen(p->gng));
            printPath(&path, stdout);
            return 1;
        }
    }

#if 0
    if (ferGNGPNodesLen(p->gng) % p->warm_start == 0){
        char fn[1000];
        FILE *out;

        sprintf(fn, "net-%08d", (int)ferGNGPNodesLen(p->gng));
        out = fopen(fn, "w");
        ferGNGPDumpNodes(p->gng, out);
        fclose(out);
    }
#endif

    return ferGNGPNodesLen(p->gng) >= p->max_nodes;
}

static void callback(void *data)
{
    params_t *p = (params_t *)data;
    size_t nodes_len;
    /*
    FILE *fout;
    char fn[1000];

    nodes_len = ferGNGPNodesLen(p->gng);

    sprintf(fn, "out/%010d.svt", ferGNGPNodesLen(p->gng));
    fout = fopen(fn, "w");
    ferGNGPDumpSVT(p->gng, fout, NULL);
    fclose(fout);
    */

    nodes_len = ferGNGPNodesLen(p->gng);
    ferTimerStopAndPrintElapsed(&p->timer, stderr, " n: %d / %d (%lu)\r",
                                nodes_len, p->max_nodes, p->evals);
}

static const fer_vec_t *inputSignal(void *data)
{
    params_t *p = (params_t *)data;
    fer_real_t val;
    size_t i;

    for (i = 0; i < 6; i++){
        val = ferRandMT(p->rand, p->aabb[2 * i], p->aabb[2 * i + 1]);
        ferVecSet(p->is, i, val);
    }

    return p->is;
}

static int eval(const fer_vec_t *w, void *data)
{
    params_t *p = (params_t *)data;
    fer_vec3_t axis, pos;
    fer_quat_t rot, rot2;

    ferVec3Set(&axis, 1, 0, 0);
    ferQuatSetAngleAxis(&rot, ferVecGet(w, 3), &axis);
    ferVec3Set(&axis, 0, 1, 0);
    ferQuatSetAngleAxis(&rot2, ferVecGet(w, 4), &axis);
    ferQuatMul(&rot, &rot2);
    ferVec3Set(&axis, 0, 0, 1);
    ferQuatSetAngleAxis(&rot2, ferVecGet(w, 5), &axis);
    ferQuatMul(&rot, &rot2);
    ferTriMeshSetRot(p->robot, &rot);

    ferVec3Set(&pos, ferVecGet(w, 0), ferVecGet(w, 1), ferVecGet(w, 2));
    ferTriMeshSetPos(p->robot, &pos);

    p->evals += 1L;

    if (ferTriMeshCollide(p->obs, p->robot) == 0)
        return FER_GNGP_FREE;
    return FER_GNGP_OBST;
}

static void printPath(fer_list_t *path, FILE *out)
{
    fer_list_t *item;
    fer_gngp_node_t *n;

    ferListForEach(path, item){
        n = ferListEntry(item, fer_gngp_node_t, path);
        ferVecPrint(6, n->w, out);
        fprintf(out, "\n");
    }
}
