#include <cu/cu.h>
#include <boruvka/cfg.h>
#include <boruvka/dbg.h>

TEST(cfg1)
{
    bor_cfg_t *cfg;
    bor_real_t vf;
    int vi;
    const char *vs;
    bor_vec2_t v2;
    bor_vec3_t v3;
    char **vss;
    const bor_real_t *vfs;
    const int *vis;
    const bor_vec2_t *v2s;
    const bor_vec3_t *v3s;
    size_t len;

    cfg = borCfgRead("cfg1.cfg");
    if (!cfg)
        return;

    assertFalse(borCfgHaveParam(cfg, "asdf"));
    assertTrue(borCfgHaveParam(cfg, "var1"));
    assertTrue(borCfgHaveParam(cfg, "var2"));
    assertTrue(borCfgHaveParam(cfg, "var3"));
    assertTrue(borCfgHaveParam(cfg, "var_v2"));
    assertTrue(borCfgHaveParam(cfg, "var_v3"));
    assertTrue(borCfgHaveParam(cfg, "var4"));
    assertTrue(borCfgHaveParam(cfg, "var5"));
    assertTrue(borCfgHaveParam(cfg, "var_s"));
    assertTrue(borCfgHaveParam(cfg, "var6"));
    assertTrue(borCfgHaveParam(cfg, "var_v2_arr"));
    assertTrue(borCfgHaveParam(cfg, "var_v3_arr"));
    assertTrue(borCfgHaveParam(cfg, "var_i"));
    assertTrue(borCfgHaveParam(cfg, "var_i_arr"));

    assertFalse(borCfgParamIsArr(cfg, "var1"));
    assertFalse(borCfgParamIsArr(cfg, "var2"));
    assertFalse(borCfgParamIsArr(cfg, "var3"));
    assertFalse(borCfgParamIsArr(cfg, "var_v2"));
    assertFalse(borCfgParamIsArr(cfg, "var_v3"));
    assertFalse(borCfgParamIsArr(cfg, "var4"));
    assertFalse(borCfgParamIsArr(cfg, "var5"));
    assertFalse(borCfgParamIsArr(cfg, "var_i"));
    assertTrue(borCfgParamIsArr(cfg, "var_s"));
    assertTrue(borCfgParamIsArr(cfg, "var6"));
    assertTrue(borCfgParamIsArr(cfg, "var_v2_arr"));
    assertTrue(borCfgParamIsArr(cfg, "var_v3_arr"));
    assertTrue(borCfgParamIsArr(cfg, "var_i_arr"));

    assertTrue(borCfgParamIsFlt(cfg, "var1"));
    assertTrue(borCfgParamIsFlt(cfg, "var2"));
    assertTrue(borCfgParamIsStr(cfg, "var3"));
    assertTrue(borCfgParamIsV2(cfg, "var_v2"));
    assertTrue(borCfgParamIsV3(cfg, "var_v3"));
    assertTrue(borCfgParamIsFlt(cfg, "var4"));
    assertTrue(borCfgParamIsStr(cfg, "var5"));
    assertTrue(borCfgParamIsStr(cfg, "var_s"));
    assertTrue(borCfgParamIsFlt(cfg, "var6"));
    assertTrue(borCfgParamIsV2(cfg, "var_v2_arr"));
    assertTrue(borCfgParamIsV3(cfg, "var_v3_arr"));
    assertTrue(borCfgParamIsInt(cfg, "var_i"));
    assertFalse(borCfgParamIsFlt(cfg, "var_i"));


    assertEquals(borCfgParamFlt(cfg, "var1", &vf), 0);
    assertTrue(borEq(vf, 10.));
    assertEquals(borCfgParamFlt(cfg, "var2", &vf), 0);
    assertTrue(borEq(vf, 12.));
    assertNotEquals(borCfgParamFlt(cfg, "var3", &vf), 0);
    assertEquals(borCfgParamStr(cfg, "var3", &vs), 0);
    assertEquals(strcmp(vs, "ahoj"), 0);
    assertEquals(borCfgParamV2(cfg, "var_v2", &v2), 0);
    assertTrue(borEq(borVec2Get(&v2, 0), 1.));
    assertTrue(borEq(borVec2Get(&v2, 1), 2.));
    assertEquals(borCfgParamV3(cfg, "var_v3", &v3), 0);
    assertTrue(borEq(borVec3Get(&v3, 0), 2.));
    assertTrue(borEq(borVec3Get(&v3, 1), 3.));
    assertTrue(borEq(borVec3Get(&v3, 2), 4.));
    assertEquals(borCfgParamFlt(cfg, "var4", &vf), 0);
    assertTrue(borEq(vf, 11.2e2));
    assertEquals(borCfgParamStr(cfg, "var5", &vs), 0);
    assertEquals(strcmp(vs, "aa"), 0);
    assertEquals(borCfgParamInt(cfg, "var_i", &vi), 0);
    assertEquals(vi, 10);
    assertNotEquals(borCfgParamFlt(cfg, "var_i", &vf), 0);

    assertEquals(borCfgParamStrArr(cfg, "var_s", &vss, &len), 0);
    assertEquals(len, 3);
    assertEquals(strcmp(vss[0], "1"), 0);
    assertEquals(strcmp(vss[1], "2"), 0);
    assertEquals(strcmp(vss[2], "3"), 0);

    assertEquals(borCfgParamFltArr(cfg, "var6", &vfs, &len), 0);
    assertEquals(len, 2);
    assertTrue(borEq(vfs[0], 1));
    assertTrue(borEq(vfs[1], 2));

    assertEquals(borCfgParamV2Arr(cfg, "var_v2_arr", &v2s, &len), 0);
    assertEquals(len, 3);
    assertTrue(borEq(borVec2Get(&v2s[0], 0), 1));
    assertTrue(borEq(borVec2Get(&v2s[0], 1), 2));
    assertTrue(borEq(borVec2Get(&v2s[1], 0), 3));
    assertTrue(borEq(borVec2Get(&v2s[1], 1), 4));
    assertTrue(borEq(borVec2Get(&v2s[2], 0), 5));
    assertTrue(borEq(borVec2Get(&v2s[2], 1), 6));

    assertEquals(borCfgParamV3Arr(cfg, "var_v3_arr", &v3s, &len), 0);
    assertEquals(len, 4);
    assertTrue(borEq(borVec3Get(&v3s[0], 0), 1));
    assertTrue(borEq(borVec3Get(&v3s[0], 1), 2));
    assertTrue(borEq(borVec3Get(&v3s[0], 2), 3));
    assertTrue(borEq(borVec3Get(&v3s[1], 0), 4));
    assertTrue(borEq(borVec3Get(&v3s[1], 1), 5));
    assertTrue(borEq(borVec3Get(&v3s[1], 2), 6));
    assertTrue(borEq(borVec3Get(&v3s[2], 0), 0));
    assertTrue(borEq(borVec3Get(&v3s[2], 1), 8));
    assertTrue(borEq(borVec3Get(&v3s[2], 2), 1));
    assertTrue(borEq(borVec3Get(&v3s[3], 0), 2));
    assertTrue(borEq(borVec3Get(&v3s[3], 1), 0));
    assertTrue(borEq(borVec3Get(&v3s[3], 2), 3));

    assertEquals(borCfgParamIntArr(cfg, "var_i_arr", &vis, &len), 0);
    assertEquals(len, 3);
    assertEquals(vis[0], 1);
    assertEquals(vis[1], 4);
    assertEquals(vis[2], 8);

    borCfgDel(cfg);
}


struct _cfg1_format_t {
    bor_real_t var1, var2;
    bor_vec3_t v3;
    bor_real_t *var6;
    size_t var6len;
    int i;
    const int *is;
    size_t ilen;
};
typedef struct _cfg1_format_t cfg1_format_t;

TEST(cfg1format)
{
    bor_cfg_t *cfg;
    cfg1_format_t f;
    size_t i;

    cfg = borCfgRead("cfg1.cfg");
    if (!cfg)
        return;

    borCfgScan(cfg, "var1:f var2:f  var_v3:v3 var6:f[] var6:f#",
               &f.var1, &f.var2, &f.v3, &f.var6, &f.var6len);
    assertTrue(borEq(f.var1, 10));
    assertTrue(borEq(f.var2, 12));
    assertTrue(borEq(borVec3Get(&f.v3, 0), 2));
    assertTrue(borEq(borVec3Get(&f.v3, 1), 3));
    assertTrue(borEq(borVec3Get(&f.v3, 2), 4));
    assertEquals(f.var6len, 2);
    assertTrue(borEq(f.var6[0], 1));
    assertTrue(borEq(f.var6[1], 2));

    borCfgScan(cfg, "var1:f var_i:i var2:f   var_i_arr:i[] var_i_arr:i# var_v3:v3 var6:f[] var6:f#",
               &f.var1, &f.i, &f.var2, &f.is, &f.ilen, &f.v3, &f.var6, &f.var6len);
    assertTrue(borEq(f.var1, 10));
    assertTrue(borEq(f.var2, 12));
    assertTrue(borEq(borVec3Get(&f.v3, 0), 2));
    assertTrue(borEq(borVec3Get(&f.v3, 1), 3));
    assertTrue(borEq(borVec3Get(&f.v3, 2), 4));
    assertEquals(f.var6len, 2);
    assertTrue(borEq(f.var6[0], 1));
    assertTrue(borEq(f.var6[1], 2));
    assertEquals(f.i, 10);
    assertEquals(f.ilen, 3);
    assertEquals(f.is[0], 1);
    assertEquals(f.is[1], 4);
    assertEquals(f.is[2], 8);

    borCfgDel(cfg);
}
