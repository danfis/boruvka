#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <signal.h>
#include <fermat/nnbp.h>
#include <fermat/image.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>


static SDL_Surface *screen = NULL;
static int end = 0;

#define IMG_TILE_WIDTH 5
#define IMG_TILE_HEIGHT 5
#define IMG_TILE_LEN (IMG_TILE_WIDTH * IMG_TILE_HEIGHT)
#define IMG_BW_TRESHOLD 0.5
#define IMG_HIDDEN1 6
#define IMG_HIDDEN2 10
#define IMG_CYCLES 5
#define IMG_MAX_EPOCHS 10000
#define IMG_OUT_PERIOD 10

int binomialNumber(int n, int k)
{
    int i, val;

    if (k == 0)
        return n;

    val = 1;
    for (i = n; i > k; i--){
        val *= i;
    }
    for (i = 2; i <= (n - k); i++){
        val /= i;
    }

    return val;
}

int maxPatterns(int h1, int h2)
{
    int val = 0, i;

    if (h1 >= h2)
        return -1;

    for (i = 0; i < h1; i++)
        val += binomialNumber(h2, i);

    return val;
}

struct _img_tile_t {
    fer_vec_t *v;
    int start;
};
typedef struct _img_tile_t img_tile_t;

static void imgSigint(int _)
{
    char cmd[100];
    printf("\nYou wish? [end|flip]: ");
    fflush(stdout);
    fgets(cmd, 100, stdin);

    if (strncmp(cmd, "end", 3) == 0){
        end = 1;
    }else if (strncmp(cmd, "flip", 4) == 0){
        SDL_Flip(screen);
    }
}

static float color(float col)
{
    //return col;
    if (col < IMG_BW_TRESHOLD){
        return 0.f;
    }else{
        return 1.f;
    }
}

static void imgTiles(img_tile_t *tiles, size_t len, const fer_image_pnmf_t *img)
{
    size_t i;
    int x, y, pos, w, k, l, p;
    float col;

    w = img->width / IMG_TILE_WIDTH;

    for (i = 0; i < len; i++){
        tiles[i].v = ferVecNew(IMG_TILE_LEN);
        x = i / w;
        y = i % w;
        tiles[i].start = (x * IMG_TILE_LEN * w) + (y * IMG_TILE_WIDTH);

        p = 0;
        for (k = 0; k < IMG_TILE_WIDTH; k++){
            pos = tiles[i].start + (k * img->width);
            for (l = 0; l < IMG_TILE_HEIGHT; l++, pos++){
                col = ferImagePNMFGetGray2(img, pos);
                ferVecSet(tiles[i].v, p++, color(col));
            }
        }
    }
}

static void imgSetTile(fer_image_pnmf_t *img, fer_nnbp_t *nn,
                       const img_tile_t *tiles, size_t i)
{
    int pos, k, l, p;
    float col;
    const fer_vec_t *vout;

    vout = ferNNBPFeed(nn, tiles[i].v);

    p = 0;
    for (k = 0; k < IMG_TILE_WIDTH; k++){
        pos = tiles[i].start + (k * img->width);
        for (l = 0; l < IMG_TILE_HEIGHT; l++, pos++){
            col = ferVecGet(vout, p++);
            ferImagePNMFSetGray2(img, pos, color(col));
        }
    }
}

static void imgTilesSave(const img_tile_t *tiles, size_t len,
                         fer_image_pnmf_t *img, const char *fn)
{
    size_t i;
    int pos, k, l, p;
    float col;

    for (i = 0; i < len; i++){
        p = 0;
        for (k = 0; k < IMG_TILE_WIDTH; k++){
            pos = tiles[i].start + (k * img->width);
            for (l = 0; l < IMG_TILE_HEIGHT; l++, pos++){
                col = ferVecGet(tiles[i].v, p++);
                ferImagePNMFSetGray2(img, pos, color(col));
            }
        }
    }

    ferImagePNMFSave(img, fn);
}


static void imgTilesDel(img_tile_t *tiles, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++){
        ferVecDel(tiles[i].v);
    }
}

static void imgTilesRecascade(fer_nnbp_t *nn, const img_tile_t *tiles, size_t len,
                              fer_image_pnmf_t *img, const char *fn)
{
    int tile;
    SDL_Surface *image;
    SDL_Rect dest;

    for (tile = 0; tile < len; tile++){
        imgSetTile(img, nn, tiles, tile);
    }
    ferImagePNMFSave(img, fn);

    system("convert in.pgm out.pgm +append in-out.pgm");

    image = IMG_Load("out.pgm");
    dest.x = img->width;
    dest.y = 0;
    dest.w = image->w;
    dest.h = image->h;
    SDL_BlitSurface(image, NULL, screen, &dest);

    /* Updatovat změněnou část obrazovky */
    SDL_UpdateRects(screen, 1, &dest);

    SDL_Flip(screen);
}

int usage(int argc, char *argv[])
{
    fprintf(stderr, "Usage: %s image\n", argv[0]);
    return -1;
}

int main(int argc, char *argv[])
{
    fer_nnbp_params_t params;
    fer_nnbp_t *nn;
    fer_image_pnmf_t *img;
    size_t layer_size[] = { IMG_TILE_LEN, IMG_HIDDEN1, IMG_HIDDEN2, IMG_TILE_LEN };
    img_tile_t *tiles;
    size_t tiles_len;
    int i, epoch, c, tile;
    fer_real_t err, minerr, maxerr, avgerr;
    SDL_Surface *image;
    SDL_Rect dest;

    if (argc != 2){
        return usage(argc, argv);
    }

    signal(SIGINT, imgSigint);


    img = ferImagePNMF(argv[1]);
    if (!img){
        fprintf(stderr, "File `%s' not found\n", argv[1]);
        ferImagePNMFDel(img);
        return -1;
    }else{
        printf("File `%s': %dx%d px\n", argv[1], img->width, img->height);
    }
    if (img->width % IMG_TILE_WIDTH != 0
            || img->height % IMG_TILE_HEIGHT != 0){
        fprintf(stderr, "Image is %dx%d pixels, but tile should be %dx%d pixels\n",
                img->width, img->height, IMG_TILE_WIDTH, IMG_TILE_HEIGHT);
        ferImagePNMFDel(img);
        return -1;
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetVideoMode(img->width * 2, img->height, 16, SDL_DOUBLEBUF);
    screen = SDL_GetVideoSurface();


    tiles_len = (img->width * img->height) / IMG_TILE_LEN;
    tiles = FER_ALLOC_ARR(img_tile_t, tiles_len);
    imgTiles(tiles, tiles_len, img);
    printf("Tiles: %d\n", (int)tiles_len);

    imgTilesSave(tiles, tiles_len, img, "in.pgm");
    image = IMG_Load("in.pgm");
    dest.x = 0;
    dest.y = 0;
    dest.w = image->w;
    dest.h = image->h;
    SDL_BlitSurface(image, NULL, screen, &dest);
    SDL_UpdateRects(screen, 1, &dest);
    SDL_Flip(screen);


    ferNNBPParamsInit(&params);
    params.layers_num = sizeof(layer_size) / sizeof(size_t);
    params.layer_size = layer_size;
    params.eta = 0.3;
    params.alpha = 0.7;
    params.lambda = 1.;
    params.func = FER_NNBP_SIGMOID;
    printf("Layers: %d", (int)params.layer_size[0]);
    for (i = 1; i < params.layers_num; i++){
        printf(" -> %d", (int)params.layer_size[i]);
    }
    printf(" | max patterns: %d\n", maxPatterns(params.layer_size[1], params.layer_size[2]));
    fflush(stdout);

    nn = ferNNBPNew(&params);

    imgTilesRecascade(nn, tiles, tiles_len, img, "out.pgm");

    for (epoch = 0; epoch < IMG_MAX_EPOCHS && !end; epoch++){
        maxerr = -FER_REAL_MAX;
        minerr = FER_REAL_MAX;
        avgerr = FER_ZERO;
        for (tile = 0; tile < tiles_len; tile++){
            for (c = 0; c < IMG_CYCLES; c++){
                ferNNBPLearn(nn, tiles[tile].v, tiles[tile].v);
            }

            err = ferNNBPErr(nn, tiles[tile].v);
            if (err > maxerr)
                maxerr = err;
            if (err < minerr)
                minerr = err;
            avgerr += err;
        }
        avgerr /= (fer_real_t)tiles_len;
        printf("[%02d] %f - %f - %f\n", (int)epoch, (float)minerr, (float)avgerr, (float)maxerr);
        fflush(stdout);

        if (epoch % IMG_OUT_PERIOD == 0){
            imgTilesRecascade(nn, tiles, tiles_len, img, "out.pgm");
        }
    }
    imgTilesRecascade(nn, tiles, tiles_len, img, "out.pgm");


    ferNNBPDel(nn);

    imgTilesDel(tiles, tiles_len);
    free(tiles);
    ferImagePNMFDel(img);


    SDL_Quit();

    return 0;
}
