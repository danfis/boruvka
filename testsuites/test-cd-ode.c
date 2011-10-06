/*************************************************************************
100*                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#include <unistd.h>
#include <ode/ode.h>
#include <ode/odemath.h>
#include <drawstuff/drawstuff.h>
#include <fermat/cd.h>
#include <fermat/list.h>
#include <fermat/timer.h>
#include <fermat/opts.h>
#include <fermat/dbg.h>
#include <fermat/alloc.h>
#include "data.h"

#define DRAWSTUFF_TEXTURE_PATH "textures"
#ifdef _MSC_VER
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif


// select correct drawing functions

#ifdef dDOUBLE
#define dsDrawBox dsDrawBoxD
#define dsDrawSphere dsDrawSphereD
#define dsDrawCylinder dsDrawCylinderD
#define dsDrawCapsule dsDrawCapsuleD
#define dsDrawConvex dsDrawConvexD
#endif


// some constants

#define NUM 1000000			// max number of objects
#define DENSITY (5.0)		// density of all objects
#define GPB 3			// maximum number of geometries per body
#define MAX_CONTACTS 8          // maximum number of contact points per body
#define MAX_FEEDBACKNUM 20
#define GRAVITY         REAL(0.5)
#define USE_GEOM_OFFSET 1

struct _obj_t {
    dBodyID body;
    dGeomID geom;
    fer_cd_geom_t *g;
    fer_list_t list;
};
typedef struct _obj_t obj_t;

static fer_cd_t *cd = NULL;
static FER_LIST(objs);
static obj_t plane;
static int pargc;
static char **pargv;
static fer_cd_params_t params;
static fer_timer_t loop_timer;

static dWorldID world;
static dSpaceID space;
static dJointGroupID contactgroup;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static int no_win = 0;
static int no_sleep = 0;
static int show_contacts = 0;
static int show_aabb = 0;
static int use_ode = 0;
static int grid_level = 1;

static char *cmds = NULL;


static void bodyMoved(dBodyID body)
{
    obj_t *obj = (obj_t *)dBodyGetData(body);
    const dReal *pos, *q;
    fer_mat3_t rot;
    fer_quat_t quat;

    pos = dBodyGetPosition(body);
    q = dBodyGetQuaternion(body);

    ferQuatSet(&quat, q[1], q[2], q[3], q[0]);
    ferQuatToMat3(&quat, &rot);

    if (obj->g){
        ferCDGeomSetTr3(cd, obj->g, pos[0], pos[1], pos[2]);
        ferCDGeomSetRot(cd, obj->g, &rot);
    }
}


static obj_t *objNew(dBodyID body, dGeomID geom, fer_cd_geom_t *g)
{
    obj_t *obj;

    obj = FER_ALLOC(obj_t);
    obj->body = body;
    obj->geom = geom;
    obj->g    = g;

    if (geom)
        dGeomSetBody(geom, body);

    dBodySetData(body, (void *)obj);
    dBodySetMovedCallback(body, bodyMoved);
    ferCDGeomSetData(g, (void *)obj);


    bodyMoved(body);

    ferListAppend(&objs, &obj->list);

    return obj;
}


void drawGeom (dGeomID g, const dReal *pos, const dReal *R, int show_aabb)
{
    int i;

    if (!g)
        return;
    if (!pos)
        pos = dGeomGetPosition (g);
    if (!R)
        R = dGeomGetRotation (g);

    int type = dGeomGetClass (g);

    if (type == dBoxClass) {
        dVector3 sides;
        dGeomBoxGetLengths (g,sides);
        dsDrawBox (pos,R,sides);

    }else if (type == dSphereClass) {
        dsDrawSphere (pos,R,dGeomSphereGetRadius (g));

    }else if (type == dCapsuleClass) {
        dReal radius,length;
        dGeomCapsuleGetParams (g,&radius,&length);
        dsDrawCapsule (pos,R,length,radius);

    }else if (type == dCylinderClass) {
        dReal radius,length;
        dGeomCylinderGetParams (g,&radius,&length);
        dsDrawCylinder (pos,R,length,radius);
    }

    if (show_aabb) {
        // draw the bounding box for this geom
        dReal aabb[6];
        dGeomGetAABB (g,aabb);
        dVector3 bbpos;

        for (i=0; i<3; i++)
            bbpos[i] = 0.5*(aabb[i*2] + aabb[i*2+1]);

        dVector3 bbsides;
        for (i=0; i<3; i++)
            bbsides[i] = aabb[i*2+1] - aabb[i*2];

        dMatrix3 RI;
        dRSetIdentity (RI);
        dsSetColorAlpha (1,0,0,0.5);
        dsDrawBox (bbpos,RI,bbsides);
    }
}

static void _drawG(fer_cd_obb_t *obb, const dReal *pos, const dReal *rot)
{
    fer_list_t *item;
    fer_cd_obb_t *o;
    dReal v[9];
    fer_cd_tri_t *t;

    if (ferListEmpty(&obb->obbs)){
        if (obb->shape->cl->type == FER_CD_SHAPE_TRI
                || obb->shape->cl->type == FER_CD_SHAPE_TRIMESH_TRI){
            t = (fer_cd_tri_t *)obb->shape;
            v[0] = ferVec3X(t->p[0]);
            v[1] = ferVec3Y(t->p[0]);
            v[2] = ferVec3Z(t->p[0]);
            v[3] = ferVec3X(t->p[1]);
            v[4] = ferVec3Y(t->p[1]);
            v[5] = ferVec3Z(t->p[1]);
            v[6] = ferVec3X(t->p[2]);
            v[7] = ferVec3Y(t->p[2]);
            v[8] = ferVec3Z(t->p[2]);

            dsDrawTriangle(pos, rot, v, v + 3, v + 6, 0);
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            _drawG(o, pos, rot);
        }
    }
}

static void drawTriMesh(fer_cd_geom_t *g, const dReal *pos, const dReal *rot)
{
    fer_list_t *item;
    fer_cd_obb_t *obb;

    FER_LIST_FOR_EACH(&g->obbs, item){
        obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        _drawG(obb, pos, rot);
    }
}


static int sepCB(const fer_cd_t *cd,
                 const fer_cd_geom_t *g1, const fer_cd_geom_t *g2,
                 const fer_cd_contacts_t *con,
                 void *data)
{
    obj_t *obj1, *obj2;
    size_t i;
    dContact contact;
    dBodyID b1, b2;
   
    obj1 = (obj_t *)ferCDGeomData((fer_cd_geom_t *)g1);
    obj2 = (obj_t *)ferCDGeomData((fer_cd_geom_t *)g2);
    b1 = (obj1 ? obj1->body : 0);
    b2 = (obj2 ? obj2->body : 0);

    for (i = 0; i < con->num; i++){
        /*
        fprintf(stdout, "# [%02d] %lx-%lx dir: <%f %f %f>, pos: <%f %f %f>, depth: %f\n",
                (int)i, (long)b1, (long)b2,
                ferVec3X(&con->dir[i]), ferVec3Y(&con->dir[i]), ferVec3Z(&con->dir[i]),
                ferVec3X(&con->pos[i]), ferVec3Y(&con->pos[i]), ferVec3Z(&con->pos[i]),
                con->depth[i]);
        */

        if (con->depth[i] < FER_ZERO)
            continue;

        contact.geom.pos[0] = ferVec3X(&con->pos[i]);
        contact.geom.pos[1] = ferVec3Y(&con->pos[i]);
        contact.geom.pos[2] = ferVec3Z(&con->pos[i]);
        contact.geom.normal[0] = -ferVec3X(&con->dir[i]);
        contact.geom.normal[1] = -ferVec3Y(&con->dir[i]);
        contact.geom.normal[2] = -ferVec3Z(&con->dir[i]);
        contact.geom.depth = con->depth[i];
        contact.geom.g1 = 0;
        contact.geom.g2 = 0;
        contact.surface.mode = dContactBounce | dContactSoftCFM;
        contact.surface.mu = dInfinity;
        contact.surface.mu2 = 0;
        contact.surface.bounce = 0.1;
        contact.surface.bounce_vel = 0.1;
        contact.surface.soft_cfm = 0.01;

        pthread_mutex_lock(&lock);
        dJointID c = dJointCreateContact(world, contactgroup, &contact);
        dJointAttach(c, b1, b2);
        pthread_mutex_unlock(&lock);

        if (!no_win && show_contacts){
            dMatrix3 RI;
            dRSetIdentity (RI);
            const dReal ss[3] = {0.02,0.02,0.02};

            dsDrawBox(contact.geom.pos, RI, ss);
        }
    }

    return 0;
}

static void nearCallback (void *data, dGeomID o1, dGeomID o2)
{
  int i;
  // if (o1->body && o2->body) return;

  // exit without doing anything if the two bodies are connected by a joint
  dBodyID b1 = dGeomGetBody(o1);
  dBodyID b2 = dGeomGetBody(o2);
  if (b1 && b2 && dAreConnectedExcluding (b1,b2,dJointTypeContact)) return;

  dContact contact[MAX_CONTACTS];   // up to MAX_CONTACTS contacts per box-box
  for (i=0; i<MAX_CONTACTS; i++) {
    contact[i].surface.mode = dContactBounce | dContactSoftCFM;
    contact[i].surface.mu = dInfinity;
    contact[i].surface.mu2 = 0;
    contact[i].surface.bounce = 0.1;
    contact[i].surface.bounce_vel = 0.1;
    contact[i].surface.soft_cfm = 0.01;
  }
  int numc = dCollide (o1,o2,MAX_CONTACTS,&contact[0].geom,
			   sizeof(dContact));
  if (numc){
    dMatrix3 RI;
    dRSetIdentity (RI);
    const dReal ss[3] = {0.02,0.02,0.02};
    for (i=0; i<numc; i++) {
      dJointID c = dJointCreateContact (world,contactgroup,contact+i);
      dJointAttach (c,b1,b2);
      if (show_contacts)
          dsDrawBox (contact[i].geom.pos,RI,ss);

    }
  }
}



static void start(void)
{
    dAllocateODEDataForThread(dAllocateMaskAll);

    //static float xyz[3] = {3.1640f,-0.3079f,1.7600f};
    //static float hpr[3] = {125.5000f,-17.0000f,0.0000f};
    //static float xyz[3] = {1.2192,0.8433,0.4700};
    //static float hpr[3] = {125.5000,-17.0000,0.0000};
    static float xyz[3] = { 5.518125, -3.677142, 2.340000 };
    static float hpr[3] = { 125.500000, -17.000000, 0.000000 };
    dsSetViewpoint (xyz,hpr);
    printf ("To drop another object, press:\n");
    printf ("   b for box.\n");
    printf ("   s for sphere.\n");
    printf ("   c for capsule.\n");
    printf ("   y for cylinder.\n");
    printf ("   m for bunny.\n");
    printf ("To print xyz, hpr viewpoint, press v.\n");
    printf ("To toggle showing the geom AABBs, press a.\n");
    printf ("To toggle showing the contact points, press t.\n");
}

static void command (int cmd);

static void simLoop(int pause)
{
    fer_list_t *item;
    obj_t *obj;

    //static int c = 1;
    //fprintf(stderr, "c: %d\n", c++);

    if (cmds && *cmds != 0x0){
        command(*cmds);
        ++cmds;
    }

    if (!no_sleep)
        usleep(10000);

    if (!no_win)
        dsSetColor (0,0,2);

    // remove all contact joints
    dJointGroupEmpty(contactgroup);

    ferTimerStart(&loop_timer);
    if (use_ode){
        dSpaceCollide(space, 0, &nearCallback);
    }else{
        ferCDSeparate(cd, sepCB, NULL);
    }
    ferTimerStop(&loop_timer);
    fprintf(stderr, "%lu us\n", ferTimerElapsedInUs(&loop_timer));

    if (!pause)
        dWorldQuickStep (world,0.02);


    if (!no_win){
        dsSetColor (1,1,0);
        dsSetTexture (DS_WOOD);

        FER_LIST_FOR_EACH(&objs, item){
            obj = FER_LIST_ENTRY(item, obj_t, list);
            if (obj->geom){
                drawGeom(obj->geom, 0, 0, show_aabb);
            }else if (obj->g){
                drawTriMesh(obj->g, dBodyGetPosition(obj->body),
                                    dBodyGetRotation(obj->body));
            }
        }
    }
}

static void command (int cmd)
{
    dBodyID body;
    dGeomID geom;
    fer_cd_geom_t *g;
    dReal sides[3];
    dMass m;
    dMatrix3 R;
    size_t i;
    static float xyz[3];
    static float hpr[3];

    if (cmd == 'b' || cmd == 's' || cmd == 'y' || cmd == 'c' || cmd == 'm'){
        body = dBodyCreate(world);
        dBodySetPosition(body, dRandReal()*2-1, dRandReal()*2-1, dRandReal()+2);
        dRFromAxisAndAngle(R, dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
                dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
        dBodySetRotation(body, R);

        for (i = 0; i < 3; i++)
            sides[i] = dRandReal() * 0.5 + 0.1;

        g = ferCDGeomNew(cd);
        geom = 0;

        if (cmd == 'b'){
            dMassSetBox(&m, DENSITY, sides[0], sides[1], sides[2]);
            geom = dCreateBox(space, sides[0], sides[1], sides[2]);
            ferCDGeomAddBox(cd, g, sides[0], sides[1], sides[2]);
        }else if (cmd == 's'){
            dMassSetSphere(&m, DENSITY, sides[0]);
            geom = dCreateSphere(space, sides[0]);
            ferCDGeomAddSphere(cd, g, sides[0]);
        }else if (cmd == 'y'){
            dMassSetCylinder (&m,DENSITY,3,sides[0],sides[1]);
            geom = dCreateCylinder (space,sides[0],sides[1]);
            g = ferCDGeomNew(cd);
            ferCDGeomAddCyl(cd, g, sides[0], sides[1]);
        }else if (cmd == 'c'){
            sides[0] *= 0.5;
            dMassSetCapsule(&m,DENSITY,3,sides[0],sides[1]);
            geom = dCreateCapsule(space,sides[0],sides[1]);
            g = ferCDGeomNew(cd);
            ferCDGeomAddCap(cd, g, sides[0], sides[1]);
        }else if (cmd == 'm'){
            dMassSetCylinder(&m,DENSITY,3,sides[0],sides[1]);
            geom = 0;
            //obj[i].geom[1] = dCreateCylinder (space,sides[0],sides[1]);
            g = ferCDGeomNew(cd);
            ferCDGeomAddTriMesh(cd, g, bunny_coords, bunny_ids, bunny_tri_len);
        }

        dBodySetMass(body, &m);
        objNew(body, geom, g);

    }else if (cmd == 'q'){
        exit(-1);

    }else if (cmd == 'a') {
        show_aabb ^= 1;
    }else if (cmd == 't') {
        show_contacts ^= 1;
    }else if (cmd == 'v'){
        dsGetViewpoint(xyz, hpr);
        printf("xyz: %f %f %f\nhpr: %f %f %f\n",
               xyz[0], xyz[1], xyz[2],
               hpr[0], hpr[1], hpr[2]);
        fflush(stdout);
    }
}

static void boxGrid(int x, int y)
{
    size_t i;
    dReal sides[3];
    dMatrix3 R;
    dMass m;
    dBodyID body;
    dGeomID geom;
    fer_cd_geom_t *g;

    for (i = 0; i < x * y; i++){
        body = dBodyCreate(world);
        sides[0] = 0.1;
        sides[1] = 0.2;
        sides[2] = 0.3;

        dBodySetPosition(body, 0.4 * (i / x),
                                      0.4 * (i % x), 0.5 + grid_level * 0.5);
        dRFromAxisAndAngle(R, dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
                              dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
        dBodySetRotation(body,R);
        dMassSetBox(&m,DENSITY,sides[0],sides[1],sides[2]);
        dBodySetMass(body, &m);

        geom = dCreateBox(space,sides[0],sides[1],sides[2]);
        g = ferCDGeomNew(cd);
        ferCDGeomAddBox(cd, g, sides[0], sides[1], sides[2]);


        objNew(body, geom, g);
    }
}

static void sphereGrid(int x, int y)
{
    size_t i;
    dMass m;
    dBodyID body;
    dGeomID geom;
    fer_cd_geom_t *g;

    for (i = 0; i < x * y; i++){
        body = dBodyCreate(world);

        dBodySetPosition(body, 0.4 * (i / x),
                               0.4 * (i % x), 0.5 + grid_level * 0.5);

        geom = dCreateSphere(space, 0.1);
        g = ferCDGeomNew(cd);
        ferCDGeomAddSphere(cd, g, 0.1);

        dMassSetSphere(&m,DENSITY, 0.1);
        dBodySetMass(body, &m);

        objNew(body, geom, g);
    }
}

static void capGrid(int x, int y)
{
    size_t i;
    dReal sides[3];
    dMatrix3 R;
    dMass m;
    dBodyID body;
    dGeomID geom;
    fer_cd_geom_t *g;

    for (i = 0; i < x * y; i++){
        body = dBodyCreate(world);
        sides[0] = 0.1;
        sides[1] = 0.2;
        sides[2] = 0.3;

        dBodySetPosition(body, 0.4 * (i / x),
                               0.4 * (i % x), 0.5 + grid_level * 0.5);
        dRFromAxisAndAngle(R, dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
                              dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
        dBodySetRotation(body,R);
        dMassSetCapsule(&m,DENSITY,3, sides[0],sides[1]);
        dBodySetMass(body, &m);

        geom = dCreateCapsule(space,sides[0],sides[1]);
        g = ferCDGeomNew(cd);
        ferCDGeomAddCap(cd, g, sides[0], sides[1]);


        objNew(body, geom, g);
    }
}

static void cylGrid(int x, int y)
{
    size_t i;
    dReal sides[3];
    dMatrix3 R;
    dMass m;
    dBodyID body;
    dGeomID geom;
    fer_cd_geom_t *g;

    for (i = 0; i < x * y; i++){
        body = dBodyCreate(world);
        sides[0] = 0.1;
        sides[1] = 0.2;
        sides[2] = 0.3;

        dBodySetPosition(body, 0.4 * (i / x),
                               0.4 * (i % x), 0.5 + grid_level * 0.5);
        dRFromAxisAndAngle(R, dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
                              dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
        dBodySetRotation(body,R);
        dMassSetCylinder(&m,DENSITY,3, sides[0],sides[1]);
        dBodySetMass(body, &m);

        geom = dCreateCylinder(space,sides[0],sides[1]);
        g = ferCDGeomNew(cd);
        ferCDGeomAddCyl(cd, g, sides[0], sides[1]);


        objNew(body, geom, g);
    }
}


static void optGrid(const char *l, char s, const fer_vec2_t *v)
{
    if (!cd)
        cd = ferCDNew(&params);

    if (strcmp(l, "box-grid") == 0){
        boxGrid(ferVec2X(v), ferVec2Y(v));
    }else if (strcmp(l, "sphere-grid") == 0){
        sphereGrid(ferVec2X(v), ferVec2Y(v));
    }else if (strcmp(l, "cap-grid") == 0){
        capGrid(ferVec2X(v), ferVec2Y(v));
    }else if (strcmp(l, "cyl-grid") == 0){
        cylGrid(ferVec2X(v), ferVec2Y(v));
    }

    grid_level++;
}

static void opt(void)
{
    ferCDParamsInit(&params);

    params.use_sap = 1;
    params.sap_hashsize = 1023 * 1023 * 10 + 1;

    ferOptsAdd("use-gpu", 0, FER_OPTS_NONE, &params.sap_gpu, NULL);
    ferOptsAdd("threads", 't', FER_OPTS_SIZE_T, &params.num_threads, NULL);
    ferOptsAdd("max-contacts", 0, FER_OPTS_SIZE_T, &params.max_contacts, NULL);
    ferOptsAdd("no-win", 0, FER_OPTS_NONE, &no_win, NULL);
    ferOptsAdd("no-sleep", 0, FER_OPTS_NONE, &no_sleep, NULL);
    ferOptsAdd("use-ode", 0, FER_OPTS_NONE, &use_ode, NULL);
    ferOptsAdd("cmds", 0, FER_OPTS_STR, &cmds, NULL);
    ferOptsAdd("box-grid", 0, FER_OPTS_V2, NULL, FER_OPTS_CB(optGrid));
    ferOptsAdd("sphere-grid", 0, FER_OPTS_V2, NULL, FER_OPTS_CB(optGrid));
    ferOptsAdd("cap-grid", 0, FER_OPTS_V2, NULL, FER_OPTS_CB(optGrid));
    ferOptsAdd("cyl-grid", 0, FER_OPTS_V2, NULL, FER_OPTS_CB(optGrid));

    ferOpts(&pargc, pargv);
}

int main (int argc, char **argv)
{

    // setup pointers to drawstuff callback functions
    dsFunctions fn;
    fn.version = DS_VERSION;
    fn.start = &start;
    fn.step = &simLoop;
    fn.command = &command;
    fn.stop = 0;
    fn.path_to_textures = DRAWSTUFF_TEXTURE_PATH;

    // create world
    dInitODE2(0);
    world = dWorldCreate();
    space = dHashSpaceCreate(0);
    contactgroup = dJointGroupCreate(0);
    dWorldSetGravity(world, 0, 0, -GRAVITY);
    dWorldSetCFM(world, 1e-5);
    dWorldSetAutoDisableFlag(world, 1);
    dWorldSetAutoDisableAverageSamplesCount(world, 10);

    dWorldSetLinearDamping(world, 0.00001);
    dWorldSetAngularDamping(world, 0.005);
    dWorldSetMaxAngularSpeed(world, 200);

    dWorldSetContactMaxCorrectingVel(world, 0.1);
    dWorldSetContactSurfaceLayer(world, 0.001);

    pargc = argc;
    pargv = argv;
    opt();

    // init collide library
    if (!cd)
        cd = ferCDNew(&params);


    plane.g = ferCDGeomNew(cd);
    ferCDGeomAddPlane(cd, plane.g);
    ferCDGeomSetData(plane.g, (void *)&plane);
    plane.geom = dCreatePlane(space, 0, 0, 1, 0);
    plane.body = NULL;


    // run simulation
    if (no_win){
        while (1){
            simLoop(0);
        }
    }else{
        dsSimulationLoop(argc, argv, 800, 600, &fn);
    }

    dJointGroupDestroy(contactgroup);
    dSpaceDestroy(space);
    dWorldDestroy(world);
    dCloseODE();

    ferCDDel(cd);

    return 0;
}
