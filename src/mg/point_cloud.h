/***
 * mg
 * ---
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of mg.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __MG_PC_H__
#define __MG_PC_H__

#include <mg/vec3.h>
#include <mg/point_cloud_internal.h>

/**
 * Struct representing point cloud.
 * Point cloud is internally allocated using several memory chunks to be
 * able to represent huge amount points.
 */
struct _mg_pc_t {
    mg_list_t head; /*!< Head of list of memory chunks (mg_pc_mem_t) */
    size_t len;     /*! Overall number of points */

    mg_real_t aabb[6]; /*!< Axis aligned bounding box encapsulating point cloud
                            [x_min, x_max, y_min, y_max, z_min, z_max] */
};
typedef struct _mg_pc_t mg_pc_t;

mg_pc_t *mgPCNew(void);
void mgPCDel(mg_pc_t *);

/**
 * Adds point to point cloud.
 */
_mg_inline void mgPCAdd(mg_pc_t *pc, const mg_vec3_t *v);
void mgPCAddCoords(mg_pc_t *pc, mg_real_t x, mg_real_t y, mg_real_t z);

/**** INLINES ****/
_mg_inline void mgPCAdd(mg_pc_t *pc, const mg_vec3_t *v)
{
    mgPCAddCoords(pc, mgVec3X(v), mgVec3Y(v), mgVec3Z(v));
}

#endif /* __MG_PC_H__ */
