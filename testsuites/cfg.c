#include <cu/cu.h>
#include <fermat/cfg.h>
#include <fermat/dbg.h>

TEST(cfg1)
{
    fer_cfg_t *cfg;
    fer_real_t vf;
    const char *vs;
    fer_vec2_t v2;
    fer_vec3_t v3;
    char **vss;
    const fer_real_t *vfs;
    const fer_vec2_t *v2s;
    const fer_vec3_t *v3s;
    size_t len;

    cfg = ferCfgRead("cfg1.cfg");
    if (!cfg)
        return;

    assertFalse(ferCfgHaveParam(cfg, "asdf"));
    assertTrue(ferCfgHaveParam(cfg, "var1"));
    assertTrue(ferCfgHaveParam(cfg, "var2"));
    assertTrue(ferCfgHaveParam(cfg, "var3"));
    assertTrue(ferCfgHaveParam(cfg, "var_v2"));
    assertTrue(ferCfgHaveParam(cfg, "var_v3"));
    assertTrue(ferCfgHaveParam(cfg, "var4"));
    assertTrue(ferCfgHaveParam(cfg, "var5"));
    assertTrue(ferCfgHaveParam(cfg, "var_s"));
    assertTrue(ferCfgHaveParam(cfg, "var6"));
    assertTrue(ferCfgHaveParam(cfg, "var_v2_arr"));
    assertTrue(ferCfgHaveParam(cfg, "var_v3_arr"));

    assertFalse(ferCfgParamIsArr(cfg, "var1"));
    assertFalse(ferCfgParamIsArr(cfg, "var2"));
    assertFalse(ferCfgParamIsArr(cfg, "var3"));
    assertFalse(ferCfgParamIsArr(cfg, "var_v2"));
    assertFalse(ferCfgParamIsArr(cfg, "var_v3"));
    assertFalse(ferCfgParamIsArr(cfg, "var4"));
    assertFalse(ferCfgParamIsArr(cfg, "var5"));
    assertTrue(ferCfgParamIsArr(cfg, "var_s"));
    assertTrue(ferCfgParamIsArr(cfg, "var6"));
    assertTrue(ferCfgParamIsArr(cfg, "var_v2_arr"));
    assertTrue(ferCfgParamIsArr(cfg, "var_v3_arr"));

    assertTrue(ferCfgParamIsFlt(cfg, "var1"));
    assertTrue(ferCfgParamIsFlt(cfg, "var2"));
    assertTrue(ferCfgParamIsStr(cfg, "var3"));
    assertTrue(ferCfgParamIsV2(cfg, "var_v2"));
    assertTrue(ferCfgParamIsV3(cfg, "var_v3"));
    assertTrue(ferCfgParamIsFlt(cfg, "var4"));
    assertTrue(ferCfgParamIsStr(cfg, "var5"));
    assertTrue(ferCfgParamIsStr(cfg, "var_s"));
    assertTrue(ferCfgParamIsFlt(cfg, "var6"));
    assertTrue(ferCfgParamIsV2(cfg, "var_v2_arr"));
    assertTrue(ferCfgParamIsV3(cfg, "var_v3_arr"));


    assertEquals(ferCfgParamFlt(cfg, "var1", &vf), 0);
    assertTrue(ferEq(vf, 10.));
    assertEquals(ferCfgParamFlt(cfg, "var2", &vf), 0);
    assertTrue(ferEq(vf, 12.));
    assertNotEquals(ferCfgParamFlt(cfg, "var3", &vf), 0);
    assertEquals(ferCfgParamStr(cfg, "var3", &vs), 0);
    assertEquals(strcmp(vs, "ahoj"), 0);
    assertEquals(ferCfgParamV2(cfg, "var_v2", &v2), 0);
    assertTrue(ferEq(ferVec2Get(&v2, 0), 1.));
    assertTrue(ferEq(ferVec2Get(&v2, 1), 2.));
    assertEquals(ferCfgParamV3(cfg, "var_v3", &v3), 0);
    assertTrue(ferEq(ferVec3Get(&v3, 0), 2.));
    assertTrue(ferEq(ferVec3Get(&v3, 1), 3.));
    assertTrue(ferEq(ferVec3Get(&v3, 2), 4.));
    assertEquals(ferCfgParamFlt(cfg, "var4", &vf), 0);
    assertTrue(ferEq(vf, 11.2e2));
    assertEquals(ferCfgParamStr(cfg, "var5", &vs), 0);
    assertEquals(strcmp(vs, "aa"), 0);

    assertEquals(ferCfgParamStrArr(cfg, "var_s", &vss, &len), 0);
    assertEquals(len, 3);
    assertEquals(strcmp(vss[0], "1"), 0);
    assertEquals(strcmp(vss[1], "2"), 0);
    assertEquals(strcmp(vss[2], "3"), 0);

    assertEquals(ferCfgParamFltArr(cfg, "var6", &vfs, &len), 0);
    assertEquals(len, 2);
    assertTrue(ferEq(vfs[0], 1));
    assertTrue(ferEq(vfs[1], 2));

    assertEquals(ferCfgParamV2Arr(cfg, "var_v2_arr", &v2s, &len), 0);
    assertEquals(len, 3);
    assertTrue(ferEq(ferVec2Get(&v2s[0], 0), 1));
    assertTrue(ferEq(ferVec2Get(&v2s[0], 1), 2));
    assertTrue(ferEq(ferVec2Get(&v2s[1], 0), 3));
    assertTrue(ferEq(ferVec2Get(&v2s[1], 1), 4));
    assertTrue(ferEq(ferVec2Get(&v2s[2], 0), 5));
    assertTrue(ferEq(ferVec2Get(&v2s[2], 1), 6));

    assertEquals(ferCfgParamV3Arr(cfg, "var_v3_arr", &v3s, &len), 0);
    assertEquals(len, 4);
    assertTrue(ferEq(ferVec3Get(&v3s[0], 0), 1));
    assertTrue(ferEq(ferVec3Get(&v3s[0], 1), 2));
    assertTrue(ferEq(ferVec3Get(&v3s[0], 2), 3));
    assertTrue(ferEq(ferVec3Get(&v3s[1], 0), 4));
    assertTrue(ferEq(ferVec3Get(&v3s[1], 1), 5));
    assertTrue(ferEq(ferVec3Get(&v3s[1], 2), 6));
    assertTrue(ferEq(ferVec3Get(&v3s[2], 0), 0));
    assertTrue(ferEq(ferVec3Get(&v3s[2], 1), 8));
    assertTrue(ferEq(ferVec3Get(&v3s[2], 2), 1));
    assertTrue(ferEq(ferVec3Get(&v3s[3], 0), 2));
    assertTrue(ferEq(ferVec3Get(&v3s[3], 1), 0));
    assertTrue(ferEq(ferVec3Get(&v3s[3], 2), 3));

    ferCfgDel(cfg);
}


struct _cfg1_format_t {
    fer_real_t var1, var2;
    fer_vec3_t v3;
    fer_real_t *var6;
    size_t var6len;
};
typedef struct _cfg1_format_t cfg1_format_t;

TEST(cfg1format)
{
    fer_cfg_t *cfg;
    cfg1_format_t f;
    size_t i;

    cfg = ferCfgRead("cfg1.cfg");
    if (!cfg)
        return;

    ferCfgScan(cfg, "var1:f var2:f  var_v3:v3 var6:f[] var6:f#",
               &f.var1, &f.var2, &f.v3, &f.var6, &f.var6len);
    assertTrue(ferEq(f.var1, 10));
    assertTrue(ferEq(f.var2, 12));
    assertTrue(ferEq(ferVec3Get(&f.v3, 0), 2));
    assertTrue(ferEq(ferVec3Get(&f.v3, 1), 3));
    assertTrue(ferEq(ferVec3Get(&f.v3, 2), 4));
    assertEquals(f.var6len, 2);
    assertTrue(ferEq(f.var6[0], 1));
    assertTrue(ferEq(f.var6[1], 2));

    ferCfgDel(cfg);
}
