#include <stdio.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/rand-mt.h>
#include <fermat/gng-plan2.h>
#include <fermat/dij.h>
#include <fermat/alloc.h>
#include <fermat/vec2.h>
#include <fermat/vec3.h>
#include <fermat/mat3.h>
#include <fermat/alloc.h>
#include <fermat/cd.h>

#define FREE FER_GNG_PLAN_FREE
#define OBST FER_GNG_PLAN_OBST

struct _plan_2_3_tri_t {
    fer_vec2_t p[3];
    fer_list_t list;
};
typedef struct _plan_2_3_tri_t plan_2_3_tri_t;

struct _plan_2_3_obj_t {
    fer_list_t tris;
    fer_cd_geom_t *g;
};
typedef struct _plan_2_3_obj_t plan_2_3_obj_t;

struct _plan_2_3_t {
    int dim;
    fer_real_t aabb[12];
    fer_vec_t *start, *goal;

    fer_real_t max_dist;
    fer_real_t min_dist;
    size_t max_nodes;
    int lambda;
    int age_max;

    fer_cd_t *cd;
    plan_2_3_obj_t robot;
    plan_2_3_obj_t obst;
};
typedef struct _plan_2_3_t plan_2_3_t;

plan_2_3_t *plan;

plan_2_3_t *planNew(const char *fn);
void planDel(plan_2_3_t *plan);

fer_gng_plan_t *gng;
fer_gng_plan_ops_t ops;
fer_gng_plan_params_t params;
fer_rand_mt_t *rand_mt;
fer_timer_t timer;
fer_vec_t *is;

int dump_period = 10;
const char *dump_prefix = NULL;
int avg_edge_len = 1;

const char *scene;

static int terminate(void *data);
static void callback(void *data);
static const void *inputSignal(void *data);
static int eval(const fer_vec_t *w, void *data);

static void dump(int last);
static void dumpScene(FILE *out);
static void dumpObj(plan_2_3_obj_t *obj, const fer_vec_t *w, FILE *out);
static void dumpRobotPath(FILE *out);


int main(int argc, char *argv[])
{
    if (argc < 2){
        fprintf(stderr, "Usage: %s scene [dump_prefix dump_period]\n", argv[0]);
        return -1;
    }
    if (argc >= 3)
        dump_prefix = argv[2];
    if (argc >= 4)
        dump_period = atoi(argv[3]);

    is = ferVecNew(3);

    plan = planNew(argv[1]);

    ferGNGPlanOpsInit(&ops);
    ferGNGPlanParamsInit(&params);

    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.eval         = eval;
    ops.callback     = callback;
    ops.callback_period = 100;

    params.dim = plan->dim;
    params.max_dist = plan->max_dist;
    params.min_dist = plan->min_dist;
    params.min_nodes = 100;
    params.start = plan->start;
    params.goal  = plan->goal;
    params.gug.dim = plan->dim;
    params.gug.aabb = plan->aabb;
    params.gug.max_dens = 0.1;
    params.gug.expand_rate = 1.1;
    params.gng.lambda = plan->lambda;
    params.gng.age_max = plan->age_max;


    rand_mt = ferRandMTNewAuto();
    gng = ferGNGPlanNew(&ops, &params);

    ferTimerStart(&timer);
    ferGNGPlanRun(gng);
    ferTimerStopAndPrintElapsed(&timer, stderr, "\r\n");

    if (dump_prefix){
        dump(1);
    }
    ferGNGPlanDumpNetSVT(gng, stdout, NULL);
    ferGNGPlanDumpObstSVT(gng, stdout, NULL);
    ferGNGPlanDumpPathSVT(gng, stdout, NULL);
    dumpScene(stdout);
    dumpRobotPath(stdout);


    ferGNGPlanDel(gng);

    ferRandMTDel(rand_mt);
    
    ferVecDel(is);

    planDel(plan);

    return 0;
}


static int terminate(void *data)
{
    size_t nodes_len;
    static int counter = 0;

    if (dump_prefix && ++counter == dump_period){
        dump(0);
        counter = 0;
    }

    nodes_len = ferGNGNodesLen(ferGNGPlanGNG(gng));
    return nodes_len >= plan->max_nodes;
}

static void callback(void *data)
{
    size_t nodes_len;

    nodes_len = ferGNGNodesLen(ferGNGPlanGNG(gng));
    ferTimerStop(&timer);
    if (avg_edge_len){
        fprintf(stderr, "%010lu, nodes: %08d, evals: %010lu, avg edge len: %f\n",
                ferTimerElapsedInMs(&timer),
                (int)nodes_len, ferGNGPlanEvals(gng),
                (float)ferGNGPlanAvgEdgeLen(gng));
    }else{
        fprintf(stderr, "%lu, nodes: %08d, evals: %010lu\n",
                ferTimerElapsedInMs(&timer),
                (int)nodes_len, ferGNGPlanEvals(gng));
    }
}

static const void *inputSignal(void *data)
{
    int i;

    for (i = 0; i < plan->dim; i++){
        ferVecSet(is, i, ferRandMT(rand_mt, plan->aabb[2 * i], plan->aabb[2 * i + 1]));
    }
    return is;
}

static int eval(const fer_vec_t *w, void *data)
{
    fer_mat3_t rot;
    fer_vec3_t pos;

    fprintf(stderr, "eval\n");
    if (plan->dim == 3){
        ferMat3SetRot3D(&rot, FER_ZERO, FER_ZERO, ferVecGet(w, 2));
        ferVec3Set(&pos, ferVecGet(w, 0), ferVecGet(w, 1), FER_ZERO);
        ferCDGeomSetRot(plan->cd, plan->robot.g, &rot);
        ferCDGeomSetTr(plan->cd, plan->robot.g, &pos);
    }else if (plan->dim == 2){
        ferVec3Set(&pos, ferVecGet(w, 0), ferVecGet(w, 1), FER_ZERO);
        ferCDGeomSetTr(plan->cd, plan->robot.g, &pos);
    }else if (plan->dim == 6){
        ferMat3SetRot3D(&rot, ferVecGet(w, 3), ferVecGet(w, 4), ferVecGet(w, 5));
        ferVec3Set(&pos, ferVecGet(w, 0), ferVecGet(w, 1), ferVecGet(w, 2));
        ferCDGeomSetRot(plan->cd, plan->robot.g, &rot);
        ferCDGeomSetTr(plan->cd, plan->robot.g, &pos);
    }

    if (ferCDGeomCollide(plan->cd, plan->robot.g, plan->obst.g))
        return OBST;
    return FREE;
}



static void dump(int last)
{
    FILE *fout;
    char fn[1000];
    static int counter = 0;

    sprintf(fn, "%s%010d.svt", dump_prefix, counter++);
    fout = fopen(fn, "w");
    if (fout){
        dumpScene(fout);
        ferGNGPlanDumpNetSVT(gng, fout, NULL);
        ferGNGPlanDumpObstSVT(gng, fout, NULL);
        ferGNGPlanDumpPathSVT(gng, fout, NULL);
        if (last){
            dumpRobotPath(fout);
        }

        fclose(fout);
    }
}

static void dumpScene(FILE *out)
{
    fer_list_t *item_tri;
    plan_2_3_tri_t *tri;
    int i, size;

    if (plan->dim > 3)
        return;

    fprintf(out, "----\n");
    fprintf(out, "Face color: 0.7 0.7 0.7\n");
    fprintf(out, "Point color: 0.7 0.7 0.7\n");
    fprintf(out, "Name: Scene\n");
    fprintf(out, "Points:\n");

    size = 0;
    FER_LIST_FOR_EACH(&plan->obst.tris, item_tri){
        tri = FER_LIST_ENTRY(item_tri, plan_2_3_tri_t, list);

        ferVec2Print(&tri->p[0], out);
        fprintf(out, " 0 \n");
        ferVec2Print(&tri->p[1], out);
        fprintf(out, " 0 \n");
        ferVec2Print(&tri->p[2], out);
        fprintf(out, " 0 \n");

        ++size;
    }

    fprintf(out, "Faces:\n");
    for (i = 0; i < size; i++){
        fprintf(out, "%d %d %d\n", 3 * i, 3 * i + 1, 3 * i + 2);
    }

    fprintf(out, "----\n");

}

static void dumpObj(plan_2_3_obj_t *obj, const fer_vec_t *w, FILE *out)
{
    fer_mat3_t tr;
    fer_vec2_t p[3];
    fer_list_t *item;
    plan_2_3_tri_t *tri;
    int i, size;

    if (plan->dim > 3)
        return;

    ferMat3SetRot(&tr, ferVecGet(w, 2));
    ferMat3Set1(&tr, 0, 2, ferVecGet(w, 0));
    ferMat3Set1(&tr, 1, 2, ferVecGet(w, 1));

    fprintf(out, "----\n");
    fprintf(out, "Edge color: 0.1 0.8 0.1\n");
    fprintf(out, "Point color: 0.1 0.8 0.1\n");
    fprintf(out, "Edge width: 1\n");
    fprintf(out, "Point size: 1\n");
    fprintf(out, "Name: Robot 1\n");
    fprintf(out, "Points:\n");

    size = 0;
    FER_LIST_FOR_EACH(&plan->robot.tris, item){
        tri = FER_LIST_ENTRY(item, plan_2_3_tri_t, list);
        ferMat3MulVec2(&p[0], &tr, &tri->p[0]);
        ferMat3MulVec2(&p[1], &tr, &tri->p[1]);
        ferMat3MulVec2(&p[2], &tr, &tri->p[2]);

        ferVec2Print(&p[0], out);
        fprintf(out, " 0 \n");
        ferVec2Print(&p[1], out);
        fprintf(out, " 0 \n");
        ferVec2Print(&p[2], out);
        fprintf(out, " 0 \n");
        size++;
    }

    fprintf(out, "Edges:\n");
    for (i = 0; i < size; i++){
        fprintf(out, "%d %d\n%d %d\n%d %d\n",
                     3 * i    , 3 * i + 1,
                     3 * i + 1, 3 * i + 2,
                     3 * i + 2, 3 * i);
    }
    fprintf(out, "----\n");
}

static void dumpRobotPath(FILE *out)
{
    fer_gng_plan_node_t *n;
    fer_list_t *item;

    fprintf(out, "# PATH: ");
    ferVecPrint(plan->dim, plan->start, out);
    fprintf(out, "\n");
    FER_LIST_FOR_EACH(&gng->path, item){
        n = FER_LIST_ENTRY(item, fer_gng_plan_node_t, path);
        fprintf(out, "# PATH: ");
        ferVecPrint(plan->dim, n->w, out);
        fprintf(out, "\n");
    }
    fprintf(out, "# PATH: ");
    ferVecPrint(plan->dim, plan->goal, out);
    fprintf(out, "\n");


    if (plan->dim > 3)
        return;

    dumpObj(&plan->robot, plan->start, out);
    FER_LIST_FOR_EACH(&gng->path, item){
        n = FER_LIST_ENTRY(item, fer_gng_plan_node_t, path);

        dumpObj(&plan->robot, n->w, out);
    }
    dumpObj(&plan->robot, plan->goal, out);
}



static char *planNextLine(FILE *fin, char *line, size_t len)
{
    char *ret;

    do {
        ret = fgets(line, len, fin);
    } while (ret != NULL && line[0] == '#');

    return ret;
}

static void planNextLineEmpty(FILE *fin, char *line, size_t len, const char *sec)
{
    char *l = planNextLine(fin, line, len);
    if (l[0] != '\n'){
        fprintf(stderr, "Invalid file format (%s).\n", sec);
        exit(-1);
    }
}

void planDump(const plan_2_3_t *plan)
{
    fer_list_t *item_tri;
    plan_2_3_tri_t *tri;
    int i;

    fprintf(stderr, "dim: %d\n", plan->dim);

    fprintf(stderr, "AABB:");
    for (i = 0; i < 2 * plan->dim; i++){
        fprintf(stderr, " %f", (float)plan->aabb[i]);
    }
    fprintf(stderr, "\n\n");

    fprintf(stderr, "start: ");
    ferVecPrint(plan->dim, plan->start, stderr);
    fprintf(stderr, "\ngoal:  ");
    ferVecPrint(plan->dim, plan->goal, stderr);
    fprintf(stderr, "\n\n");

    fprintf(stderr, "max dist:  %f\n", plan->max_dist);
    fprintf(stderr, "min dist:  %f\n", plan->min_dist);
    fprintf(stderr, "max nodes: %d\n", (int)plan->max_nodes);
    fprintf(stderr, "lambda:    %d\n", (int)plan->lambda);
    fprintf(stderr, "age max:   %d\n", (int)plan->age_max);
    fprintf(stderr, "\n");

    /*
    FER_LIST_FOR_EACH(&plan->robot.tris, item_tri){
        tri = FER_LIST_ENTRY(item_tri, plan_2_3_tri_t, list);
        fprintf(stderr, "robot: <%f %f, %f %f, %f %f>\n",
                ferVec2X(&tri->p[0]), ferVec2Y(&tri->p[0]),
                ferVec2X(&tri->p[1]), ferVec2Y(&tri->p[1]),
                ferVec2X(&tri->p[2]), ferVec2Y(&tri->p[2]));
    }
    fprintf(stderr, "\n");

    FER_LIST_FOR_EACH(&plan->obst.tris, item_tri){
        tri = FER_LIST_ENTRY(item_tri, plan_2_3_tri_t, list);

        fprintf(stderr, "obst: <%f %f, %f %f, %f %f>\n",
                ferVec2X(&tri->p[0]), ferVec2Y(&tri->p[0]),
                ferVec2X(&tri->p[1]), ferVec2Y(&tri->p[1]),
                ferVec2X(&tri->p[2]), ferVec2Y(&tri->p[2]));
    }
    */
}

static int scan2f(const char *line, float *f)
{
    return sscanf(line, "%f %f", f, f + 1) == 2;
}
static int scan3f(const char *line, float *f)
{
    return sscanf(line, "%f %f %f", f, f + 1, f + 2) == 3;
}
static int scan4f(const char *line, float *f)
{
    return sscanf(line, "%f %f %f %f", f, f + 1, f + 2, f + 3) == 4;
}
static int scan6f(const char *line, float *f)
{
    return sscanf(line, "%f %f %f %f %f %f", f, f + 1, f + 2, f + 3, f + 4, f + 5) == 6;
}
static int scan9f(const char *line, float *f)
{
    return sscanf(line, "%f %f %f %f %f %f %f %f %f", f, f + 1, f + 2, f + 3, f + 4, f + 5, f + 6, f + 7, f + 8) == 9;
}
static int scan12f(const char *line, float *f)
{
    return sscanf(line, "%f %f %f %f %f %f %f %f %f %f %f %f",
                  f, f + 1, f + 2, f + 3, f + 4, f + 5, f + 6, f + 7, f + 8, f + 9, f + 10, f + 11) == 12;
}

plan_2_3_t *planNew(const char *fn)
{
    fer_cd_params_t cdparams;
    plan_2_3_t *plan;
    plan_2_3_tri_t *tri;
    fer_vec3_t pts[3];
    FILE *fin;
    size_t __len = 1024;
    char __line[1024];
    char *line;
    float f[12];
    int i, d[3];

    if ((fin = fopen(fn, "r")) == NULL){
        perror("Can't open file");
        return NULL;
    }

    plan = FER_ALLOC(plan_2_3_t);
    plan->start = ferVecNew(6);
    plan->goal  = ferVecNew(6);

    ferCDParamsInit(&cdparams);
    cdparams.build_flags = FER_CD_FIT_NAIVE
                            | FER_CD_FIT_NAIVE_NUM_ROT(5)
                            | FER_CD_BUILD_PARALLEL(8);
    plan->cd = ferCDNew(&cdparams);

    // dim
    line = planNextLine(fin, __line, __len);
    if (!line || sscanf(line, "%d", d) != 1){
        fprintf(stderr, "Invalid file format (dim).\n");
        exit(-1);
    }
    plan->dim = d[0];
    planNextLineEmpty(fin, __line, __len, "dim");

    // AABB
    line = planNextLine(fin, __line, __len);
    if (!line
            || (plan->dim == 3 && !scan6f(line, f))
            || (plan->dim == 2 && !scan4f(line, f))
            || (plan->dim == 6 && !scan12f(line, f))
       ){
        fprintf(stderr, "Invalid file format (AABB).\n");
        exit(-1);
    }
    for (i = 0; i < 2 * plan->dim; i++){
        plan->aabb[i] = f[i];
    }
    planNextLineEmpty(fin, __line, __len, "AABB");

    // Start position
    line = planNextLine(fin, __line, __len);
    if (!line
            || (plan->dim == 3 && !scan3f(line, f))
            || (plan->dim == 2 && !scan2f(line, f))
            || (plan->dim == 6 && !scan6f(line, f))
        ){
        fprintf(stderr, "Invalid file format (start position).\n");
        exit(-1);
    }
    for (i = 0; i < plan->dim; i++)
        ferVecSet(plan->start, i, f[i]);
    planNextLineEmpty(fin, __line, __len, "start position");

    // Goal position
    line = planNextLine(fin, __line, __len);
    if (!line
            || (plan->dim == 3 && !scan3f(line, f))
            || (plan->dim == 2 && !scan2f(line, f))
            || (plan->dim == 6 && !scan6f(line, f))
        ){
        fprintf(stderr, "Invalid file format (goal position).\n");
        exit(-1);
    }
    for (i = 0; i < plan->dim; i++)
        ferVecSet(plan->goal, i, f[i]);
    planNextLineEmpty(fin, __line, __len, "goal position");

    // params
    line = planNextLine(fin, __line, __len);
    if (!line || sscanf(line, "%f %f %d %d %d", f, f + 1, d, d + 1, d + 2) != 5){
        fprintf(stderr, "Invalid file format (params).\n");
        exit(-1);
    }
    plan->max_dist = f[0];
    plan->min_dist = f[1];
    plan->max_nodes = d[0];
    plan->lambda    = d[1];
    plan->age_max   = d[2];
    planNextLineEmpty(fin, __line, __len, "params");

    // robot
    plan->robot.g = ferCDGeomNew(plan->cd);
    ferListInit(&plan->robot.tris);
    line = planNextLine(fin, __line, __len);
    while (line && line[0] != '\n'){
        if ((plan->dim <= 3 && !scan6f(line, f))
                || (plan->dim == 6 && !scan9f(line, f))
           ){
            fprintf(stderr, "Invalid file format (robot).\n");
            exit(-1);
        }

        tri = FER_ALLOC(plan_2_3_tri_t);
        ferVec2Set(&tri->p[0], f[0], f[1]);
        ferVec2Set(&tri->p[1], f[2], f[3]);
        ferVec2Set(&tri->p[2], f[4], f[5]);
        ferListAppend(&plan->robot.tris, &tri->list);

        if (plan->dim <= 3){
            ferVec3Set(&pts[0], f[0], f[1], FER_ZERO);
            ferVec3Set(&pts[1], f[2], f[3], FER_ZERO);
            ferVec3Set(&pts[2], f[4], f[5], FER_ZERO);
        }else if (plan->dim == 6){
            ferVec3Set(&pts[0], f[0], f[1], f[2]);
            ferVec3Set(&pts[0], f[3], f[4], f[5]);
            ferVec3Set(&pts[0], f[6], f[7], f[8]);
        }
        ferCDGeomAddTri(plan->cd, plan->robot.g, &pts[0], &pts[1], &pts[2]);

        line = planNextLine(fin, __line, __len);
    }

    // obstacle
    plan->obst.g = ferCDGeomNew(plan->cd);
    ferListInit(&plan->obst.tris);
    line = planNextLine(fin, __line, __len);
    while (line && line[0] != '\n'){
        if ((plan->dim <= 3 && !scan6f(line, f))
                || (plan->dim == 6 && !scan9f(line, f))
           ){
            fprintf(stderr, "Invalid file format (obst).\n");
            exit(-1);
        }

        tri = FER_ALLOC(plan_2_3_tri_t);
        ferVec2Set(&tri->p[0], f[0], f[1]);
        ferVec2Set(&tri->p[1], f[2], f[3]);
        ferVec2Set(&tri->p[2], f[4], f[5]);
        ferListAppend(&plan->obst.tris, &tri->list);

        if (plan->dim <= 3){
            ferVec3Set(&pts[0], f[0], f[1], FER_ZERO);
            ferVec3Set(&pts[1], f[2], f[3], FER_ZERO);
            ferVec3Set(&pts[2], f[4], f[5], FER_ZERO);
        }else if (plan->dim == 6){
            ferVec3Set(&pts[0], f[0], f[1], f[2]);
            ferVec3Set(&pts[0], f[3], f[4], f[5]);
            ferVec3Set(&pts[0], f[6], f[7], f[8]);
        }
        ferCDGeomAddTri(plan->cd, plan->obst.g, &pts[0], &pts[1], &pts[2]);

        line = planNextLine(fin, __line, __len);
    }

    fprintf(stderr, "Building robot...\n");
    ferCDGeomBuild(plan->cd, plan->robot.g);
    fprintf(stderr, "Building obstacle...\n");
    ferCDGeomBuild(plan->cd, plan->obst.g);

    planDump(plan);
    return plan;
}

void planDel(plan_2_3_t *plan)
{
    fer_list_t *item;
    plan_2_3_tri_t *tri;

    while (!ferListEmpty(&plan->robot.tris)){
        item = ferListNext(&plan->robot.tris);
        tri  = FER_LIST_ENTRY(item, plan_2_3_tri_t, list);
        ferListDel(item);
        free(tri);
    }

    while (!ferListEmpty(&plan->obst.tris)){
        item = ferListNext(&plan->obst.tris);
        tri  = FER_LIST_ENTRY(item, plan_2_3_tri_t, list);
        ferListDel(item);
        free(tri);
    }

    ferVecDel(plan->start);
    ferVecDel(plan->goal);

    free(plan);
}
