#include <fermat/vec3.h>

fer_vec3_t protein_small_center[] = {
    FER_VEC3_STATIC(-9.186000, -2.129000, -20.917000),
    FER_VEC3_STATIC(-9.528000, -3.348000, -20.291000),
    FER_VEC3_STATIC(-8.810000, -2.342000, -22.388000),
    FER_VEC3_STATIC(-7.303000, -2.563000, -22.594999),
    FER_VEC3_STATIC(-6.623000, -3.979000, -21.733999),
    FER_VEC3_STATIC(-9.210000, -0.896000, -23.370001),
    FER_VEC3_STATIC(-10.419000, -3.591000, -20.555000)
};

fer_real_t protein_small_radius[] = {
    1.700000,
    1.520000,
    1.700000,
    1.700000,
    1.700000,
    1.700000,
    1.200000
};

size_t protein_small_len = sizeof(protein_small_radius) / sizeof(fer_real_t);
