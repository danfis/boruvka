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

#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <fermat/image.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

#define CHECKWS(c) \
    ((c) == ' ' \
        || (c) == '\n' \
        || (c) == '\r' \
        || (c) == '\t')

fer_image_pgmf_t *ferImagePGMF(const char *filename)
{
    fer_image_pgmf_t *img = NULL;
    int width, height, maxval;
    int i, len;
    int fd;
    struct stat st;
    void *file;
    char *data;

    // open file
    if ((fd = open(filename, O_RDONLY)) == -1){
        fprintf(stderr, "PGMF: Can't open file `%s'\n", filename);
        return NULL;
    }

    // get size of file
    if (fstat(fd, &st) == -1){
        fprintf(stderr, "PGMF (`%s'): Can't read file size.\n", filename);
        close(fd);
        return NULL;
    }

    // mmap whole file
    file = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file == MAP_FAILED){
        fprintf(stderr, "PGMF (`%s'): Can't map file to memory.\n", filename);
        close(fd);
        return NULL;
    }

    if (strncmp((const char *)file, "P2", 2) == 0){
        // TODO: ASCII version
        fprintf(stderr, "PGMF: TODO: ASCII version\n");
    }else if (strncmp((const char *)file, "P5", 2) == 0){
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

        img = _FER_ALLOC_MEMORY(fer_image_pgmf_t, NULL,
                                sizeof(fer_image_pgmf_t) + (width * height * sizeof(float)));
        img->width  = width;
        img->height = height;
        img->data   = (float *)((char *)img + sizeof(fer_image_pgmf_t));

        len = width * height;
        if (maxval <= 255){
            for (i = 0; i < len; i++){
                img->data[i] = (float)*(unsigned char *)data / (float)maxval;
                ++data;
            }
        }else{
            // TODO: 2-byte PGM
            fprintf(stderr, "PGMF: TODO: 2-byte version\n");
        }
    }else{
        fprintf(stderr, "PGMF (`%s'): Not a PGM file!\n", filename);
        munmap(file, st.st_size);
        close(fd);
        return NULL;
    }

    // release resources
    munmap(file, st.st_size);
    close(fd);

    return img;
}


void ferImagePGMFDel(fer_image_pgmf_t *p)
{
    FER_FREE(p);
}

void ferImagePGMFSave(fer_image_pgmf_t *img, const char *filename)
{
    FILE *fout;
    int i, len;

    fout = fopen(filename, "w");
    if (!fout)
        return;

    fprintf(fout, "P5\n%d %d\n255\n", img->width, img->height);

    len = img->width * img->height;
    for (i = 0; i < len; i++){
        fprintf(fout, "%c", (char)(img->data[i] * 255));
    }

    fclose(fout);
}
