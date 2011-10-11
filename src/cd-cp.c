/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#include <fermat/cd.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>
#include <fermat/hfunc.h>


struct _fer_cd_cp_contact_t {
    fer_list_t hash;           /*!< Connection into hash table */
    const fer_cd_geom_t *g[2]; /*!< First and second geom */
    fer_cd_contacts_t *con;    /*!< Contacts structure */
    fer_vec3_t *relpos[2];     /*!< Array of relative positions of contacts
                                    (relpos[0,1][i] corresponds to con->pos[i]). */
    fer_list_t active;
};
typedef struct _fer_cd_cp_contact_t fer_cd_cp_contact_t;

static fer_cd_cp_contact_t *conNew(fer_cd_cp_t *cp,
                                   fer_hmap_t *cs,
                                   const fer_cd_geom_t *g1,
                                   const fer_cd_geom_t *g2);
static void conDel(fer_cd_cp_contact_t *c);

/** Returns true if {id}'s contact pos wasn't changed too much (max_dist
 *  treshold). */
static int checkCon(fer_cd_cp_contact_t *c, int id,
                    fer_real_t max_dist);
/** Copies fid's contact into cid's contact */
static void copyCon(fer_cd_cp_contact_t *c, int cid,
                    const fer_cd_contacts_t *f, int fid,
                    int swap);
/** Adds {id}'s contact into {c} */
static void addCon(fer_cd_cp_contact_t *c,
                   const fer_cd_contacts_t *con, int id,
                   int swap, size_t deepest);


struct _fer_cd_cp_contact_hash_t {
    fer_list_t hash;
    const fer_cd_geom_t *g[2];
};
typedef struct _fer_cd_cp_contact_hash_t fer_cd_cp_contact_hash_t;

static uint32_t hashKey(fer_list_t *key, void *data);
static int hashEq(const fer_list_t *key1, const fer_list_t *key2, void *data);
static fer_cd_cp_contact_t *hashGet(const fer_hmap_t *m,
                                    const fer_cd_geom_t *g1,
                                    const fer_cd_geom_t *g2);

fer_cd_cp_t *ferCDCPNew(size_t hashsize, fer_real_t max_dist)
{
    fer_cd_cp_t *cp;

    cp = FER_ALLOC(fer_cd_cp_t);
    cp->cs = ferHMapNew(hashsize, hashKey, hashEq, cp);
    cp->max_dist     = max_dist;

    ferListInit(&cp->active);
    ferListInit(&cp->unactive);

    return cp;
}

void ferCDCPDel(fer_cd_cp_t *cp)
{
    fer_list_t list, *item;
    fer_cd_cp_contact_t *c;

    ferListInit(&list);
    ferHMapGather(cp->cs, &list);
    FER_LIST_FOR_EACH(&list, item){
        c = FER_LIST_ENTRY(item, fer_cd_cp_contact_t, hash);
        conDel(c);
    }

    ferHMapDel(cp->cs);
    FER_FREE(cp);
}


const fer_cd_contacts_t *ferCDCPUpdate(fer_cd_cp_t *cp,
                                       const fer_cd_geom_t *g1,
                                       const fer_cd_geom_t *g2,
                                       const fer_cd_contacts_t *con)
{
    fer_cd_cp_contact_t *c;
    uint8_t swap;
    size_t i, deepest = 0;

    if (!cp || (g1->cp == 0 && g2->cp == 0))
        return con;

    DBG2("");
    // find out or create contacts between g1 and g2
    c = hashGet(cp->cs, g1, g2);
    if (c == NULL){
        c = conNew(cp, cp->cs, g1, g2);
    }

    // remove all incorrect contacts and find the deepest one
    for (i = 0; i < c->con->num;){
        if (!checkCon(c, i, cp->max_dist)){
            if (i != c->con->num - 1){
                ferVec3Copy(&c->con->pos[i], &c->con->pos[c->con->num - 1]);
                ferVec3Copy(&c->con->dir[i], &c->con->dir[c->con->num - 1]);
                c->con->depth[i] = c->con->depth[c->con->num - 1];
            }
            c->con->num--;
        }else{
            if (c->con->depth[i] > c->con->depth[deepest])
                deepest = i;
            ++i;
        }
    }

    // set to true if g1 and g2 are swapped compared to c->con->g[0,1]
    swap = (c->g[0] != g1);

    // adds contacts point
    for (i = 0; i < con->num; i++){
        addCon(c, con, i, swap, deepest);
    }

    // move contact into active list
    ferListDel(&c->active);
    ferListAppend(&cp->active, &c->active);

    return c->con;
}

void ferCDCPMaintainance(fer_cd_cp_t *cp)
{
    fer_cd_cp_contact_t *con;
    fer_list_t *item;

    // remove contacts from unactive list
    while (!ferListEmpty(&cp->unactive)){
        item = ferListNext(&cp->unactive);
        con = FER_LIST_ENTRY(item, fer_cd_cp_contact_t, active);
        ferListDel(item);
        conDel(con);
    }

    // move all contacts from active list into unactive list
    while (!ferListEmpty(&cp->active)){
        item = ferListNext(&cp->active);
        ferListDel(item);
        ferListAppend(&cp->unactive, item);
    }
}



static fer_cd_cp_contact_t *conNew(fer_cd_cp_t *cp,
                                   fer_hmap_t *cs,
                                   const fer_cd_geom_t *g1,
                                   const fer_cd_geom_t *g2)
{
    fer_cd_cp_contact_t *c;
    size_t max_contacts;

    if (g1->cp == 0){
        max_contacts = g2->cp;
    }else if (g2->cp == 0){
        max_contacts = g1->cp;
    }else{
        max_contacts = FER_MIN(g1->cp, g2->cp);
    }

    c = FER_ALLOC(fer_cd_cp_contact_t);
    ferListInit(&c->hash);
    c->g[0] = g1;
    c->g[1] = g2;
    c->con = ferCDContactsNew(max_contacts);
    c->relpos[0] = ferVec3ArrNew(max_contacts);
    c->relpos[1] = ferVec3ArrNew(max_contacts);

    ferListAppend(&cp->unactive, &c->active);

    ferHMapPut(cs, &c->hash);

    return c;
}

static void conDel(fer_cd_cp_contact_t *c)
{
    FER_FREE(c->relpos[0]);
    FER_FREE(c->relpos[1]);
    ferListDel(&c->hash);
    ferCDContactsDel(c->con);
    FER_FREE(c);
}


static int checkCon(fer_cd_cp_contact_t *c, int id,
                    fer_real_t max_dist)
{
    fer_real_t dist;
    fer_vec3_t p;
    int i;

    for (i = 0; i < 2; i++){
        ferMat3MulVec(&p, &c->g[i]->rot, &c->relpos[i][id]);
        ferVec3Add(&p, &c->g[i]->tr);
        dist = ferVec3Dist2(&p, &c->con->pos[id]);
        if (dist > max_dist)
            return 0;
    }

    return 1;
}

static void copyCon(fer_cd_cp_contact_t *c, int cid,
                    const fer_cd_contacts_t *f, int fid,
                    int swap)
{
    int i;
    fer_vec3_t v;

    ferVec3Copy(&c->con->pos[cid], &f->pos[fid]);
    if (swap){
        ferVec3Scale2(&c->con->dir[cid], &f->dir[fid], -FER_ONE);
    }else{
        ferVec3Copy(&c->con->dir[cid], &f->dir[fid]);
    }
    c->con->depth[cid] = f->depth[fid];

    // compute relative position
    for (i = 0; i < 2; i++){
        ferVec3Sub2(&v, &f->pos[fid], &c->g[i]->tr);
        ferMat3MulVecTrans(&c->relpos[i][cid], &c->g[i]->rot, &v);
    }
}


static void addCon(fer_cd_cp_contact_t *c,
                   const fer_cd_contacts_t *con, int id,
                   int swap, size_t deepest)
{
    fer_real_t dist, mindist = FER_REAL_MAX;
    size_t i, min = 0;

    if (c->con->num < c->con->size){
        copyCon(c, c->con->num, con, id, swap);
        c->con->num++;
    }else{
        for (i = 0; i < c->con->num; i++){
            if (i == deepest)
                continue;

            dist = ferVec3Dist2(&c->con->pos[i], &con->pos[id]);
            if (dist < mindist){
                mindist = dist;
                min = i;
            }
        }
        copyCon(c, min, con, id, swap);
    }
}



static uint32_t hashKey(fer_list_t *key, void *data)
{
    fer_cd_cp_contact_t *c;
    uint32_t k;

    c = FER_LIST_ENTRY(key, fer_cd_cp_contact_t, hash);

    k = ferHashJenkins((const uint32_t *)c->g,
                       (2 * sizeof(void *)) / sizeof(uint32_t),
                       111);
    return k;
}

static int hashEq(const fer_list_t *key1, const fer_list_t *key2, void *data)
{
    const fer_cd_cp_contact_t *c1, *c2;
    c1 = FER_LIST_ENTRY(key1, const fer_cd_cp_contact_t, hash);
    c2 = FER_LIST_ENTRY(key2, const fer_cd_cp_contact_t, hash);

    return (c1->g[0] == c2->g[0] && c1->g[1] == c2->g[1])
            || (c1->g[1] == c2->g[0] && c1->g[0] == c2->g[1]);
}

static fer_cd_cp_contact_t *hashGet(const fer_hmap_t *m,
                                    const fer_cd_geom_t *g1,
                                    const fer_cd_geom_t *g2)
{
    fer_cd_cp_contact_hash_t c;
    fer_list_t *l;
    fer_cd_cp_contact_t *con = NULL;

    c.g[0] = g1;
    c.g[1] = g2;

    l = ferHMapGet(m, &c.hash);
    if (l)
        con = FER_LIST_ENTRY(l, fer_cd_cp_contact_t, hash);
    return con;
}
