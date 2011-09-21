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

#include <getopt.h>
#include <fermat/gsrm.h>
#include <fermat/parse.h>


typedef enum {
    HELP = 'h',
    OUTFILE = 'o',
    VERBOSE = 'v',

    EPSILON_N = 1000,
    EPSILON_B,
    LAMBDA,
    BETA,
    ALPHA,
    AGE_MAX,
    MAX_NODES,
    MIN_DANGLE,
    MAX_ANGLE,
    ANGLE_MERGE_EDGES,

    DUMP_TRIANGLES,

    NN_GUG,
    NN_VPTREE,
    NN_LINEAR,
    VPTREE_MAX_SIZE,
    GUG_MAX_DENS,
    GUG_EXPAND_RATE
} options_enum;

static struct option options[] = {
    { "help", no_argument, NULL, HELP },

    { "epsilon-n", required_argument, NULL, EPSILON_N },
    { "epsilon-b", required_argument, NULL, EPSILON_B },
    { "lambda",    required_argument, NULL, LAMBDA },
    { "beta",      required_argument, NULL, BETA },
    { "alpha",     required_argument, NULL, ALPHA },
    { "age-max",   required_argument, NULL, AGE_MAX },
    { "max-nodes", required_argument, NULL, MAX_NODES },
    { "min-dangle", required_argument, NULL, MIN_DANGLE },
    { "max-angle", required_argument, NULL, MAX_ANGLE },
    { "angle-merge-edges", required_argument, NULL, ANGLE_MERGE_EDGES },

    { "dump-triangles", required_argument, NULL, DUMP_TRIANGLES },

    { "outfile", required_argument, NULL, OUTFILE },

    { "nn-gug",    no_argument, NULL, NN_GUG },
    { "nn-vptree", no_argument, NULL, NN_VPTREE },
    { "nn-linear", no_argument, NULL, NN_LINEAR },
    { "vptree-max-size", required_argument, NULL, VPTREE_MAX_SIZE },
    { "gug-max-dens", required_argument, NULL, GUG_MAX_DENS },
    { "gug-expand-rate", required_argument, NULL, GUG_EXPAND_RATE },

    { NULL, 0, NULL, 0}
};

#define DUMP_TRIANGLES_FN_LEN 100
static fer_gsrm_params_t params;
static fer_gsrm_t *gsrm;
static const char *is_fn = NULL;
static const char *outfile_fn;
static FILE *dump_triangles = NULL;
static char dump_triangles_fn[DUMP_TRIANGLES_FN_LEN + 1] = "";

static void usage(int argc, char *argv[], const char *opt_msg);
static void readOptions(int argc, char *argv[]);
static void printAttrs(void);

int main(int argc, char *argv[])
{
    fer_mesh3_t *mesh;
    size_t islen;
    FILE *outfile;
    fer_timer_t timer;

    readOptions(argc, argv);

    gsrm = ferGSRMNew(&params);

    printAttrs();

    // open output file
    if (outfile_fn == NULL){
        outfile = stdout;
    }else{
        outfile = fopen(outfile_fn, "w");
        if (outfile == NULL){
            fprintf(stderr, "Can't open '%s' for writing!\n", outfile_fn);
            return -1;
        }
    }

    ferTimerStart(&timer);
    ferTimerStopAndPrintElapsed(&timer, stderr, " Reading input signals:\n");
    ferTimerStopAndPrintElapsed(&timer, stderr, "   -- '%s'...\n", is_fn);
    islen = ferGSRMAddInputSignals(gsrm, is_fn);
    ferTimerStopAndPrintElapsed(&timer, stderr, "     --  Added %d input signals.\n", islen);
    fprintf(stderr, "\n");

    if (ferGSRMRun(gsrm) == 0){
        ferGSRMPostprocess(gsrm);

        ferTimerStart(&timer);

        mesh = ferGSRMMesh(gsrm);
        ferMesh3DumpSVT(mesh, outfile, "Result");

        if (params.verbosity >= 2){
            fprintf(stderr, "\n");
            ferTimerStopAndPrintElapsed(&timer, stderr, " Mesh dumped to '%s'.\n",
                                        (outfile == stdout ? "stdout" : outfile_fn));
        }

        if (dump_triangles != NULL){
            ferMesh3DumpTriangles(mesh, dump_triangles);
            fclose(dump_triangles);

            if (params.verbosity >= 2){
                ferTimerStopAndPrintElapsed(&timer, stderr,
                                            " Mesh dumped as triangles into '%s'.\n",
                                            dump_triangles_fn);
            }
        }
    }

    ferGSRMDel(gsrm);


    // close output file
    if (outfile != stdout)
        fclose(outfile);

    return 0;
}


void readOptions(int argc, char *argv[])
{
    int c, option_index;
    long iv;
    fer_real_t fv;

    ferGSRMParamsInit(&params);
    params.verbosity = 1;
    params.nn.gug.num_cells = 0;
    params.nn.gug.max_dens = 0.1;
    params.nn.gug.expand_rate = 1.5;

    while ((c = getopt_long(argc, argv, "hvo:", options, &option_index)) != -1){
        switch(c){
            case HELP:
                usage(argc, argv, NULL);
                break;
            case VERBOSE:
                params.verbosity += 1;
                break;
            case EPSILON_N:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "epsilon-n must be floating point "
                                      "number");
                params.en = fv;
                break;
            case EPSILON_B:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "epsilon-b must be floating point "
                                      "number");
                params.eb = fv;
                break;
            case LAMBDA:
                if (ferParseLong(optarg, optarg + strlen(optarg), &iv, NULL) != 0)
                    usage(argc, argv, "lambda must be int number");
                params.lambda = iv;
                break;
            case BETA:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "beta must be floating point number");
                params.beta = fv;
                break;
            case ALPHA:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "alpha must be floating point number");
                params.alpha = fv;
                break;
            case AGE_MAX:
                if (ferParseLong(optarg, optarg + strlen(optarg), &iv, NULL) != 0)
                    usage(argc, argv, "age-max must be int number");
                params.age_max = iv;
                break;
            case MAX_NODES:
                if (ferParseLong(optarg, optarg + strlen(optarg), &iv, NULL) != 0)
                    usage(argc, argv, "max-nodes must be int number");
                params.max_nodes = iv;
                break;

            case MIN_DANGLE:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "min-dangle must be float");
                params.min_dangle = fv;
                break;
            case MAX_ANGLE:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "max-angle must be float");
                params.max_angle = fv;
                break;
            case ANGLE_MERGE_EDGES:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "angle-merge-edges must be float");
                params.angle_merge_edges = fv;
                break;

            case DUMP_TRIANGLES:
                    dump_triangles = fopen(optarg, "w");
                    if (dump_triangles == NULL)
                        usage(argc, argv, "can't open file for dump-triangles");
                    strncpy(dump_triangles_fn, optarg, DUMP_TRIANGLES_FN_LEN);
                    break;

            case NN_GUG:
                params.nn.type = FER_NN_GUG;
                break;
            case NN_VPTREE:
                params.nn.type = FER_NN_VPTREE;
                break;
            case NN_LINEAR:
                params.nn.type = FER_NN_LINEAR;
                break;
            case VPTREE_MAX_SIZE:
                if (ferParseLong(optarg, optarg + strlen(optarg), &iv, NULL) != 0)
                    usage(argc, argv, "vptree-max-size be int number");
                params.nn.vptree.maxsize = iv;
                break;
            case GUG_MAX_DENS:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "gug-max-dens must be float");
                params.nn.gug.max_dens = fv;
                break;
            case GUG_EXPAND_RATE:
                if (ferParseReal(optarg, optarg + strlen(optarg), &fv, NULL) != 0)
                    usage(argc, argv, "gug-expand-rate must be float");
                params.nn.gug.expand_rate = fv;
                break;

            case OUTFILE:
                if (strcmp(optarg, "stdout") == 0){
                    outfile_fn = NULL;
                }else{
                    outfile_fn = optarg;
                }
                break;

            default:
                usage(argc, argv, "");
        }
    }
    if (argc - optind != 1){
        usage(argc, argv, "filename must be specified");
    }
    is_fn = argv[argc - 1];
}

static void usage(int argc, char *argv[], const char *opt_msg)
{
    if (opt_msg != NULL){
        fprintf(stderr, "%s\n", opt_msg);
    }

    fprintf(stderr, "\n");
    fprintf(stderr, "Usage %s [ options ] filename\n", argv[0]);
    fprintf(stderr, "   Options: --epsilon-b float  Winner learning rate\n");
    fprintf(stderr, "            --epsilon-n float  Winner's neighbors learning rate\n");
    fprintf(stderr, "            --lambda    int    Steps in cycle\n");
    fprintf(stderr, "            --beta      float  Error counter decreasing rate\n");
    fprintf(stderr, "            --alpha     float  Error counter decreasing rate\n");
    fprintf(stderr, "            --age-max   int\n");
    fprintf(stderr, "            --max-nodes int    Stop Criterium\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "            --min-dangle        float  Minimal dihedral angle between faces\n");
    fprintf(stderr, "            --max-angle         float  Maximal angle in cusp of face\n");
    fprintf(stderr, "            --angle-merge-edges float  Minimal angle between edges to merge them\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "            --nn-gug                  Use Growing Uniform Grid for NN search (default choise)\n");
    fprintf(stderr, "            --nn-vptree               Use VP-Tree for NN search\n");
    fprintf(stderr, "            --nn-linear               Use linear NN search\n");
    fprintf(stderr, "            --vptree-max-size  int    Maximal number of elements in leaf node\n");
    fprintf(stderr, "            --gug-max-dens     float  Maximal density\n");
    fprintf(stderr, "            --gug-expand-rate  float  Expand rate\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "\n");

    fprintf(stderr, "            --outfile / -o   filename Filename where will be dumped resulting mesh (stdout is default)\n");
    fprintf(stderr, "            --dump-triangles filename Filename where will be stored triangles from reconstructed object.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "            -v / -vv / ...  Increases verbosity\n");
    fprintf(stderr, "\n");

    exit(-1);
}

void printAttrs(void)
{
    const fer_gsrm_params_t *param;

    param = ferGSRMParams(gsrm);

    fprintf(stderr, "Attributes:\n");
    fprintf(stderr, "    lambda:    %d\n", (int)param->lambda);
    fprintf(stderr, "    eb:        %f\n", (float)param->eb);
    fprintf(stderr, "    en:        %f\n", (float)param->en);
    fprintf(stderr, "    alpha      %f\n", (float)param->alpha);
    fprintf(stderr, "    beta:      %f\n", (float)param->beta);
    fprintf(stderr, "    age_max:   %d\n", (int)param->age_max);
    fprintf(stderr, "    max nodes: %d\n", (int)param->max_nodes);
    fprintf(stderr, "\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "    min d. angle:  %f\n", (float)param->min_dangle);
    fprintf(stderr, "    max angle:     %f\n", (float)param->max_angle);
    fprintf(stderr, "    ang. merge e.: %f\n", (float)param->angle_merge_edges);
    fprintf(stderr, "\n");
    fprintf(stderr, "    input signals: %s\n", is_fn);
    fprintf(stderr, "\n");
    fprintf(stderr, "    outfile: %s\n", (outfile_fn == NULL ? "stdout" : outfile_fn));
    fprintf(stderr, "\n");
    fprintf(stderr, "VP-Tree:\n");
    fprintf(stderr, "    maxsize: %d\n", (int)param->nn.vptree.maxsize);
    fprintf(stderr, "GUG:\n");
    fprintf(stderr, "    num cells:   %d\n", (int)param->nn.gug.num_cells);
    fprintf(stderr, "    max dens:    %f\n", (float)param->nn.gug.max_dens);
    fprintf(stderr, "    expand rate: %f\n", (float)param->nn.gug.expand_rate);
    fprintf(stderr, "\n");
}

