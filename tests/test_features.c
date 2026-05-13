#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/cpuinfo.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    tests_run++; \
    printf("  TEST: %s... ", #name); \
    name(); \
    tests_passed++; \
    printf("PASS\n"); \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL at %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        exit(1); \
    } \
} while(0)

#define ASSERT_EQ_INT(a, b) do { \
    int _a = (a), _b = (b); \
    if (_a != _b) { \
        printf("FAIL at %s:%d: %d != %d\n", __FILE__, __LINE__, _a, _b); \
        exit(1); \
    } \
} while(0)

#define ASSERT_STR_EQ(a, b) do { \
    if (strcmp((a), (b)) != 0) { \
        printf("FAIL at %s:%d: \"%s\" != \"%s\"\n", __FILE__, __LINE__, (a), (b)); \
        exit(1); \
    } \
} while(0)

static void fill_avx512_raw(struct cpu_raw_data_t* raw)
{
    memset(raw, 0, sizeof(*raw));

    /* Intel Skylake-X with AVX-512 */
    raw->basic[0][REG_EAX] = 0x00000016;
    raw->basic[0][REG_EBX] = 0x756E6547; /* "Genu" */
    raw->basic[0][REG_EDX] = 0x49656E69; /* "ineI" */
    raw->basic[0][REG_ECX] = 0x6C65746E; /* "ntel" */

    /* Leaf 1: family 6, model 0x55, stepping 4 */
    raw->basic[1][REG_EAX] = 0x00050654;
    raw->basic[1][REG_EBX] = 0x00100800;
    raw->basic[1][REG_ECX] = 0x7FFEFBFF;
    raw->basic[1][REG_EDX] = 0xBFEBFBFF;

    /* Leaf 7 EBX: FSGSBASE(0), SGX(2), BMI1(3), HLE(4), AVX2(5), BMI2(8), ERMS(9),
       INVPCID(10), RTM(11), AVX512F(16), AVX512DQ(17), RDSEED(18), ADX(19),
       AVX512CD(28), SHA(29), AVX512BW(30), AVX512VL(31) */
    raw->basic[7][REG_EBX] = 0xF00F0F3D;
    /* Leaf 7 ECX: AVX512VBMI(1), AVX512VBMI2(6), VAES(8), VPCLMULQDQ(10),
       AVX512VNNI(11), AVX512BITALG(12) */
    raw->basic[7][REG_ECX] = 0x00001D42;

    /* Intel cache leaf for core count */
    raw->intel_cache[0][REG_EAX] = (5u << 26) | 0x0001;

    /* Extended */
    raw->ext[0][REG_EAX] = 0x80000008;
    raw->ext[1][REG_ECX] = 0x00000001;
    raw->ext[1][REG_EDX] = 0x2C100800;

    /* Brand string */
    const char brand[] = "Intel Xeon W-2195 CPU @ 2.30GHz\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    memcpy(&raw->ext[2][REG_EAX], brand + 0, 16);
    memcpy(&raw->ext[3][REG_EAX], brand + 16, 16);
    memcpy(&raw->ext[4][REG_EAX], brand + 32, 16);
}

static void fill_amd_features_raw(struct cpu_raw_data_t* raw)
{
    memset(raw, 0, sizeof(*raw));

    /* AMD Zen3 */
    raw->basic[0][REG_EAX] = 0x00000010;
    raw->basic[0][REG_EBX] = 0x68747541;
    raw->basic[0][REG_EDX] = 0x69746E65;
    raw->basic[0][REG_ECX] = 0x444D4163;

    raw->basic[1][REG_EAX] = 0x00A50F00;
    raw->basic[1][REG_EBX] = 0x00100800;
    raw->basic[1][REG_ECX] = 0x7ED8320B;
    raw->basic[1][REG_EDX] = 0x178BFBFF;

    raw->basic[7][REG_EBX] = 0x219C97A9;
    raw->basic[7][REG_ECX] = 0x00000000;

    raw->ext[0][REG_EAX] = 0x80000023;
    /* ext leaf 1 ECX: LAHF_LM(0), CMP_LEGACY(1), SVM(2), ABM(3),
       SSE4A(6), MISALIGNSSE(7), 3DNOW_PREFETCH(8), OSVW(9), IBS(10),
       XOP(11), SKINIT(12), WDT(13), FMA4(16), TBM(21) */
    raw->ext[1][REG_ECX] = 0x75C237FF;
    raw->ext[1][REG_EDX] = 0x2FD3FBFF;

    raw->ext[7][REG_EDX] = 0x00006799;
    raw->ext[8][REG_ECX] = 0x0000000F;

    const char brand[] = "AMD Ryzen 9 5950X 16-Core Processor\0\0\0\0\0\0\0\0\0\0\0\0\0";
    memcpy(&raw->ext[2][REG_EAX], brand + 0, 16);
    memcpy(&raw->ext[3][REG_EAX], brand + 16, 16);
    memcpy(&raw->ext[4][REG_EAX], brand + 32, 16);
}

static void test_avx512_features(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;

    fill_avx512_raw(&raw);
    int ret = cpuinfo_identify(&raw, &id);

    ASSERT_EQ_INT(ret, ERR_OK);
    ASSERT(id.flags[FEAT_AVX512F]);
    ASSERT(id.flags[FEAT_AVX512DQ]);
    ASSERT(id.flags[FEAT_AVX512CD]);
    ASSERT(id.flags[FEAT_AVX512BW]);
    ASSERT(id.flags[FEAT_AVX512VL]);
    ASSERT(id.flags[FEAT_AVX512VBMI]);
    ASSERT(id.flags[FEAT_AVX512VBMI2]);
    ASSERT(id.flags[FEAT_AVX512VNNI]);
    ASSERT(id.flags[FEAT_AVX512BITALG]);
    ASSERT(id.flags[FEAT_FSGSBASE]);
    ASSERT(id.flags[FEAT_ERMS]);
    ASSERT(id.flags[FEAT_INVPCID]);
    ASSERT(id.flags[FEAT_VAES]);
    ASSERT(id.flags[FEAT_VPCLMULQDQ]);
    ASSERT(id.flags[FEAT_SGX]);
    ASSERT(id.flags[FEAT_SHA]);
    ASSERT(id.flags[FEAT_RDSEED]);
    ASSERT(id.flags[FEAT_ADX]);
    ASSERT(id.flags[FEAT_BMI1]);
    ASSERT(id.flags[FEAT_BMI2]);
    ASSERT(id.flags[FEAT_AVX2]);
    ASSERT(id.flags[FEAT_HLE]);
    ASSERT(id.flags[FEAT_RTM]);
}

static void test_amd_extended_features(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;

    fill_amd_features_raw(&raw);
    /* Set XOP(11), FMA4(16), TBM(21) bits for testing */
    raw.ext[1][REG_ECX] |= (1u << 11) | (1u << 16) | (1u << 21);

    int ret = cpuinfo_identify(&raw, &id);

    ASSERT_EQ_INT(ret, ERR_OK);
    ASSERT_EQ_INT(id.vendor, VENDOR_AMD);

    ASSERT(id.flags[FEAT_LAHF_LM]);
    ASSERT(id.flags[FEAT_CMP_LEGACY]);
    ASSERT(id.flags[FEAT_SVM]);
    ASSERT(id.flags[FEAT_ABM]);
    ASSERT(id.flags[FEAT_SSE4A]);
    ASSERT(id.flags[FEAT_MISALIGNSSE]);
    ASSERT(id.flags[FEAT_3DNOW_PREFETCH]);
    ASSERT(id.flags[FEAT_OSVW]);
    ASSERT(id.flags[FEAT_IBS]);
    ASSERT(id.flags[FEAT_XOP]);
    ASSERT(id.flags[FEAT_SKINIT]);
    ASSERT(id.flags[FEAT_WDT]);
    ASSERT(id.flags[FEAT_FMA4]);
    ASSERT(id.flags[FEAT_TBM]);

    ASSERT(id.flags[FEAT_CONSTANT_TSC]);
    ASSERT(id.flags[FEAT_CPB]);
    ASSERT(id.flags[FEAT_APERFMPERF]);
}

static void test_feature_name_completeness(void)
{
    for (int i = 0; i < NUM_FEATURES; i++) {
        const char* name = cpuinfo_feature_str((cpu_feature_t)i);
        ASSERT(name != NULL);
        ASSERT(strlen(name) > 0);
    }
}

static void test_feature_name_values(void)
{
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_FSGSBASE), "fsgsbase");
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_ERMS), "erms");
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_INVPCID), "invpcid");
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_VAES), "vaes");
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_VPCLMULQDQ), "vpclmulqdq");
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_AVX512BITALG), "avx512bitalg");
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_AVX512VNNI), "avx512vnni");
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_AVX512VBMI), "avx512vbmi");
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_AVX512VBMI2), "avx512vbmi2");
}

static void test_no_features_on_minimal_cpu(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;
    memset(&raw, 0, sizeof(raw));

    /* Minimal valid CPU: just vendor string, leaf count = 1 */
    raw.basic[0][REG_EAX] = 0x00000001;
    raw.basic[0][REG_EBX] = 0x756E6547;
    raw.basic[0][REG_EDX] = 0x49656E69;
    raw.basic[0][REG_ECX] = 0x6C65746E;
    raw.basic[1][REG_EAX] = 0x00000400; /* family 4, model 0 */
    raw.basic[1][REG_EDX] = 0x00000000; /* no features */
    raw.basic[1][REG_ECX] = 0x00000000;

    int ret = cpuinfo_identify(&raw, &id);
    ASSERT_EQ_INT(ret, ERR_OK);
    ASSERT(!id.flags[FEAT_SSE]);
    ASSERT(!id.flags[FEAT_SSE2]);
    ASSERT(!id.flags[FEAT_AVX]);
    ASSERT(!id.flags[FEAT_AVX512F]);
    ASSERT(!id.flags[FEAT_FSGSBASE]);
}

static void test_intel_amd_feature_compatibility(void)
{
    struct cpu_raw_data_t raw_intel, raw_amd;
    struct cpu_id_t id_intel, id_amd;

    fill_avx512_raw(&raw_intel);
    fill_amd_features_raw(&raw_amd);

    cpuinfo_identify(&raw_intel, &id_intel);
    cpuinfo_identify(&raw_amd, &id_amd);

    ASSERT_EQ_INT(id_intel.vendor, VENDOR_INTEL);
    ASSERT_EQ_INT(id_amd.vendor, VENDOR_AMD);

    /* Both should detect SSE/SSE2/SSE3 */
    ASSERT(id_intel.flags[FEAT_SSE]);
    ASSERT(id_intel.flags[FEAT_SSE2]);
    ASSERT(id_amd.flags[FEAT_SSE]);
    ASSERT(id_amd.flags[FEAT_SSE2]);

    /* AVX512 only on Intel */
    ASSERT(id_intel.flags[FEAT_AVX512F]);
    ASSERT(!id_amd.flags[FEAT_AVX512F]);

    /* AMD-specific features only on AMD */
    ASSERT(id_amd.flags[FEAT_SVM]);
    ASSERT(!id_intel.flags[FEAT_SVM]);
}

int main(void)
{
    printf("Running feature detection tests...\n");

    TEST(test_avx512_features);
    TEST(test_amd_extended_features);
    TEST(test_feature_name_completeness);
    TEST(test_feature_name_values);
    TEST(test_no_features_on_minimal_cpu);
    TEST(test_intel_amd_feature_compatibility);

    printf("\n%d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
