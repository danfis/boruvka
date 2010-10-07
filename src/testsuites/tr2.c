#include <stdio.h>
#include "cu.h"
#include <mg/tr2.h>

static mg_tr2_t *tr;
static mg_vec2_t *v, *dest;

TEST(tr2SetUp)
{
    tr = mgTr2New();
    v = mgVec2New(0., 0.);
    dest = mgVec2New(0., 0.);
}

TEST(tr2TearDown)
{
    mgTr2Del(tr);
    mgVec2Del(v);
    mgVec2Del(dest);
}

static void print_matrix(const mg_tr2_t *m, FILE *out, const char *title)
{
    if (title != NULL)
        fprintf(out, "%s :\n", title);

    fprintf(out, "| %6f %6f %6f |\n",
            mgTr2Get(m, 0, 0), mgTr2Get(m, 0, 1), mgTr2Get(m, 0, 2));
    fprintf(out, "| %6f %6f %6f |\n",
            mgTr2Get(m, 1, 0), mgTr2Get(m, 1, 1), mgTr2Get(m, 1, 2));
    fprintf(out, "| %6f %6f %6f |\n",
            mgTr2Get(m, 2, 0), mgTr2Get(m, 2, 1), mgTr2Get(m, 2, 2));
}

static void print_vector(const mg_vec2_t *v, FILE *out, const char *title)
{
    if (title != NULL)
        fprintf(out, "%s :\n", title);

    fprintf(out, "[ %6f %6f ]\n", mgVec2X(v), mgVec2Y(v));
}


TEST(tr2Translation)
{

    printf("\n1 ----------------------\n");

    mgTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    mgVec2Set(v, -1.2, 5.);
    mgTr2Translate(tr, v);
    print_matrix(tr, stdout, "Translated");
    mgTr2Translate(tr, v);
    print_matrix(tr, stdout, "Translated (next time)");
}

TEST(tr2Rotation)
{
    printf("\n2 ----------------------\n");

    mgTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    mgTr2Rotate(tr, M_PI_4);
    print_matrix(tr, stdout, "Rotated about origin");

    mgTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    mgVec2Set(v, 1., 1.);
    mgTr2RotateCenter(tr, M_PI_4, v);
    print_matrix(tr, stdout, "Rotated:");

    mgTr2(tr, v);
    assertTrue(mgVec2Eq2(v, 1., 1.));
    print_vector(v, stdout, "(1, 1) rotated about itself");
}

TEST(tr2TransformVector)
{
    printf("\n3 ----------------------\n");
    mgVec2Set(dest, 0., 0.);

    mgTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    mgVec2Set(v, 1., 1.);
    mgTr2Translate(tr, v);
    print_matrix(tr, stdout, "Translated");

    mgTr2(tr, dest);
    print_vector(dest, stdout, "Vector");


    printf("\n4 ----------------------\n");
    mgVec2Set(dest, 0., 0.);

    mgTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    mgVec2Set(v, 1., 0.);
    mgTr2Translate(tr, v);
    print_matrix(tr, stdout, "Translated");

    mgVec2Set(v, 1., -1.);
    mgTr2RotateCenter(tr, M_PI_2, v);
    print_matrix(tr, stdout, "Rotated");

    mgTr2(tr, dest);
    print_vector(dest, stdout, "Vector");

    printf("\n5 ----------------------\n");
    mgVec2Set(dest, 0., 0.);

    mgTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    mgVec2Set(v, 1., 1.);
    mgTr2Translate(tr, v);
    print_matrix(tr, stdout, "Translated");

    mgTr2Rotate(tr, M_PI_2);
    print_matrix(tr, stdout, "Rotated");

    mgTr2(tr, dest);
    print_vector(dest, stdout, "Vector");


    printf("\n6 ----------------------\n");
    mgVec2Set(dest, 0., 0.);

    mgTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    mgTr2Rotate(tr, M_PI_2);
    print_matrix(tr, stdout, "Rotated");

    mgVec2Set(v, 1., 1.);
    mgTr2Translate(tr, v);
    print_matrix(tr, stdout, "Translated");

    mgTr2(tr, dest);
    print_vector(dest, stdout, "Vector");
}

TEST(tr2Clone)
{
    mg_tr2_t *r;


    mgTr2Identity(tr);
    mgVec2Set(v, 1., 1.);
    mgTr2Translate(tr, v);

    mgVec2Set(dest, 0., 1.);
    mgTr2(tr, dest);
    assertTrue(mgVec2Eq2(dest, 1., 2.));

    r = mgTr2Clone(tr);
    mgVec2Set(dest, 0., 1.);
    mgTr2(r, dest);
    assertTrue(mgVec2Eq2(dest, 1., 2.));

    mgTr2Del(r);
}

TEST(tr2Copy)
{
    mg_tr2_t *tr1, *tr2;
    int i, j;

    tr1 = mgTr2New();
    tr2 = mgTr2New();

    mgTr2TranslateCoords(tr1, 10., 10.);
    mgTr2Copy(tr2, tr1);

    for (i=0; i < 3; i++){
        for (j=0; j < 3; j++){
            assertTrue(mgEq(mgTr2Get(tr1, i, j), mgTr2Get(tr2, i, j)));
        }
    }

    mgTr2Del(tr1);
    mgTr2Del(tr2);
}
