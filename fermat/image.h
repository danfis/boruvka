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

#ifndef __FER_IMAGE_H__
#define __FER_IMAGE_H__

#include <fermat/core.h>
#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Image
 * ======
 *
 */

/**
 * PGM image stored in floats.
 */
struct _fer_image_pgmf_t {
    int width;
    int height;
    float *data;
};
typedef struct _fer_image_pgmf_t fer_image_pgmf_t;

/**
 * Loads PGM from file.
 */
fer_image_pgmf_t *ferImagePGMF(const char *filename);

/**
 * Deletes PGM file.
 */
void ferImagePGMFDel(fer_image_pgmf_t *img);

/**
 * Save image as given file
 */
void ferImagePGMFSave(fer_image_pgmf_t *img, const char *filename);

/**
 * Returns color from (row, col) position
 */
_fer_inline float ferImagePGMFGet(const fer_image_pgmf_t *img, int row, int col);

/**
 * Set color at (row, col) position
 */
_fer_inline void ferImagePGMFSet(const fer_image_pgmf_t *img, int row, int col, float color);


/**** INLINES ****/
_fer_inline float ferImagePGMFGet(const fer_image_pgmf_t *img, int row, int col)
{
    return img->data[row * img->width + col];
}

_fer_inline void ferImagePGMFSet(const fer_image_pgmf_t *img, int row, int col, float color)
{
    img->data[row * img->width + col] = color;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_IMAGE_H__ */
