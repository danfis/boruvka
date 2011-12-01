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

#ifndef FER_CFG_MAP_H_
#define FER_CFG_MAP_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int ferCfgMapInit(const char *fn);
void ferCfgMapDestroy(void);
int ferCfgMapRobot(const char *name, fer_real_t *h,
                   fer_vec_t *init, fer_vec_t *goal);
void ferCfgMapListRobots(FILE *out);
int ferCfgMapCollide(const fer_vec_t *conf);
const fer_vec_t *ferCfgConf(void);

const fer_real_t *ferCfgMapAABB(void);
int ferCfgDim(void);
int ferCfgConfDim(void);
void ferCfgUseRot(void);

void ferCfgMapDumpSVT(FILE *out, const char *name);
void ferCfgMapRobotDumpSVT(const fer_vec_t *conf, FILE *out, const char *name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FER_CFG_MAP_H_ */
