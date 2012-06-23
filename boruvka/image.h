/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_IMAGE_H__
#define __BOR_IMAGE_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Image
 * ======
 *
 */

/**
 * PPM image
 */
struct _bor_image_pnmf_t {
    uint8_t type;
    int width;
    int height;
    float *data;
};
typedef struct _bor_image_pnmf_t bor_image_pnmf_t;

/**
 * Loads PPM from file.
 */
bor_image_pnmf_t *borImagePNMF(const char *filename);

/**
 * Deletes PPM file.
 */
void borImagePNMFDel(bor_image_pnmf_t *img);

/**
 * Save image as given file
 */
void borImagePNMFSave(bor_image_pnmf_t *img, const char *filename);

/**
 * Returns color from (row, col) position
 */
void borImagePNMFGetRGB(const bor_image_pnmf_t *img, int row, int col,
                        float *r, float *g, float *b);

/**
 * Set color at (row, col) position
 */
void borImagePNMFSetRGB(const bor_image_pnmf_t *img, int row, int col,
                        float r, float g, float b);

/**
 * Returns color from position
 */
void borImagePNMFGetRGB2(const bor_image_pnmf_t *img, int pos,
                         float *r, float *g, float *b);

/**
 * Set color at position
 */
void borImagePNMFSetRGB2(const bor_image_pnmf_t *img, int pos,
                         float r, float g, float b);

/**
 * Returns gray from position
 */
float borImagePNMFGetGray2(const bor_image_pnmf_t *img, int pos);

/**
 * Set gray at position
 */
void borImagePNMFSetGray2(const bor_image_pnmf_t *img, int pos, float gr);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_IMAGE_H__ */
