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
#include <limits.h>
#include <ode/ode.h>
#include <ode/odemath.h>
#include <drawstuff/drawstuff.h>
#include <fermat/cd.h>
#include <fermat/cd-ode.h>
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
static int show_obb = 0;
static int use_ode = 0;
static int grid_level = 1;
static int paus = 0;
static int alpha = 1;
static long num_steps = LONG_MAX;
static int no_plane = 0;

static char *cmds = NULL;

#ifdef FER_CD_TIME_MEASURE
FILE *meas = NULL;
#endif /* FER_CD_TIME_MEASURE */


static obj_t *objNew(dBodyID body, dGeomID geom, fer_cd_geom_t *g)
{
    obj_t *obj;

    obj = FER_ALLOC(obj_t);
    obj->body = body;
    obj->geom = geom;
    obj->g    = g;

    if (geom && body){
        dGeomSetBody(geom, body);
    }

    if (g){
        ferCDGeomSetData(g, (void *)obj);
        if (body)
            ferCDGeomSetBody(cd, g, body);
    }

    ferListAppend(&objs, &obj->list);

    return obj;
}

static void objsDel(void)
{
    fer_list_t *item;
    obj_t *obj;

    while (!ferListEmpty(&objs)){
        item = ferListNext(&objs);
        ferListDel(item);
        obj = FER_LIST_ENTRY(item, obj_t, list);
        free(obj);
    }
}

static void _drawShape(fer_cd_geom_t *g, fer_cd_shape_t *shape,
                       const fer_mat3_t *rot2, const fer_vec3_t *pos2)
{
    dVector3 sides;
    dReal pos[4];
    dReal rot[12];
    fer_vec3_t tr;
    fer_mat3_t r;
    dReal v[9];
    fer_cd_tri_t *t;
    fer_cd_box_t *box;
    fer_cd_sphere_t *sphere;
    fer_cd_cap_t *cap;
    fer_cd_cyl_t *cyl;
    fer_cd_shape_off_t *off;

    dsSetColorAlpha (1,1,0,0.7 + alpha * 0.3);
    dsSetTexture (DS_WOOD);

    ferMat3Mul2(&r, ferCDGeomRot(cd, g), rot2);
    ferMat3MulVec(&tr, ferCDGeomRot(cd, g), pos2);
    ferVec3Add(&tr, ferCDGeomTr(cd, g));
    //tr = ferCDGeomTr(cd, g);
    //r  = ferCDGeomRot(cd, g);
    pos[0] = ferVec3X(&tr);
    pos[1] = ferVec3Y(&tr);
    pos[2] = ferVec3Z(&tr);
    rot[0] = ferMat3Get(&r, 0, 0);
    rot[1] = ferMat3Get(&r, 0, 1);
    rot[2] = ferMat3Get(&r, 0, 2);
    rot[4] = ferMat3Get(&r, 1, 0);
    rot[5] = ferMat3Get(&r, 1, 1);
    rot[6] = ferMat3Get(&r, 1, 2);
    rot[8] = ferMat3Get(&r, 2, 0);
    rot[9] = ferMat3Get(&r, 2, 1);
    rot[10] = ferMat3Get(&r, 2, 2);

    if (shape->cl->type == FER_CD_SHAPE_TRI
            || shape->cl->type == FER_CD_SHAPE_TRIMESH_TRI){
        t = (fer_cd_tri_t *)shape;
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

    }else if (shape->cl->type == FER_CD_SHAPE_BOX){
        box = (fer_cd_box_t *)shape;
        sides[0] = 2. * ferVec3X(box->half_extents);
        sides[1] = 2. * ferVec3Y(box->half_extents);
        sides[2] = 2. * ferVec3Z(box->half_extents);
        dsDrawBox(pos, rot, sides);

    }else if (shape->cl->type == FER_CD_SHAPE_SPHERE){
        sphere = (fer_cd_sphere_t *)shape;
        dsDrawSphere(pos, rot, sphere->radius);

    }else if (shape->cl->type == FER_CD_SHAPE_CAP){
        cap = (fer_cd_cap_t *)shape;
        dsDrawCapsule(pos, rot, 2 * cap->half_height, cap->radius);

    }else if (shape->cl->type == FER_CD_SHAPE_CYL){
        cyl = (fer_cd_cyl_t *)shape;
        dsDrawCylinder(pos, rot, 2 * cyl->half_height, cyl->radius);

    }else if (shape->cl->type == FER_CD_SHAPE_OFF){
        off = (fer_cd_shape_off_t *)shape;
        _drawShape(g, off->shape, off->rot, off->tr);
    }
}

static void _drawG(fer_cd_geom_t *g, fer_cd_obb_t *obb)
{
    dReal pos[4];
    dReal rot[12];
    dVector3 sides;
    fer_list_t *item;
    fer_cd_obb_t *o;
    const fer_vec3_t *tr;
    const fer_mat3_t *r;

    tr = ferCDGeomTr(cd, g);
    r  = ferCDGeomRot(cd, g);
    pos[0] = ferVec3X(tr);
    pos[1] = ferVec3Y(tr);
    pos[2] = ferVec3Z(tr);
    rot[0] = ferMat3Get(r, 0, 0);
    rot[1] = ferMat3Get(r, 0, 1);
    rot[2] = ferMat3Get(r, 0, 2);
    rot[4] = ferMat3Get(r, 1, 0);
    rot[5] = ferMat3Get(r, 1, 1);
    rot[6] = ferMat3Get(r, 1, 2);
    rot[8] = ferMat3Get(r, 2, 0);
    rot[9] = ferMat3Get(r, 2, 1);
    rot[10] = ferMat3Get(r, 2, 2);

    if (ferListEmpty(&obb->obbs)){
        _drawShape(g, obb->shape, fer_mat3_identity, fer_vec3_origin);

        if (show_obb){
            dReal x, y, z;
            dReal pos2[4];
            dReal rot2[12];

            sides[0] = 2. * ferVec3X(&obb->half_extents);
            sides[1] = 2. * ferVec3Y(&obb->half_extents);
            sides[2] = 2. * ferVec3Z(&obb->half_extents);
            x = ferVec3X(&obb->center);
            y = ferVec3Y(&obb->center);
            z = ferVec3Z(&obb->center);
            pos2[0] = x * rot[0] + y * rot[1] + z * rot[2] + pos[0];
            pos2[1] = x * rot[4] + y * rot[5] + z * rot[6] + pos[1];
            pos2[2] = x * rot[8] + y * rot[9] + z * rot[10] + pos[2];
            rot2[0] = rot[0] * ferVec3X(&obb->axis[0])
                        + rot[1] * ferVec3Y(&obb->axis[0])
                        + rot[2] * ferVec3Z(&obb->axis[0]);
            rot2[1] = rot[0] * ferVec3X(&obb->axis[1])
                        + rot[1] * ferVec3Y(&obb->axis[1])
                        + rot[2] * ferVec3Z(&obb->axis[1]);
            rot2[2] = rot[0] * ferVec3X(&obb->axis[2])
                        + rot[1] * ferVec3Y(&obb->axis[2])
                        + rot[2] * ferVec3Z(&obb->axis[2]);
            rot2[4] = rot[4] * ferVec3X(&obb->axis[0])
                        + rot[5] * ferVec3Y(&obb->axis[0])
                        + rot[6] * ferVec3Z(&obb->axis[0]);
            rot2[5] = rot[4] * ferVec3X(&obb->axis[1])
                        + rot[5] * ferVec3Y(&obb->axis[1])
                        + rot[6] * ferVec3Z(&obb->axis[1]);
            rot2[6] = rot[4] * ferVec3X(&obb->axis[2])
                        + rot[5] * ferVec3Y(&obb->axis[2])
                        + rot[6] * ferVec3Z(&obb->axis[2]);
            rot2[8] = rot[8] * ferVec3X(&obb->axis[0])
                        + rot[9] * ferVec3Y(&obb->axis[0])
                        + rot[10] * ferVec3Z(&obb->axis[0]);
            rot2[9] = rot[8] * ferVec3X(&obb->axis[1])
                        + rot[9] * ferVec3Y(&obb->axis[1])
                        + rot[10] * ferVec3Z(&obb->axis[1]);
            rot2[10] = rot[8] * ferVec3X(&obb->axis[2])
                        + rot[9] * ferVec3Y(&obb->axis[2])
                        + rot[10] * ferVec3Z(&obb->axis[2]);

            dsSetColorAlpha (1,0,0,0.3);
            dsDrawBox(pos2, rot2, sides);
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            _drawG(g, o);
        }
    }
}

static void drawG(fer_cd_geom_t *g)
{
    fer_list_t *item;
    fer_cd_obb_t *obb;

    FER_LIST_FOR_EACH(&g->obbs, item){
        obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        _drawG(g, obb);
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
        fprintf(stdout, "# [%02d] %lx-%lx dir: <%f %f %f>, pos: <%f %f %f>, depth: %f\n",
                (int)i, (long)b1, (long)b2,
                ferVec3X(&con->dir[i]), ferVec3Y(&con->dir[i]), ferVec3Z(&con->dir[i]),
                ferVec3X(&con->pos[i]), ferVec3Y(&con->pos[i]), ferVec3Z(&con->pos[i]),
                con->depth[i]);

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
        /*
        contact.surface.mode = dContactBounce | dContactSoftCFM;
        contact.surface.mu = dInfinity;
        contact.surface.mu2 = 0;
        contact.surface.bounce = 0.1;
        contact.surface.bounce_vel = 0.1;
        contact.surface.soft_cfm = 0.01;
        */
   
        contact.surface.mu = dInfinity;
        contact.surface.mode = dContactSoftCFM | dContactApprox1_1 | dContactApprox1_2;
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
    printf ("To toggle showing the geom OBBs, press o.\n");
    printf ("To toggle showing the contact points, press t.\n");
}

static void command (int cmd);

static void simLoop(int _pause)
{
    fer_list_t *item;
    obj_t *obj;
    static long i = 0;

    if (!_pause && !paus)
        fprintf(stderr, "Step: %06ld\n", i++);

    if (!no_sleep)
        usleep(10000);

    if (!no_win)
        dsSetColor (0,0,2);

    if (!_pause && !paus){
        if (cmds && *cmds != 0x0){
            command(*cmds);
            ++cmds;
        }

        // remove all contact joints
        dJointGroupEmpty(contactgroup);

        ferTimerStart(&loop_timer);
        if (use_ode){
            dSpaceCollide(space, 0, &nearCallback);
        }else{
            ferCDSeparate(cd, sepCB, NULL);
#ifdef FER_CD_TIME_MEASURE
            fprintf(stderr, "Time (us):\n");
            fprintf(stderr, "    update dirty: %f\n", (float)cd->time_update_dirty);
            fprintf(stderr, "    radix sort:   %f [%f + %f + %f]\n",
                    (float)(cd->time_radix[0] + cd->time_radix[1] + cd->time_radix[2]),
                    (float)cd->time_radix[0],
                    (float)cd->time_radix[1],
                    (float)cd->time_radix[2]);
            fprintf(stderr, "    remove pairs: %f\n", (float)cd->time_remove_pairs);
            fprintf(stderr, "    find pairs:   %f\n", (float)cd->time_find_pairs);
            fprintf(stderr, "    all:          %f\n", (float)cd->time_separate);

            fprintf(meas, "%ld %f %f %f %f %f %f %f\n",
                    i,
                    (float)cd->time_update_dirty,
                    (float)cd->time_radix[0],
                    (float)cd->time_radix[1],
                    (float)cd->time_radix[2],
                    (float)cd->time_remove_pairs,
                    (float)cd->time_find_pairs,
                    (float)cd->time_separate);
            fflush(meas);
#endif /* FER_CD_TIME_MEASURE */
        }
        ferTimerStop(&loop_timer);
        fprintf(stderr, "%lu us\n", ferTimerElapsedInUs(&loop_timer));

        dWorldQuickStep (world,0.02);
    }


    if (!no_win){
        dsSetColor (1,1,0);
        dsSetTexture (DS_WOOD);

        FER_LIST_FOR_EACH(&objs, item){
            obj = FER_LIST_ENTRY(item, obj_t, list);
            if (obj->g){
                drawG(obj->g);
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

        g = NULL;
        geom = 0;
        if (!use_ode)
            g = ferCDGeomNew(cd);

        if (cmd == 'b'){
            dMassSetBox(&m, DENSITY, sides[0], sides[1], sides[2]);
            if (use_ode){
                geom = dCreateBox(space, sides[0], sides[1], sides[2]);
            }else{
                ferCDGeomAddBox(cd, g, sides[0], sides[1], sides[2]);
            }
        }else if (cmd == 's'){
            dMassSetSphere(&m, DENSITY, sides[0]);
            if (use_ode){
                geom = dCreateSphere(space, sides[0]);
            }else{
                ferCDGeomAddSphere(cd, g, sides[0]);
            }
        }else if (cmd == 'y'){
            dMassSetCylinder (&m,DENSITY,3,sides[0],sides[1]);
            if (use_ode){
                geom = dCreateCylinder (space,sides[0],sides[1]);
            }else{
                ferCDGeomAddCyl(cd, g, sides[0], sides[1]);
            }
        }else if (cmd == 'c'){
            sides[0] *= 0.5;
            dMassSetCapsule(&m,DENSITY,3,sides[0],sides[1]);
            if (use_ode){
                geom = dCreateCapsule(space,sides[0],sides[1]);
            }else{
                ferCDGeomAddCap(cd, g, sides[0], sides[1]);
            }
        }else if (cmd == 'm'){
            dMassSetCylinder(&m,DENSITY,3,sides[0],sides[1]);
            if (use_ode){
                geom = 0;
            }else{
                ferCDGeomAddTriMesh(cd, g, bunny_coords, bunny_ids, bunny_tri_len);
            }
        }

        dBodySetMass(body, &m);
        objNew(body, geom, g);

    }else if (cmd == 'q'){
        exit(-1);

    }else if (cmd == 'o') {
        show_obb ^= 1;
    }else if (cmd == 't') {
        show_contacts ^= 1;
    }else if (cmd == 'v'){
        dsGetViewpoint(xyz, hpr);
        printf("xyz: %f %f %f\nhpr: %f %f %f\n",
               xyz[0], xyz[1], xyz[2],
               hpr[0], hpr[1], hpr[2]);
        fflush(stdout);
    }else if (cmd == 'p'){
        paus ^= 1;
    }else if (cmd == 'a'){
        alpha ^= 1;
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
        //ferCDGeomContactPersistence(cd, g, 4);


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

static void optNoPlane(const char *l, char s)
{
    static fer_vec3_t plane_pts[] = {
        FER_VEC3_STATIC(-5, -5, 0.2),
        FER_VEC3_STATIC(5, -5, 0.2),
        FER_VEC3_STATIC(5, 5, 0.2),
        FER_VEC3_STATIC(-5, 5, 0.2)
    };
    static unsigned int plane_ids[] = {
        0, 1, 2,
        2, 3, 0
    };
    static size_t plane_num_tris = 2;

    fer_cd_geom_t *g;

    no_plane = 1;

    if (!cd)
        cd = ferCDNew(&params);

    g = ferCDGeomNew(cd);
    ferCDGeomAddTriMesh(cd, g, plane_pts, plane_ids, plane_num_tris);
    ferCDGeomContactPersistence(cd, g, 10);

    objNew(0, 0, g);
}

static void opt(void)
{
    ferCDParamsInit(&params);

    params.use_sap = 1;
    params.sap_hashsize = 1023 * 1023 * 10 + 1;
    params.use_cp = 1;

    ferOptsAdd("use-gpu", 0, FER_OPTS_NONE, &params.sap_gpu, NULL);
    ferOptsAdd("use-cp", 0, FER_OPTS_NONE, &params.use_cp, NULL);
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
    ferOptsAdd("pause", 0, FER_OPTS_NONE, &paus, NULL);
    ferOptsAdd("num-steps", 0, FER_OPTS_LONG, &num_steps, NULL);
    ferOptsAdd("no-plane", 0, FER_OPTS_NONE, NULL, FER_OPTS_CB(optNoPlane));

    ferOpts(&pargc, pargv);
}

int main (int argc, char **argv)
{
    long i;

    // setup pointers to drawstuff callback functions
    dsFunctions fn;
    fn.version = DS_VERSION;
    fn.start = &start;
    fn.step = &simLoop;
    fn.command = &command;
    fn.stop = 0;
    fn.path_to_textures = DRAWSTUFF_TEXTURE_PATH;

#ifdef FER_CD_TIME_MEASURE
    meas = fopen("test-cd-ode.meas", "w");
    fprintf(meas, "# update_dirty radix[0] radix[1] radix[2] remove_pairs find_pairs separate\n");
#endif /* FER_CD_TIME_MEASURE */

    // create world
    dInitODE2(0);
    world = dWorldCreate();
    space = dHashSpaceCreate(0);
    //space = dSimpleSpaceCreate(0);
    contactgroup = dJointGroupCreate(0);
    dWorldSetGravity(world, 0, 0, -GRAVITY);
    /*
    dWorldSetCFM(world, 1e-5);
    dWorldSetAutoDisableFlag(world, 1);
    dWorldSetAutoDisableAverageSamplesCount(world, 10);

    dWorldSetLinearDamping(world, 0.00001);
    dWorldSetAngularDamping(world, 0.005);
    dWorldSetMaxAngularSpeed(world, 200);

    dWorldSetContactMaxCorrectingVel(world, 0.1);
    dWorldSetContactSurfaceLayer(world, 0.001);
    */

    dWorldSetERP(world, 0.2);
    dWorldSetCFM(world, 1e-5);
    //dWorldSetERP(_world, 0.5);
    //dWorldSetCFM(_world, 0.001);
    //dWorldSetCFM(_world, 0.01);

    pargc = argc;
    pargv = argv;
    opt();

    // init collide library
    if (!cd)
        cd = ferCDNew(&params);

    if (!no_plane){
        plane.g = ferCDGeomNew(cd);
        ferCDGeomAddPlane(cd, plane.g);
        ferCDGeomSetData(plane.g, (void *)&plane);
        plane.geom = dCreatePlane(space, 0, 0, 1, 0);
        plane.body = NULL;
        /*
        fer_cd_geom_t *g;
        g = ferCDGeomNew(cd);
        ferCDGeomAddBox(cd, g, 20, 20, 0.2);
        //ferCDGeomAddBox(cd, g, 0.2, 20, 20);
        //ferCDGeomSetRotEuler(cd, g, 0, M_PI_2, 0);
        objNew(0, 0, g);
        */
    }


    // run simulation
    if (no_win){
        for (i = 0; i < num_steps; i++){
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

    ferOptsClear();

#ifdef FER_CD_TIME_MEASURE
    fclose(meas);
#endif /* FER_CD_TIME_MEASURE */

    return 0;
}
