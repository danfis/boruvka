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
#include <fermat/timer.h>
#include <fermat/dbg.h>
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

#define NUM 100000			// max number of objects
#define DENSITY (5.0)		// density of all objects
#define GPB 3			// maximum number of geometries per body
#define MAX_CONTACTS 8          // maximum number of contact points per body
#define MAX_FEEDBACKNUM 20
#define GRAVITY         REAL(0.5)
#define USE_GEOM_OFFSET 1

int num_boxes[2] = {0, 0};
int num_spheres[2] = {0, 0};


// dynamics and collision objects

struct MyObject {
  dBodyID body;			// the body
  dGeomID geom[GPB];		// geometries representing this body
  fer_cd_geom_t *g[GPB];
};

static fer_cd_t *cd;
static int num=0;		// number of objects in simulation
static int nextobj=0;		// next object to recycle if num==NUM
static dWorldID world;
static dSpaceID space;
static struct MyObject obj[NUM];
static struct MyObject plane;
static dJointGroupID contactgroup;
static int selected = -1;	// selected object
static int show_aabb = 0;	// show geom AABBs?
static int show_contacts = 0;	// show contact points?
static int random_pos = 1;	// drop objects from random position?
static int show_body = 0;
static int use_sleep = 1;
static int use_gl = 1;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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
      if (show_contacts) dsDrawBox (contact[i].geom.pos,RI,ss);

    }
  }
}


static int sepCB(const fer_cd_t *cd,
                 const fer_cd_geom_t *g1, const fer_cd_geom_t *g2,
                 const fer_cd_contacts_t *con,
                 void *data)
{
    struct MyObject *obj1, *obj2;
    size_t i;
    dContact contact;
    dBodyID b1, b2;
   
    obj1 = (struct MyObject *)ferCDGeomData((fer_cd_geom_t *)g1);
    obj2 = (struct MyObject *)ferCDGeomData((fer_cd_geom_t *)g2);
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

        if (use_gl && show_contacts){
            dMatrix3 RI;
            dRSetIdentity (RI);
            const dReal ss[3] = {0.02,0.02,0.02};

            dsDrawBox(contact.geom.pos, RI, ss);
        }
    }

    return 0;
}

static void bodyMoved(dBodyID body)
{
    struct MyObject *obj = (struct MyObject *)dBodyGetData(body);
    size_t i;
    const dReal *pos, *q;
    fer_mat3_t rot;
    fer_quat_t quat;

    pos = dBodyGetPosition(body);
    q = dBodyGetQuaternion(body);

    ferQuatSet(&quat, q[1], q[2], q[3], q[0]);
    ferQuatToMat3(&quat, &rot);

    for (i = 0; i < GPB; i++){
        if (obj->g[i]){
            ferCDGeomSetTr3(cd, obj->g[i], pos[0], pos[1], pos[2]);
            ferCDGeomSetRot(cd, obj->g[i], &rot);
        }
    }
}


// start simulation - set viewpoint

static void initBoxes(void)
{
    size_t i;
    dReal sides[3];
    dMatrix3 R;
    dMass m;

    for (i = 0; i < num_boxes[0] * num_boxes[1]; i++){
        obj[num].body = dBodyCreate(world);
        sides[0] = 0.1;
        sides[1] = 0.2;
        sides[2] = 0.3;

        dBodySetPosition(obj[num].body, 0.4 * (i / num_boxes[0]),
                                      0.4 * (i % num_boxes[0]), 1.);
        dRFromAxisAndAngle(R, dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
                              dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
        dBodySetRotation(obj[num].body,R);

        dBodySetData(obj[num].body, (void*)&obj[num]);
        dBodySetMovedCallback(obj[num].body, bodyMoved);

        obj[num].geom[0] = dCreateBox (space,sides[0],sides[1],sides[2]);
        dGeomSetBody(obj[num].geom[0], obj[num].body);

        obj[num].g[0] = ferCDGeomNew(cd);
        ferCDGeomSetData(obj[num].g[0], (void *)&obj[num]);
        ferCDGeomAddBox(cd, obj[num].g[0], sides[0], sides[1], sides[2]);

        bodyMoved(obj[num].body);

        dMassSetBox(&m,DENSITY,sides[0],sides[1],sides[2]);
        dBodySetMass(obj[num].body, &m);

        num++;
    }
}

static void initSpheres(void)
{
    size_t i;
    dMass m;

    for (i = 0; i < num_spheres[0] * num_spheres[1]; i++){
        obj[num].body = dBodyCreate(world);

        dBodySetPosition(obj[num].body, 0.4 * (i / num_spheres[0]),
                                      0.4 * (i % num_spheres[0]), 1.5);

        dBodySetData(obj[num].body, (void*)&obj[num]);
        dBodySetMovedCallback(obj[num].body, bodyMoved);

        obj[num].geom[0] = dCreateSphere(space, 0.1);
        dGeomSetBody(obj[num].geom[0], obj[num].body);

        obj[num].g[0] = ferCDGeomNew(cd);
        ferCDGeomSetData(obj[num].g[0], (void *)&obj[num]);
        ferCDGeomAddSphere(cd, obj[num].g[0], 0.1);

        bodyMoved(obj[num].body);

        dMassSetSphere(&m,DENSITY, 0.1);
        dBodySetMass(obj[num].body, &m);

        num++;
    }
}

static void start()
{
    dAllocateODEDataForThread(dAllocateMaskAll);

    /*
    static float xyz[3] = {2.1640f,-1.3079f,1.7600f};
    static float hpr[3] = {125.5000f,-17.0000f,0.0000f};
    */
    static float xyz[3] = {1.2192,0.8433,0.4700};
    static float hpr[3] = {125.5000,-17.0000,0.0000};
    dsSetViewpoint (xyz,hpr);
    printf ("To drop another object, press:\n");
    printf ("   b for box.\n");
    printf ("   s for sphere.\n");
    printf ("   c for capsule.\n");
    printf ("   y for cylinder.\n");
    printf ("   x for a composite object.\n");
    printf ("To select an object, press space.\n");
    printf ("To disable the selected object, press d.\n");
    printf ("To enable the selected object, press e.\n");
    printf ("To dump transformation data for the selected object, press p.\n");
    printf ("To toggle showing the geom AABBs, press a.\n");
    printf ("To toggle showing the contact points, press t.\n");
    printf ("To toggle dropping from random position/orientation, press r.\n");
    printf ("To save the current state to 'state.dif', press 1.\n");
}


char locase (char c)
{
    if (c >= 'A' && c <= 'Z') return c - ('a'-'A');
    else return c;
}


// called when a key pressed

static void command (int cmd)
{
    size_t i;
    int j,k;
    dReal sides[3];
    dMass m;
    int setBody;

    cmd = locase (cmd);
    if (cmd == 'b' || cmd == 's' || cmd == 'c' || cmd == 'x' || cmd == 'y' || cmd == 'm'){
        setBody = 0;
        if (num < NUM) {
            i = num;
            num++;
        }else{
            i = nextobj;
            nextobj++;
            if (nextobj >= num)
                nextobj = 0;

            // destroy the body and geoms for slot i
            dBodyDestroy (obj[i].body);
            for (k=0; k < GPB; k++) {
                if (obj[i].geom[k])
                    dGeomDestroy (obj[i].geom[k]);
                if (obj[i].g[k])
                    ferCDGeomDel(cd, obj[i].g[k]);
            }
            memset (&obj[i],0,sizeof(obj[i]));
        }

        obj[i].body = dBodyCreate (world);
        for (k=0; k<3; k++)
            sides[k] = dRandReal()*0.5+0.1;

        dMatrix3 R;
        if (random_pos){
            dBodySetPosition(obj[i].body,
                             dRandReal()*2-1, dRandReal()*2-1, dRandReal()+2);
            dRFromAxisAndAngle(R,dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
                               dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
        }else{
            dReal maxheight = 0;
            for (k=0; k<num; k++){
                const dReal *pos = dBodyGetPosition (obj[k].body);
                if (pos[2] > maxheight) maxheight = pos[2];
            }
            dBodySetPosition (obj[i].body, 0,0,maxheight+1);
            dRSetIdentity (R);
            //dRFromAxisAndAngle (R,0,0,1,/*dRandReal()*10.0-5.0*/0);
        }
        dBodySetRotation (obj[i].body,R);
        dBodySetData (obj[i].body, (void*)&obj[i]);
        dBodySetMovedCallback(obj[i].body, bodyMoved);

        if (cmd == 'b') {
            dMassSetBox (&m,DENSITY,sides[0],sides[1],sides[2]);
            obj[i].geom[0] = dCreateBox (space,sides[0],sides[1],sides[2]);
            obj[i].g[0] = ferCDGeomNew(cd);
            ferCDGeomSetData(obj[i].g[0], (void *)&obj[i]);
            ferCDGeomAddBox(cd, obj[i].g[0], sides[0], sides[1], sides[2]);

        }else if (cmd == 'c') {
            sides[0] *= 0.5;
            dMassSetCapsule (&m,DENSITY,3,sides[0],sides[1]);
            obj[i].geom[0] = dCreateCapsule (space,sides[0],sides[1]);
            obj[i].g[0] = ferCDGeomNew(cd);
            ferCDGeomSetData(obj[i].g[0], (void *)&obj[i]);
            ferCDGeomAddCap(cd, obj[i].g[0], sides[0], sides[1]);

        }else if (cmd == 'y') {
            dMassSetCylinder (&m,DENSITY,3,sides[0],sides[1]);
            obj[i].geom[0] = dCreateCylinder (space,sides[0],sides[1]);
            obj[i].g[0] = ferCDGeomNew(cd);
            ferCDGeomSetData(obj[i].g[0], (void *)&obj[i]);
            ferCDGeomAddCyl(cd, obj[i].g[0], sides[0], sides[1]);

        }else if (cmd == 's') {
            sides[0] *= 0.5;
            dMassSetSphere (&m,DENSITY,sides[0]);
            obj[i].geom[0] = dCreateSphere (space,sides[0]);
            obj[i].g[0] = ferCDGeomNew(cd);
            ferCDGeomSetData(obj[i].g[0], (void *)&obj[i]);
            ferCDGeomAddSphere(cd, obj[i].g[0], sides[0]);

        }else if (cmd == 'm') {
            dMassSetCylinder(&m,DENSITY,3,sides[0],sides[1]);
            obj[i].geom[0] = 0;
            //obj[i].geom[1] = dCreateCylinder (space,sides[0],sides[1]);
            obj[i].g[0] = ferCDGeomNew(cd);
            ferCDGeomSetData(obj[i].g[0], (void *)&obj[i]);

            ferCDGeomAddTriMesh(cd, obj[i].g[0], bunny_coords, bunny_ids, bunny_tri_len);

        }else if (cmd == 'x' && USE_GEOM_OFFSET) {
            setBody = 1;
            // start accumulating masses for the encapsulated geometries
            dMass m2;
            dMassSetZero (&m);

            dReal dpos[GPB][3];	// delta-positions for encapsulated geometries
            dMatrix3 drot[GPB];

            // set random delta positions
            for (j=0; j<GPB; j++) {
                for (k=0; k<3; k++)
                    dpos[j][k] = dRandReal()*0.3-0.15;
            }

            for (k=0; k<GPB; k++) {
                if (k==0) {
                    dReal radius = dRandReal()*0.25+0.05;
                    obj[i].geom[k] = dCreateSphere (space,radius);
                    dMassSetSphere (&m2,DENSITY,radius);

                }else if (k==1) {
                    obj[i].geom[k] = dCreateBox (space,sides[0],sides[1],sides[2]);
                    dMassSetBox (&m2,DENSITY,sides[0],sides[1],sides[2]);
                }else{
                    dReal radius = dRandReal()*0.1+0.05;
                    dReal length = dRandReal()*1.0+0.1;
                    obj[i].geom[k] = dCreateCapsule (space,radius,length);
                    dMassSetCapsule (&m2,DENSITY,3,radius,length);
                }

                dRFromAxisAndAngle (drot[k],dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
                        dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
                dMassRotate (&m2,drot[k]);

                dMassTranslate (&m2,dpos[k][0],dpos[k][1],dpos[k][2]);

                // add to the total mass
                dMassAdd (&m,&m2);

            }

            for (k=0; k<GPB; k++) {
                dGeomSetBody (obj[i].geom[k],obj[i].body);
                dGeomSetOffsetPosition (obj[i].geom[k],
                        dpos[k][0]-m.c[0],
                        dpos[k][1]-m.c[1],
                        dpos[k][2]-m.c[2]);
                dGeomSetOffsetRotation(obj[i].geom[k], drot[k]);
            }
            dMassTranslate (&m,-m.c[0],-m.c[1],-m.c[2]);
            dBodySetMass (obj[i].body,&m);
        }

        if (!setBody){
            for (k=0; k < GPB; k++) {
                if (obj[i].geom[k])
                    dGeomSetBody (obj[i].geom[k],obj[i].body);
            }
        }

        bodyMoved(obj[i].body);

        dBodySetMass(obj[i].body, &m);
    }

    if (cmd == ' ') {
        selected++;
        if (selected >= num) selected = 0;
        if (selected < 0) selected = 0;

    }else if (cmd == 'd' && selected >= 0 && selected < num) {
        dBodyDisable (obj[selected].body);

    }else if (cmd == 'e' && selected >= 0 && selected < num) {
        dBodyEnable (obj[selected].body);
    }else if (cmd == 'a') {
        show_aabb ^= 1;

    }else if (cmd == 't') {
        show_contacts ^= 1;

    }else if (cmd == 'r') {
        random_pos ^= 1;

    }else if (cmd == 'p'&& selected >= 0){
        const dReal* pos = dGeomGetPosition(obj[selected].geom[0]);
        const dReal* rot = dGeomGetRotation(obj[selected].geom[0]);
        printf("POSITION:\n\t[%f,%f,%f]\n\n",pos[0],pos[1],pos[2]);
        printf("ROTATION:\n\t[%f,%f,%f,%f]\n\t[%f,%f,%f,%f]\n\t[%f,%f,%f,%f]\n\n",
                rot[0],rot[1],rot[2],rot[3],
                rot[4],rot[5],rot[6],rot[7],
                rot[8],rot[9],rot[10],rot[11]);
    }
}


// draw a geom

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

    }else if (type == dGeomTransformClass) {
        dGeomID g2 = dGeomTransformGetGeom (g);
        const dReal *pos2 = dGeomGetPosition (g2);
        const dReal *R2 = dGeomGetRotation (g2);
        dVector3 actual_pos;
        dMatrix3 actual_R;
        dMultiply0_331 (actual_pos,R,pos2);
        actual_pos[0] += pos[0];
        actual_pos[1] += pos[1];
        actual_pos[2] += pos[2];
        dMultiply0_333 (actual_R,R,R2);
        drawGeom (g2,actual_pos,actual_R,0);
    }

    if (show_body) {
        dBodyID body = dGeomGetBody(g);
        if (body) {
            const dReal *bodypos = dBodyGetPosition (body); 
            const dReal *bodyr = dBodyGetRotation (body); 
            dReal bodySides[3] = { 0.1, 0.1, 0.1 };
            dsSetColorAlpha(0,1,0,1);
            dsDrawBox(bodypos,bodyr,bodySides); 
        }
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


// simulation loop

static void simLoop (int pause)
{
    //static int c = 1;
    //fprintf(stderr, "c: %d\n", c++);

    if (use_sleep)
        usleep(10000);

    if (use_gl)
        dsSetColor (0,0,2);

    // remove all contact joints
    dJointGroupEmpty (contactgroup);

    ferCDSeparate(cd, sepCB, NULL);
    //dSpaceCollide (space,0,&nearCallback);

    if (!pause)
        dWorldQuickStep (world,0.02);


    if (use_gl){
        dsSetColor (1,1,0);
        dsSetTexture (DS_WOOD);
        for (int i=0; i<num; i++) {
            for (int j=0; j < GPB; j++) {
                if (i==selected) {
                    dsSetColor (0,0.7,1);
                }else if (!dBodyIsEnabled (obj[i].body)) {
                    dsSetColor (1,0.8,0);
                }else{
                    dsSetColor (1,1,0);
                }
                if (obj[i].geom[j]){
                    drawGeom (obj[i].geom[j],0,0,show_aabb);
                }else if (obj[i].g[j]){
                    drawTriMesh(obj[i].g[j], dBodyGetPosition(obj[i].body),
                                             dBodyGetRotation(obj[i].body));
                }
            }
        }
    }
}


int main (int argc, char **argv)
{
    fer_cd_params_t params;
    fer_timer_t timer;
    int i;
    int num_threads = 1;

    // setup pointers to drawstuff callback functions
    dsFunctions fn;
    fn.version = DS_VERSION;
    fn.start = &start;
    fn.step = &simLoop;
    fn.command = &command;
    fn.stop = 0;
    fn.path_to_textures = DRAWSTUFF_TEXTURE_PATH;

    if (argc > 4 && (strcmp(argv[1], "test") == 0
                        || strcmp(argv[1], "testgl") == 0)){
        num_threads = atoi(argv[4]);
    }

    ferCDParamsInit(&params);
    params.use_sap = 1;
    params.sap_size = 1023 * 1023 * 10 + 1;
    params.separate_threads = num_threads;
    cd = ferCDNew(&params);

    // create world
    dInitODE2(0);
    world = dWorldCreate();
    space = dHashSpaceCreate (0);
    contactgroup = dJointGroupCreate (0);
    dWorldSetGravity (world,0,0,-GRAVITY);
    dWorldSetCFM (world,1e-5);
    dWorldSetAutoDisableFlag (world,1);

#if 1

    dWorldSetAutoDisableAverageSamplesCount( world, 10 );

#endif

    dWorldSetLinearDamping(world, 0.00001);
    dWorldSetAngularDamping(world, 0.005);
    dWorldSetMaxAngularSpeed(world, 200);

    dWorldSetContactMaxCorrectingVel (world,0.1);
    dWorldSetContactSurfaceLayer (world,0.001);

    plane.g[0] = ferCDGeomNew(cd);
    ferCDGeomAddPlane(cd, plane.g[0]);
    ferCDGeomSetData(plane.g[0], (void *)&plane);
    plane.geom[0] = dCreatePlane (space,0,0,1,0);

    memset (obj,0,sizeof(obj));

    if (argc > 3 && strcmp(argv[1], "test") == 0){
        use_sleep = 0;
        use_gl = 0;
        num_boxes[0] = num_spheres[0] = atoi(argv[2]);
        num_boxes[1] = num_spheres[1] = atoi(argv[3]);
        initBoxes();
        initSpheres();
    }else if (argc > 3 && strcmp(argv[1], "testgl") == 0){
        use_sleep = 0;
        use_gl = 1;
        num_boxes[0] = num_spheres[0] = atoi(argv[2]);
        num_boxes[1] = num_spheres[1] = atoi(argv[3]);
        initBoxes();
        initSpheres();
    }

    // run simulation
    if (use_gl){
        dsSimulationLoop (argc,argv,800,600,&fn);
    }else{
        ferTimerStart(&timer);

        for (i = 0; i < 400; i++){
            simLoop(0);
        }

        ferTimerStop(&timer);
        fprintf(stderr, "Elapsed time: %lu us\n",
                ferTimerElapsedInUs(&timer));
    }

    dJointGroupDestroy (contactgroup);
    dSpaceDestroy (space);
    dWorldDestroy (world);
    dCloseODE();

    ferCDDel(cd);

    return 0;
}
