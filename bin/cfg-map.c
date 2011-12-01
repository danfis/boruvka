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

#include <fermat/cfg.h>
#include <fermat/cd.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>
#include <fermat/vec.h>
#include <fermat/vec3.h>
#include <fermat/mat3.h>
#include <fermat/rand-mt.h>


static fer_cfg_t *cfg = NULL;
static fer_rand_mt_t *rnd = NULL;

static fer_cd_t *cd = NULL;
static fer_cd_geom_t *map = NULL;
static fer_cd_geom_t *robot = NULL;
static const char *robot_name = NULL;
static fer_real_t aabb[12];
static int dim;
static int conf_dim;
static FER_VEC(conf, 6);


static fer_cd_geom_t *buildTriMesh(const fer_vec3_t *pts,
                                   const int *ids, size_t ids_len);
static void loadMap(void);

#define CHECK_PARAM(name, type) \
    if (!ferCfgHaveParam(cfg, name)){ \
        fprintf(stderr, "Error: Parameter `%s' is not present.\n", name); \
        ok = 0; \
    }else if (ferCfgParamType(cfg, name) != (type)){ \
        fprintf(stderr, "Error: Invalid type of parameter `%s'.\n", name); \
        ok = 0; \
    }

int ferCfgMapInit(const char *fn)
{
    char name[300];
    char **robots;
    size_t robots_len;
    size_t i, len;
    int ok = 1;
    fer_cd_params_t cd_params;
    const fer_real_t *aabb2;

    ferVecSetZero(6, conf);

    rnd = ferRandMTNewAuto();
    if (!rnd){
        fprintf(stderr, "Can't initialize random generator.\n");
        return -1;
    }

    cfg = ferCfgRead(fn);
    if (!cfg){
        fprintf(stderr, "Can't read cfg file `%s'\n", fn);
        return -1;
    }

    // check params
    CHECK_PARAM("name", FER_CFG_PARAM_STR)
    CHECK_PARAM("dim", FER_CFG_PARAM_INT)
    CHECK_PARAM("aabb", FER_CFG_PARAM_FLT | FER_CFG_PARAM_ARR)
    CHECK_PARAM("pts", FER_CFG_PARAM_V3 | FER_CFG_PARAM_ARR)
    CHECK_PARAM("ids", FER_CFG_PARAM_INT | FER_CFG_PARAM_ARR)
    CHECK_PARAM("robots", FER_CFG_PARAM_STR | FER_CFG_PARAM_ARR)
    if (!ok)
        return -1;

    // check robots
    ferCfgParamStrArr(cfg, "robots", &robots, &robots_len);
    for (i = 0; i < robots_len; i++){
        snprintf(name, 300, "%s_pts", robots[i]);
        CHECK_PARAM(name, FER_CFG_PARAM_V3 | FER_CFG_PARAM_ARR)
        snprintf(name, 300, "%s_ids", robots[i]);
        CHECK_PARAM(name, FER_CFG_PARAM_INT | FER_CFG_PARAM_ARR)
        snprintf(name, 300, "%s_init", robots[i]);
        CHECK_PARAM(name, FER_CFG_PARAM_V3)
        snprintf(name, 300, "%s_goal", robots[i]);
        CHECK_PARAM(name, FER_CFG_PARAM_V3)
        snprintf(name, 300, "%s_h", robots[i]);
        CHECK_PARAM(name, FER_CFG_PARAM_FLT)
    }
    if (!ok)
        return -1;

    ferCDParamsInit(&cd_params);
    cd_params.build_flags = FER_CD_TOP_DOWN
                                | FER_CD_FIT_NAIVE
                                | FER_CD_FIT_NAIVE_NUM_ROT(5);
    cd_params.use_sap = 0;
    cd_params.num_threads = 1;
    cd = ferCDNew(&cd_params);

    loadMap();

    ferCfgParamInt(cfg, "dim", &dim);
    conf_dim = dim;
    ferCfgParamFltArr(cfg, "aabb", &aabb2, &len);
    for (i = 0; i < len; i++){
        aabb[i] = aabb2[i];
    }
    if (dim == 2){
        aabb[4] = -M_PI;
        aabb[5] = M_PI;
    }else{ // dim == 3
        for (i = 0; i < 3; i++){
            aabb[6 + 2 * i] = -M_PI;
            aabb[7 + 2 * i] =  M_PI;
        }
    }
    return 0;
}

void ferCfgMapDestroy(void)
{
    if (cfg)
        ferCfgDel(cfg);
    if (rnd)
        ferRandMTDel(rnd);
    if (cd)
        ferCDDel(cd);
}

int ferCfgMapRobot(const char *name, fer_real_t *h,
                     fer_vec_t *init, fer_vec_t *goal)
{
    int ok = 1;
    char pname[300];
    char format[500];
    const fer_vec3_t *pts;
    const int *ids;
    size_t ids_len;

    snprintf(pname, 300, "%s_pts", name);
    CHECK_PARAM(pname, FER_CFG_PARAM_V3 | FER_CFG_PARAM_ARR)
    snprintf(pname, 300, "%s_ids", name);
    CHECK_PARAM(pname, FER_CFG_PARAM_INT | FER_CFG_PARAM_ARR)
    snprintf(pname, 300, "%s_init", name);
    CHECK_PARAM(pname, FER_CFG_PARAM_V3)
    snprintf(pname, 300, "%s_goal", name);
    CHECK_PARAM(pname, FER_CFG_PARAM_V3)
    snprintf(pname, 300, "%s_h", name);
    CHECK_PARAM(pname, FER_CFG_PARAM_FLT)
    if (!ok){
        fprintf(stderr, "Error: No `%s' robot defined.\n", name);
        return -1;
    }

    snprintf(format, 500, "%s_pts:v3[] %s_ids:i[] %s_ids:i# %s_h:f %s_init:v3 %s_goal:v3",
             name, name, name, name, name, name);
    ferCfgScan(cfg, format, &pts, &ids, &ids_len, h, init, goal);
    robot = buildTriMesh(pts, ids, ids_len);
    robot_name = name;
    ferVecSet(init, 3, FER_ZERO);
    ferVecSet(init, 4, FER_ZERO);
    ferVecSet(init, 5, FER_ZERO);
    ferVecSet(goal, 3, FER_ZERO);
    ferVecSet(goal, 4, FER_ZERO);
    ferVecSet(goal, 5, FER_ZERO);
    return 0;
}

void ferCfgMapListRobots(FILE *out)
{
    char **robots;
    size_t robots_len, i;

    if (ferCfgScan(cfg, "robots:s[] robots:s#", &robots, &robots_len) != 0){
        fprintf(stderr, "# No robots parameter in cfg file.\n");
        return;
    }

    for (i = 0; i < robots_len; i++){
        fprintf(out, "%s\n", robots[i]);
    }
}

int ferCfgMapCollide(const fer_vec_t *conf)
{
    if (!robot)
        return 0;

    if (conf_dim == 2){
        ferCDGeomSetTr3(cd, robot, ferVecGet(conf, 0), ferVecGet(conf, 1), FER_ZERO);
    }else if (conf_dim == 3){
        ferCDGeomSetTr3(cd, robot, ferVecGet(conf, 0), ferVecGet(conf, 1), FER_ZERO);
        ferCDGeomSetRotEuler(cd, robot, FER_ZERO, FER_ZERO, ferVecGet(conf, 2));
    }else if (conf_dim == 6){
        ferCDGeomSetTr(cd, robot, (const fer_vec3_t *)conf);
        ferCDGeomSetRotEuler(cd, robot, ferVecGet(conf, 3), ferVecGet(conf, 4), ferVecGet(conf, 5));
    }
    //ferCDGeomSetRotEuler(cd, robot, FER_ZERO, FER_ZERO, ferVecGet(conf, 2));
    return ferCDGeomCollide(cd, map, robot);
}

const fer_vec_t *ferCfgConf(void)
{
    int i;
    for (i = 0; i < conf_dim; i++){
        ferVecSet(conf, i, ferRandMT(rnd, aabb[2 * i], aabb[2 * i + 1]));
    }

    return conf;
}

const fer_real_t *ferCfgMapAABB(void)
{
    return aabb;
}
int ferCfgDim(void)
{
    return dim;
}

int ferCfgConfDim(void)
{
    return conf_dim;
}

void ferCfgUseRot(void)
{
    if (dim == 2){
        conf_dim = 3;
    }else{
        conf_dim = 6;
    }
}

void ferCfgMapDumpSVT(FILE *out, const char *name)
{
    const fer_vec3_t *pts;
    size_t pts_len;
    const int *ids;
    size_t ids_len;
    size_t i;

    if (ferCfgScan(cfg, "pts:v3[] pts:v3# ids:i[] ids:i#", &pts, &pts_len, &ids, &ids_len) != 0){
        fprintf(stderr, "Can't load map from cfg file.\n");
    }

    fprintf(out, "----\n");
    if (name){
        fprintf(out, "Name: %s\n", name);
    }else{
        fprintf(out, "Name: Map\n");
    }

    fprintf(out, "Points off: 1\n");
    fprintf(out, "Face color: 0.8 0.8 0.8\n");
    fprintf(out, "Points:\n");
    for (i = 0; i < pts_len; i++){
        ferVec3Print(&pts[i], out);
        fprintf(out, "\n");
    }

    fprintf(out, "Faces:\n");
    for (i = 0; (i + 2) < ids_len; i += 3){
        fprintf(out, "%d %d %d\n", (int)ids[i], (int)ids[i + 1], (int)ids[i + 2]);
    }

    fprintf(out, "----\n");
}

void ferCfgMapRobotDumpSVT(const fer_vec_t *conf, FILE *out, const char *name)
{
    char format[500];
    const fer_vec3_t *pts;
    size_t pts_len;
    const int *ids;
    size_t ids_len;
    size_t i;
    fer_vec3_t w;
    fer_mat3_t rot;
    fer_vec3_t tr;

    if (!robot)
        return;

    if (dim == 2){
        ferVec3Set(&tr, ferVecGet(conf, 0), ferVecGet(conf, 1), FER_ZERO);
        ferMat3SetRot3D(&rot, FER_ZERO, FER_ZERO, ferVecGet(conf, 2));
    }else{ //if (dim == 3)
        ferVec3Set(&tr, ferVecGet(conf, 0), ferVecGet(conf, 1), ferVecGet(conf, 2));
        ferMat3SetRot3D(&rot, ferVecGet(conf, 3), ferVecGet(conf, 4), ferVecGet(conf, 5));
    }

    snprintf(format, 500, "%s_pts:v3[] %s_pts:v3# %s_ids:i[] %s_ids:i#",
             robot_name, robot_name, robot_name, robot_name);
    ferCfgScan(cfg, format, &pts, &pts_len, &ids, &ids_len);

    fprintf(out, "----\n");
    if (name){
        fprintf(out, "Name: %s\n", name);
    }else{
        fprintf(out, "Name: Robot\n");
    }

    fprintf(out, "Points off: 1\n");
    fprintf(out, "Face color: 0.1 0.8 0.1\n");
    fprintf(out, "Points:\n");
    for (i = 0; i < pts_len; i++){
        ferMat3MulVec(&w, &rot, &pts[i]);
        ferVec3Add(&w, &tr);
        ferVec3Print(&w, out);
        fprintf(out, "\n");
    }

    /*
    fprintf(out, "Edges:\n");
    for (i = 0; (i + 2) < ids_len; i += 3){
        fprintf(out, "%d %d\n", (int)ids[i], (int)ids[i + 1]);
        fprintf(out, "%d %d\n", (int)ids[i + 1], (int)ids[i + 2]);
        fprintf(out, "%d %d\n", (int)ids[i], (int)ids[i + 2]);
    }
    */
    fprintf(out, "Faces:\n");
    for (i = 0; (i + 2) < ids_len; i += 3){
        fprintf(out, "%d %d %d\n", (int)ids[i], (int)ids[i + 1], (int)ids[i + 2]);
    }

    fprintf(out, "----\n");
}

static fer_cd_geom_t *buildTriMesh(const fer_vec3_t *pts,
                                   const int *ids, size_t ids_len)
{
    fer_cd_geom_t *g;
    size_t i;

    g = ferCDGeomNew(cd);
    for (i = 0; (i + 2) < ids_len; i += 3){
        ferCDGeomAddTri(cd, g, &pts[ids[i]], &pts[ids[i + 1]], &pts[ids[i + 2]]);
    }
    ferCDGeomBuild(cd, g);

    return g;
}

static void loadMap(void)
{
    const fer_vec3_t *pts;
    const int *ids;
    size_t ids_len;

    if (ferCfgScan(cfg, "pts:v3[] ids:i[] ids:i#", &pts, &ids, &ids_len) != 0){
        fprintf(stderr, "Can't load map from cfg file.\n");
    }
    map = buildTriMesh(pts, ids, ids_len);
}
