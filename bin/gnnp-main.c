/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
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

#include <fermat/gnnp.h>
#include <fermat/rand-mt.h>
#include <fermat/timer.h>
#include <fermat/cd.h>
#include <fermat/cfg.h>
#include <fermat/dbg.h>
#include <fermat/alloc.h>


fer_rand_mt_t *rnd;
fer_vec_t *is;
fer_vec_t *start, *goal;
//fer_real_t aabb[4] = {-5, 5, -5, 5};
unsigned long evals = 0UL;

static fer_cd_params_t cd_params;
static fer_cd_t *cd = NULL;
static fer_cd_geom_t *map = NULL;
static fer_cd_geom_t *robot = NULL;

static fer_cfg_t *cfg = NULL;
static fer_vec2_t minit, mgoal;
static fer_real_t aabb[4];
static fer_vec2_t *map_pts = NULL;
size_t pts_len;
static int *ids;
size_t ids_len;
static fer_vec3_t *robot_pts = NULL;
size_t robot_pts_len;
static unsigned int *robot_ids = NULL;
size_t robot_ids_len;

static int mapLoad(const char *fn)
{
    const char *name;
    size_t i;
    int ret;
    fer_vec3_t p[3];
    fer_real_t scale;
    fer_vec3_t m;
    const fer_real_t *_aabb;
    static fer_vec2_t *pts;

    cfg = ferCfgRead(fn);
    if (!cfg){
        fprintf(stderr, "Can't read cfg file `%s'\n", fn);
        return -1;
    }

    ret = ferCfgScan(cfg, "name:s aabb:f[] pts:v2[] pts:v2# ids:i[] ids:i# init:v2 goal:v2",
                     &name, &_aabb, &pts, &pts_len, &ids, &ids_len, &minit, &mgoal);
    if (ret != 0){
        ferCfgDel(cfg);
        fprintf(stderr, "Invalid format of cfg file.\n");
        return -1;
    }

    cd_params.build_flags = FER_CD_TOP_DOWN
                                | FER_CD_FIT_NAIVE
                                | FER_CD_FIT_NAIVE_NUM_ROT(5);
    ferCDParamsInit(&cd_params);
    cd_params.use_sap = 0;
    cd_params.num_threads = 1;
    cd = ferCDNew(&cd_params);

    ferVec3Set(&m, (_aabb[0] + _aabb[1]) / 2., (_aabb[2] + _aabb[3]) / 2., FER_ZERO);
    scale = FER_MAX(_aabb[1] - _aabb[0], _aabb[3] - _aabb[2]);
    scale = 10. / scale;
    aabb[0] = (_aabb[0] - ferVec3X(&m)) * scale;
    aabb[1] = (_aabb[1] - ferVec3X(&m)) * scale;
    aabb[2] = (_aabb[2] - ferVec3Y(&m)) * scale;
    aabb[3] = (_aabb[3] - ferVec3Y(&m)) * scale;

    ferVec2Sub(&minit, (const fer_vec2_t *)&m);
    ferVec2Scale(&minit, scale);
    ferVec2Sub(&mgoal, (const fer_vec2_t *)&m);
    ferVec2Scale(&mgoal, scale);

    map_pts = FER_ALLOC_ARR(fer_vec2_t, pts_len);
    for (i = 0; i < pts_len; i++){
        ferVec2Sub2(&map_pts[i], &pts[i], (fer_vec2_t *)&m);
        ferVec2Scale(&map_pts[i], scale);
    }

    map = ferCDGeomNew(cd);
    for (i = 0; (i + 2) < ids_len; i += 3){
        ferVec3Set(&p[0], ferVec2X(&pts[ids[i]]), ferVec2Y(&pts[ids[i]]), FER_ZERO);
        ferVec3Set(&p[1], ferVec2X(&pts[ids[i+1]]), ferVec2Y(&pts[ids[i+1]]), FER_ZERO);
        ferVec3Set(&p[2], ferVec2X(&pts[ids[i+2]]), ferVec2Y(&pts[ids[i+2]]), FER_ZERO);
        ferVec3Sub(&p[0], &m);
        ferVec3Sub(&p[1], &m);
        ferVec3Sub(&p[2], &m);
        ferVec3Scale(&p[0], scale);
        ferVec3Scale(&p[1], scale);
        ferVec3Scale(&p[2], scale);
        ferCDGeomAddTri(cd, map, &p[0], &p[1], &p[2]);
    }
    ferCDGeomBuild(cd, map);


    fprintf(stderr, "AABB: %f %f %f %f\n", (float)aabb[0], (float)aabb[1], (float)aabb[2], (float)aabb[3]);
    fprintf(stderr, "Init: %f %f\n", (float)ferVec2X(&minit), (float)ferVec2Y(&minit));
    fprintf(stderr, "Goal: %f %f\n", (float)ferVec2X(&mgoal), (float)ferVec2Y(&mgoal));

    return 0;
}

static void mapRobotSquare(fer_real_t he)
{
    robot_pts = ferVec3ArrNew(4);
    robot_pts_len = 4;
    robot_ids = FER_ALLOC_ARR(unsigned int, 6);
    robot_ids_len = 6;

    ferVec3Set(&robot_pts[0], -he, -he, FER_ZERO);
    ferVec3Set(&robot_pts[1],  he, -he, FER_ZERO);
    ferVec3Set(&robot_pts[2],  he,  he, FER_ZERO);
    ferVec3Set(&robot_pts[3], -he,  he, FER_ZERO);
    robot_ids[0] = 0;
    robot_ids[1] = 1;
    robot_ids[2] = 2;
    robot_ids[3] = 0;
    robot_ids[4] = 2;
    robot_ids[5] = 3;

    robot = ferCDGeomNew(cd);
    ferCDGeomAddTriMesh(cd, robot, robot_pts, robot_ids, robot_ids_len / 3);
    ferCDGeomBuild(cd, robot);
}

static void mapDel(void)
{
    if (cd)
        ferCDDel(cd);
    if (cfg)
        ferCfgDel(cfg);
    if (map_pts)
        FER_FREE(map_pts);
    if (robot_pts)
        FER_FREE(robot_pts);
    if (robot_ids)
        FER_FREE(robot_ids);
}

_fer_inline int mapCD(void)
{
    if (!cd || !map || !robot)
        return 0;
    return ferCDGeomCollide(cd, map, robot);
}

_fer_inline int mapCDConf(int dim, const fer_vec_t *conf)
{
    if (!robot)
        return 0;

    //DBG("%f %f", ferVecGet(conf, 0), ferVecGet(conf, 1));
    if (dim == 2){
        ferCDGeomSetTr3(cd, robot, ferVecGet(conf, 0), ferVecGet(conf, 1), FER_ZERO);
    }

    return mapCD();
}

static void mapDumpSVT(FILE *out, const char *name)
{
    size_t i;

    fprintf(out, "----\n");
    if (name){
        fprintf(out, "Name: %s\n", name);
    }else{
        fprintf(out, "Name: Map\n");
    }

    fprintf(out, "Points off: 1\n");
    fprintf(out, "Face color: 0.8 0.8 0.8\n");
    fprintf(out, "Points2d:\n");
    for (i = 0; i < pts_len; i++){
        ferVec2Print(&map_pts[i], out);
        fprintf(out, "\n");
    }

    fprintf(out, "Faces:\n");
    for (i = 0; (i + 2) < ids_len; i += 3){
        fprintf(out, "%d %d %d\n", (int)ids[i], (int)ids[i + 1], (int)ids[i + 2]);
    }

    fprintf(out, "----\n");
}




static int terminate(fer_gnnp_t *nn, void *data)
{
    static int count = 0;

    ++count;
    if (count == 10000000)
        return 1;
    return 0;
}

static const fer_vec_t *inputSignal(fer_gnnp_t *nn, void *data)
{
    ferVecSet(is, 0, ferRandMT(rnd, aabb[0], aabb[1]));
    ferVecSet(is, 1, ferRandMT(rnd, aabb[2], aabb[3]));
    return is;
}

static int eval(fer_gnnp_t *nn, const fer_vec_t *conf, void *data)
{
    evals += 1UL;
    return !mapCDConf(2, conf);
    fer_real_t x, y, r = 0.05;
    x = ferVecGet(conf, 0);
    y = ferVecGet(conf, 1);

    evals += 1UL;

    if (y < -2
            || (y < 4 && y > -2 && x > -r && x < r)
            || (y > 4 && x > -2 && x < 2)){
        return 1;
    }
    return 0;
}

static void callback(fer_gnnp_t *nn, void *data)
{
    static int c = 0;
    char fn[100];
    FILE *fout;
    size_t i;

    snprintf(fn, 100, "g-%06d", c);
    fout = fopen(fn, "w");
    if (fout){
        ferGNNPDumpSVT(nn, fout, NULL);
        mapDumpSVT(fout, NULL);
        fclose(fout);
    }


    fprintf(stderr, "step %d, nodes: %d, evals: %ld, max_depth: %d\n",
            c, (int)ferGNNPNodesLen(nn), (long)evals, (int)nn->max_depth);

    fprintf(stderr, "    depth:");
    for (i = 0; i < nn->depths_alloc; i++){
        if (nn->depths[i] != 0)
            fprintf(stderr, " %02d:%04d", (int)i, (int)nn->depths[i]);
    }
    fprintf(stderr, "\n");

    
    fprintf(stderr, "    free: %d\n", (int)nn->nodes_set[0].len);
    fprintf(stderr, "    obst: %d\n", (int)nn->nodes_set[1].len);

    c++;
}

static void printPath(fer_gnnp_t *nn, fer_list_t *path)
{
    fer_list_t *item;
    fer_gnnp_node_t *n;
    int i, len;

    printf("----\n");
    printf("Name: PATH\n");

    printf("Points off: 1\n");
    printf("Edge color: 1 0 0\n");
    printf("Edge width: 2\n");
    printf("Points:\n");
    len = 0;
    FER_LIST_FOR_EACH(path, item){
        n = FER_LIST_ENTRY(item, fer_gnnp_node_t, path);
        ferVecPrint(2, n->w, stdout);
        printf("\n");
        len++;
    }

    printf("Edges:\n");
    for (i = 0; i < len - 1; i++){
        printf("%d %d\n", i, i + 1);
    }

    printf("----\n");
    printf("----\n");
}

int main(int argc, char *argv[])
{
    fer_list_t path;
    int ret;
    fer_gnnp_params_t params;
    fer_gnnp_ops_t ops;
    fer_gnnp_t *nn;
    fer_timer_t timer;

    if (argc != 2){
        fprintf(stderr, "Usage: %s cfg_file\n", argv[0]);
        return -1;
    }

    if (mapLoad(argv[1]) != 0)
        return -1;
    //mapRobotSquare(0.5);
    mapRobotSquare(0.08);

    ferGNNPOpsInit(&ops);
    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.eval         = eval;
    ops.callback        = callback;
    ops.callback_period = 10000;

    ferGNNPParamsInit(&params);
    params.dim  = 2;
    params.rmax = 6;
    params.h    = 0.25;
    params.h    = 0.05;
    params.prune_delay = 500;
    params.tournament = 3;
    params.nn.type = FER_NN_GUG;
    params.nn.gug.max_dens = 1.;
    params.nn.gug.expand_rate = 1.3;
    params.nn.gug.aabb = aabb;

    //rnd = ferRandMTNewAuto();
    rnd = ferRandMTNew(1111);

    is    = ferVecNew(2);
    start = ferVecNew(2);
    goal  = ferVecNew(2);
    //ferVecSetN(2, start, -4., -4.);
    //ferVecSetN(2, goal, 1.5, 4.5);
    ferVecSetN(2, start, ferVec2X(&minit), ferVec2Y(&minit));
    ferVecSetN(2, goal, ferVec2X(&mgoal), ferVec2Y(&mgoal));
    /*
    mapDumpSVT(stdout, NULL);
    mapCDConf(2, start);
    ferCDGeomDumpSVT(robot, stdout, NULL);
    mapCDConf(2, goal);
    ferCDGeomDumpSVT(robot, stdout, NULL);
    return 0;
    */


    nn = ferGNNPNew(&ops, &params);

    ferTimerStart(&timer);
    ret = ferGNNPFindPath(nn, start, goal, &path);
    ferTimerStop(&timer);
    callback(nn, NULL);
    fprintf(stderr, "ret: %d\n", ret);
    fprintf(stderr, "evals: %ld\n", (long)evals);
    ferGNNPDumpSVT(nn, stdout, NULL);
    if (ret == 0){
        printPath(nn, &path);
    }
    mapDumpSVT(stdout, NULL);

    fprintf(stderr, "Time: %lu us\n", ferTimerElapsedInUs(&timer));


    ferGNNPDel(nn);


    ferVecDel(is);
    ferVecDel(start);
    ferVecDel(goal);

    ferRandMTDel(rnd);

    mapDel();

    return 0;
}
