#include <stdio.h>
#include "cu.h"
#include <fermat/tr2.h>

static fer_tr2_t *tr;
static fer_vec2_t *v, *dest;

TEST(tr2SetUp)
{
    tr = ferTr2New();
    v = ferVec2New(0., 0.);
    dest = ferVec2New(0., 0.);
}

TEST(tr2TearDown)
{
    ferTr2Del(tr);
    ferVec2Del(v);
    ferVec2Del(dest);
}

static void print_matrix(const fer_tr2_t *m, FILE *out, const char *title)
{
    if (title != NULL)
        fprintf(out, "%s :\n", title);

    fprintf(out, "| %6f %6f %6f |\n",
            ferTr2Get(m, 0, 0), ferTr2Get(m, 0, 1), ferTr2Get(m, 0, 2));
    fprintf(out, "| %6f %6f %6f |\n",
            ferTr2Get(m, 1, 0), ferTr2Get(m, 1, 1), ferTr2Get(m, 1, 2));
    fprintf(out, "| %6f %6f %6f |\n",
            ferTr2Get(m, 2, 0), ferTr2Get(m, 2, 1), ferTr2Get(m, 2, 2));
}

static void print_vector(const fer_vec2_t *v, FILE *out, const char *title)
{
    if (title != NULL)
        fprintf(out, "%s :\n", title);

    fprintf(out, "[ %6f %6f ]\n", ferVec2X(v), ferVec2Y(v));
}


TEST(tr2Translation)
{

    printf("\n1 ----------------------\n");

    ferTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    ferVec2Set(v, -1.2, 5.);
    ferTr2Translate(tr, v);
    print_matrix(tr, stdout, "Translated");
    ferTr2Translate(tr, v);
    print_matrix(tr, stdout, "Translated (next time)");
}

TEST(tr2Rotation)
{
    printf("\n2 ----------------------\n");

    ferTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    ferTr2Rotate(tr, M_PI_4);
    print_matrix(tr, stdout, "Rotated about origin");

    ferTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    ferVec2Set(v, 1., 1.);
    ferTr2RotateCenter(tr, M_PI_4, v);
    print_matrix(tr, stdout, "Rotated:");

    ferTr2(tr, v);
    assertTrue(ferVec2Eq2(v, 1., 1.));
    print_vector(v, stdout, "(1, 1) rotated about itself");
}

TEST(tr2TransformVector)
{
    printf("\n3 ----------------------\n");
    ferVec2Set(dest, 0., 0.);

    ferTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    ferVec2Set(v, 1., 1.);
    ferTr2Translate(tr, v);
    print_matrix(tr, stdout, "Translated");

    ferTr2(tr, dest);
    print_vector(dest, stdout, "Vector");


    printf("\n4 ----------------------\n");
    ferVec2Set(dest, 0., 0.);

    ferTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    ferVec2Set(v, 1., 0.);
    ferTr2Translate(tr, v);
    print_matrix(tr, stdout, "Translated");

    ferVec2Set(v, 1., -1.);
    ferTr2RotateCenter(tr, M_PI_2, v);
    print_matrix(tr, stdout, "Rotated");

    ferTr2(tr, dest);
    print_vector(dest, stdout, "Vector");

    printf("\n5 ----------------------\n");
    ferVec2Set(dest, 0., 0.);

    ferTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    ferVec2Set(v, 1., 1.);
    ferTr2Translate(tr, v);
    print_matrix(tr, stdout, "Translated");

    ferTr2Rotate(tr, M_PI_2);
    print_matrix(tr, stdout, "Rotated");

    ferTr2(tr, dest);
    print_vector(dest, stdout, "Vector");


    printf("\n6 ----------------------\n");
    ferVec2Set(dest, 0., 0.);

    ferTr2Identity(tr);
    print_matrix(tr, stdout, "Identity");

    ferTr2Rotate(tr, M_PI_2);
    print_matrix(tr, stdout, "Rotated");

    ferVec2Set(v, 1., 1.);
    ferTr2Translate(tr, v);
    print_matrix(tr, stdout, "Translated");

    ferTr2(tr, dest);
    print_vector(dest, stdout, "Vector");
}

TEST(tr2Clone)
{
    fer_tr2_t *r;


    ferTr2Identity(tr);
    ferVec2Set(v, 1., 1.);
    ferTr2Translate(tr, v);

    ferVec2Set(dest, 0., 1.);
    ferTr2(tr, dest);
    assertTrue(ferVec2Eq2(dest, 1., 2.));

    r = ferTr2Clone(tr);
    ferVec2Set(dest, 0., 1.);
    ferTr2(r, dest);
    assertTrue(ferVec2Eq2(dest, 1., 2.));

    ferTr2Del(r);
}

TEST(tr2Copy)
{
    fer_tr2_t *tr1, *tr2;
    int i, j;

    tr1 = ferTr2New();
    tr2 = ferTr2New();

    ferTr2TranslateCoords(tr1, 10., 10.);
    ferTr2Copy(tr2, tr1);

    for (i=0; i < 3; i++){
        for (j=0; j < 3; j++){
            assertTrue(ferEq(ferTr2Get(tr1, i, j), ferTr2Get(tr2, i, j)));
        }
    }

    ferTr2Del(tr1);
    ferTr2Del(tr2);
}
