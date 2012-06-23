#include <cu/cu.h>
#include <boruvka/opts.h>

static void opts1_help(const char *long_name, char short_name)
{
    fprintf(stdout, "opts1 :: HELP `%s' '%c'\n", long_name, short_name);
}

TEST(opts1)
{
    bor_real_t opt1 = 0.;
    int help = -1;
    static int argc = 5;
    static char *argv[] = { "program", "--opt1", "11.1", "-h", "filename" };
    int i;

    ferOptsAdd("opt1", 'o', FER_OPTS_REAL, (void *)&opt1, NULL);
    ferOptsAdd(NULL, 'h', FER_OPTS_NONE, (void *)&help, FER_OPTS_CB(opts1_help));

    ferOpts(&argc, (char **)argv);

    assertTrue(ferEq(opt1, 11.1));
    assertEquals(help, 1);

    fprintf(stdout, "opts1 :: Remaining options:\n");
    for (i = 0; i < argc; i++){
        fprintf(stdout, "  [%02d] `%s'\n", i, argv[i]);
    }

    ferOptsClear();
}
