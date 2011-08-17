#include <cu/cu.h>
#include <fermat/vptree.h>
#include <fermat/rand-mt.h>
#include <fermat/dbg.h>

struct _el_t {
    fer_vec2_t w;
    fer_vptree_el_t el;
    fer_list_t list;
};
typedef struct _el_t el_t;

TEST(vptreeBuild)
{
    fer_rand_mt_t *rand;
    fer_vptree_t *vp;
    fer_vptree_params_t params;
    static fer_list_t els_list;
    static int els_len = 20;
    static el_t els[20];
    int i;

    rand = ferRandMTNewAuto();

    ferListInit(&els_list);
    for (i = 0; i < els_len; i++){
        ferVec2Set(&els[i].w, ferRandMT(rand, 0, 3), ferRandMT(rand, 0, 3));
        ferVPTreeElInit(&els[i].el, (const fer_vec_t *)&els[i].w);
        ferListAppend(&els_list, &els[i].list);
    }

    ferVPTreeParamsInit(&params);
    params.dim = 2;
    params.maxsize = 2;
    vp = ferVPTreeBuild(&params, &els[0].el, els_len, sizeof(el_t));

    ferVPTreeDump(vp, stdout);

    ferVPTreeDel(vp);

    ferRandMTDel(rand);
}
