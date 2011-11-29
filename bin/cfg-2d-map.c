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


static fer_cfg_t *cfg = NULL;

static fer_cd_t *cd = NULL;
static fer_cd_geom_t *map = NULL;
static fer_cd_geom_t *robot = NULL;
static const char *robot_name = NULL;


static fer_cd_geom_t *buildTriMesh(const fer_vec2_t *pts,
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

int ferCfg2DMapInit(const char *fn)
{
    char name[300];
    char **robots;
    size_t robots_len;
    size_t i;
    int ok = 1;
    fer_cd_params_t cd_params;

    cfg = ferCfgRead(fn);
    if (!cfg){
        fprintf(stderr, "Can't read cfg file `%s'\n", fn);
        return -1;
    }

    // check params
    CHECK_PARAM("name", FER_CFG_PARAM_STR)
    CHECK_PARAM("aabb", FER_CFG_PARAM_FLT | FER_CFG_PARAM_ARR)
    CHECK_PARAM("pts", FER_CFG_PARAM_V2 | FER_CFG_PARAM_ARR)
    CHECK_PARAM("ids", FER_CFG_PARAM_INT | FER_CFG_PARAM_ARR)
    CHECK_PARAM("robots", FER_CFG_PARAM_STR | FER_CFG_PARAM_ARR)
    if (!ok)
        return -1;

    // check robots
    ferCfgParamStrArr(cfg, "robots", &robots, &robots_len);
    for (i = 0; i < robots_len; i++){
        snprintf(name, 300, "%s_pts", robots[i]);
        CHECK_PARAM(name, FER_CFG_PARAM_V2 | FER_CFG_PARAM_ARR)
        snprintf(name, 300, "%s_ids", robots[i]);
        CHECK_PARAM(name, FER_CFG_PARAM_INT | FER_CFG_PARAM_ARR)
        snprintf(name, 300, "%s_init", robots[i]);
        CHECK_PARAM(name, FER_CFG_PARAM_V2)
        snprintf(name, 300, "%s_goal", robots[i]);
        CHECK_PARAM(name, FER_CFG_PARAM_V2)
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
    return 0;
}

void ferCfg2DMapDestroy(void)
{
    if (cfg)
        ferCfgDel(cfg);
    if (cd)
        ferCDDel(cd);
}

int ferCfg2DMapRobot(const char *name, fer_real_t *h,
                     fer_vec_t *init, fer_vec_t *goal)
{
    int ok = 1;
    char pname[300];
    char format[500];
    const fer_vec2_t *pts;
    const int *ids;
    size_t ids_len;

    snprintf(pname, 300, "%s_pts", name);
    CHECK_PARAM(pname, FER_CFG_PARAM_V2 | FER_CFG_PARAM_ARR)
    snprintf(pname, 300, "%s_ids", name);
    CHECK_PARAM(pname, FER_CFG_PARAM_INT | FER_CFG_PARAM_ARR)
    snprintf(pname, 300, "%s_init", name);
    CHECK_PARAM(pname, FER_CFG_PARAM_V2)
    snprintf(pname, 300, "%s_goal", name);
    CHECK_PARAM(pname, FER_CFG_PARAM_V2)
    snprintf(pname, 300, "%s_h", name);
    CHECK_PARAM(pname, FER_CFG_PARAM_FLT)
    if (!ok){
        fprintf(stderr, "Error: No `%s' robot defined.\n", name);
        return -1;
    }

    snprintf(format, 500, "%s_pts:v2[] %s_ids:i[] %s_ids:i# %s_h:f %s_init:v2 %s_goal:v2",
             name, name, name, name, name, name);
    ferCfgScan(cfg, format, &pts, &ids, &ids_len, h, init, goal);
    robot = buildTriMesh(pts, ids, ids_len);
    robot_name = name;
    return 0;
}

void ferCfg2DMapListRobots(FILE *out)
{
    char **robots;
    size_t robots_len, i;

    if (ferCfgScan(cfg, "robots:s[] robots:s#", &robots, &robots_len) != 0){
        fprintf(stderr, "No robots parameter in cfg file.\n");
        return;
    }

    fprintf(out, "Robots:\n");
    for (i = 0; i < robots_len; i++){
        fprintf(out, "    %s\n", robots[i]);
    }
}

int ferCfg2DMapCollide(const fer_vec_t *conf)
{
    if (!robot)
        return 0;

    ferCDGeomSetTr3(cd, robot, ferVecGet(conf, 0), ferVecGet(conf, 1), FER_ZERO);
    return ferCDGeomCollide(cd, map, robot);
}

const fer_real_t *ferCfg2DMapAABB(void)
{
    const fer_real_t *aabb;
    size_t len;
    ferCfgParamFltArr(cfg, "aabb", &aabb, &len);
    return aabb;
}

void ferCfg2DMapDumpSVT(FILE *out, const char *name)
{
    const fer_vec2_t *pts;
    size_t pts_len;
    const int *ids;
    size_t ids_len;
    size_t i;

    if (ferCfgScan(cfg, "pts:v2[] pts:v2# ids:i[] ids:i#", &pts, &pts_len, &ids, &ids_len) != 0){
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
    fprintf(out, "Points2d:\n");
    for (i = 0; i < pts_len; i++){
        ferVec2Print(&pts[i], out);
        fprintf(out, "\n");
    }

    fprintf(out, "Faces:\n");
    for (i = 0; (i + 2) < ids_len; i += 3){
        fprintf(out, "%d %d %d\n", (int)ids[i], (int)ids[i + 1], (int)ids[i + 2]);
    }

    fprintf(out, "----\n");
}

void ferCfg2DMapRobotDumpSVT(const fer_vec_t *conf, FILE *out, const char *name)
{
    char format[500];
    const fer_vec2_t *pts;
    size_t pts_len;
    const int *ids;
    size_t ids_len;
    size_t i;
    fer_vec2_t w;

    if (!robot)
        return;

    snprintf(format, 500, "%s_pts:v2[] %s_pts:v2# %s_ids:i[] %s_ids:i#",
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
    fprintf(out, "Points2d:\n");
    for (i = 0; i < pts_len; i++){
        ferVec2Add2(&w, &pts[i], (const fer_vec2_t *)conf);
        ferVec2Print(&w, out);
        fprintf(out, "\n");
    }

    fprintf(out, "Faces:\n");
    for (i = 0; (i + 2) < ids_len; i += 3){
        fprintf(out, "%d %d %d\n", (int)ids[i], (int)ids[i + 1], (int)ids[i + 2]);
    }

    fprintf(out, "----\n");
}

static fer_cd_geom_t *buildTriMesh(const fer_vec2_t *pts,
                                   const int *ids, size_t ids_len)
{
    fer_vec3_t p[3];
    fer_cd_geom_t *g;
    size_t i;

    g = ferCDGeomNew(cd);
    for (i = 0; (i + 2) < ids_len; i += 3){
        ferVec3Set(&p[0], ferVec2X(&pts[ids[i]]), ferVec2Y(&pts[ids[i]]), FER_ZERO);
        ferVec3Set(&p[1], ferVec2X(&pts[ids[i+1]]), ferVec2Y(&pts[ids[i+1]]), FER_ZERO);
        ferVec3Set(&p[2], ferVec2X(&pts[ids[i+2]]), ferVec2Y(&pts[ids[i+2]]), FER_ZERO);
        ferCDGeomAddTri(cd, g, &p[0], &p[1], &p[2]);
    }
    ferCDGeomBuild(cd, g);

    return g;
}

static void loadMap(void)
{
    const fer_vec2_t *pts;
    const int *ids;
    size_t ids_len;

    if (ferCfgScan(cfg, "pts:v2[] ids:i[] ids:i#", &pts, &ids, &ids_len) != 0){
        fprintf(stderr, "Can't load map from cfg file.\n");
    }
    map = buildTriMesh(pts, ids, ids_len);
}
