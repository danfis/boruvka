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

#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <boruvka/image.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

#define BOR_IMAGE_PGMF 2
#define BOR_IMAGE_PPMF 3


#define CHECKWS(c) \
    ((c) == ' ' \
        || (c) == '\n' \
        || (c) == '\r' \
        || (c) == '\t')

bor_image_pnmf_t *borImagePNMF(const char *filename)
{
    bor_image_pnmf_t *img = NULL;
    int width, height, maxval;
    int i, len, size, type;
    int fd;
    struct stat st;
    void *file;
    char *data;

    // open file
    if ((fd = open(filename, O_RDONLY)) == -1){
        fprintf(stderr, "Image: Can't open file `%s'\n", filename);
        return NULL;
    }

    // get size of file
    if (fstat(fd, &st) == -1){
        fprintf(stderr, "Image (`%s'): Can't read file size.\n", filename);
        close(fd);
        return NULL;
    }

    // mmap whole file
    file = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file == MAP_FAILED){
        fprintf(stderr, "Image (`%s'): Can't map file to memory.\n", filename);
        close(fd);
        return NULL;
    }

    if (strncmp((const char *)file, "P1", 2) == 0){
        fprintf(stderr, "Image: TODO: PBM ASCII version\n");
    }else if (strncmp((const char *)file, "P2", 2) == 0){
        fprintf(stderr, "Image: TODO: PGM ASCII version\n");
    }else if (strncmp((const char *)file, "P3", 2) == 0){
        fprintf(stderr, "Image: TODO: PPM ASCII version\n");
    }else if (strcmp((const char *)file, "P4") == 0){
        fprintf(stderr, "Image: TODO: PBM BIN version\n");
    }else if (strncmp((const char *)file, "P5", 2) == 0
                || strncmp((const char *)file, "P6", 2) == 0){
        data = (char *)file;
        data += 2; // point after magic number

        // skip whitespace
        while (CHECKWS(*data))
            ++data;

        // get width
        sscanf(data, "%d", &width);
        while (!CHECKWS(*data))
            ++data;
        while (CHECKWS(*data))
            ++data;

        // get height
        sscanf(data, "%d", &height);
        while (!CHECKWS(*data))
            ++data;
        while (CHECKWS(*data))
            ++data;

        // get maxval
        sscanf(data, "%d", &maxval);
        while (!CHECKWS(*data))
            ++data;
        while (CHECKWS(*data))
            ++data;

        if (strncmp((const char *)file, "P5", 2) == 0){
            size = width * height * sizeof(float);
            type = BOR_IMAGE_PGMF;
        }else{
            size = width * height * sizeof(float) * 3;
            type = BOR_IMAGE_PPMF;
        }

        img = _BOR_ALLOC_MEMORY(bor_image_pnmf_t, NULL, sizeof(bor_image_pnmf_t) * size);
        img->type   = type;
        img->width  = width;
        img->height = height;
        img->data   = (float *)((char *)img + sizeof(bor_image_pnmf_t));

        len = width * height;
        if (maxval <= 255){
            for (i = 0; i < len; i++){
                if (img->type == BOR_IMAGE_PGMF){
                    img->data[i] = (float)*(unsigned char *)data / (float)maxval;
                    ++data;
                }else if (img->type == BOR_IMAGE_PPMF){
                    img->data[3 * i]     = (float)*(unsigned char *)data / (float)maxval;
                    ++data;
                    img->data[3 * i + 1] = (float)*(unsigned char *)data / (float)maxval;
                    ++data;
                    img->data[3 * i + 2] = (float)*(unsigned char *)data / (float)maxval;
                    ++data;
                }
            }
        }else{
            // TODO: 2-byte PGM
            fprintf(stderr, "Image: TODO: 2-byte PNM version\n");
        }
    }else{
        fprintf(stderr, "Image (`%s'): Not a PNM file!\n", filename);
        munmap(file, st.st_size);
        close(fd);
        return NULL;
    }

    // release resources
    munmap(file, st.st_size);
    close(fd);

    return img;
}

void borImagePNMFDel(bor_image_pnmf_t *img)
{
    BOR_FREE(img);
}

void borImagePNMFSave(bor_image_pnmf_t *img, const char *filename)
{
    FILE *fout;
    int i, len;

    fout = fopen(filename, "w");
    if (!fout){
        fprintf(stderr, "Image: Can't open file `%s'.\n", filename);
        return;
    }

    len = img->width * img->height;

    if (img->type == BOR_IMAGE_PGMF){
        fprintf(fout, "P5\n%d %d\n255\n", img->width, img->height);
        for (i = 0; i < len; i++){
            fprintf(fout, "%c", (char)(img->data[i] * 255));
        }
    }else if (img->type == BOR_IMAGE_PPMF){
        fprintf(fout, "P6\n%d %d\n255\n", img->width, img->height);
        for (i = 0; i < len; i++){
            fprintf(fout, "%c", (char)(img->data[3 * i] * 255));
            fprintf(fout, "%c", (char)(img->data[3 * i + 1] * 255));
            fprintf(fout, "%c", (char)(img->data[3 * i + 2] * 255));
        }
    }


    fclose(fout);
}


static float rgbToGray(float r, float g, float b)
{
    // Three methods:
    // 1. The lightness method averages the most prominent and least
    // prominent colors: (max(R, G, B) + min(R, G, B)) / 2.
    //
    // 2. The average method simply averages the values: (R + G + B) / 3.
    //
    // 3. The luminosity method is a more sophisticated version of the
    // average method. It also averages the values, but it forms a weighted
    // average to account for human perception. We’re more sensitive to
    // green than other colors, so green is weighted most heavily. The
    // formula for luminosity is 0.21 R + 0.71 G + 0.07 B.

    return (r * 0.2125) + (g * 0.7154) + (b * 0.0721);
}

static void grayToRGB(float gray, float *r, float *g, float *b)
{
    *r = *g = *b = gray;
}

void borImagePNMFGetRGB(const bor_image_pnmf_t *img, int row, int col,
                        float *r, float *g, float *b)
{
    borImagePNMFGetRGB2(img, row * img->width + col, r, g, b);
}

void borImagePNMFSetRGB(const bor_image_pnmf_t *img, int row, int col,
                        float r, float g, float b)
{
    borImagePNMFSetRGB2(img, row * img->width + col, r, g, b);
}

float borImagePNMFGetGray(const bor_image_pnmf_t *img, int row, int col)
{
    return borImagePNMFGetGray2(img, row * img->width + col);
}

void borImagePNMFSetGray(const bor_image_pnmf_t *img, int row, int col, float gr)
{
    borImagePNMFSetGray2(img, row * img->width + col, gr);
}

void borImagePNMFGetRGB2(const bor_image_pnmf_t *img, int pos,
                         float *r, float *g, float *b)
{
    if (img->type == BOR_IMAGE_PGMF){
        grayToRGB(img->data[pos], r, g, b);
    }else{
        pos *= 3;
        *r = img->data[pos];
        *g = img->data[pos + 1];
        *b = img->data[pos + 2];
    }
}

void borImagePNMFSetRGB2(const bor_image_pnmf_t *img, int pos,
                         float r, float g, float b)
{
    if (img->type == BOR_IMAGE_PGMF){
        img->data[pos] = rgbToGray(r, g, b);
    }else{
        pos *= 3;
        img->data[pos]     = r;
        img->data[pos + 1] = g;
        img->data[pos + 2] = b;
    }
}

float borImagePNMFGetGray2(const bor_image_pnmf_t *img, int pos)
{
    if (img->type == BOR_IMAGE_PGMF){
        return img->data[pos];
    }else{
        pos *= 3;
        return rgbToGray(img->data[pos], img->data[pos + 1], img->data[pos + 2]);
    }
}

void borImagePNMFSetGray2(const bor_image_pnmf_t *img, int pos, float gr)
{
    if (img->type == BOR_IMAGE_PGMF){
        img->data[pos] = gr;
    }else{
        pos *= 3;
        grayToRGB(gr, &img->data[pos], &img->data[pos + 1], &img->data[pos + 2]);
    }
}
