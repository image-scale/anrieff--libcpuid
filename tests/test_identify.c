#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
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

/* Helper: create raw data simulating an Intel Core i7 */
static void fill_intel_raw(struct cpu_raw_data_t* raw)
{
    memset(raw, 0, sizeof(*raw));

    /* Leaf 0: max basic leaf = 0x16, vendor = "GenuineIntel" */
    raw->basic[0][REG_EAX] = 0x00000016;
    raw->basic[0][REG_EBX] = 0x756E6547; /* "Genu" */
    raw->basic[0][REG_EDX] = 0x49656E69; /* "ineI" */
    raw->basic[0][REG_ECX] = 0x6C65746E; /* "ntel" */

    /* Leaf 1: family 6, model 158, stepping 9 (Coffee Lake)
       EAX = 0x000906EA: family=6, model_ext=9, model=E, stepping=A
       Actually let's use: family=6, ext_model=9, model=E, stepping=A => eax = 0x000906EA
       That gives ext_family=6, ext_model = (9<<4)|E = 0x9E
    */
    raw->basic[1][REG_EAX] = 0x000906EA;
    /* EBX: logical processors count in bits 23:16, e.g. 12 */
    raw->basic[1][REG_EBX] = 0x000C0800;
    /* ECX: SSE3, PCLMUL, SSSE3, FMA3(12), CX16(13), SSE4.1(19), SSE4.2(20),
       MOVBE(22), POPCNT(23), AES(25), XSAVE(26), OSXSAVE(27), AVX(28), F16C(29), RDRAND(30) */
    raw->basic[1][REG_ECX] = 0x7FFAFBBF;
    /* EDX: FPU(0), VME(1), DE(2), PSE(3), TSC(4), MSR(5), PAE(6), MCE(7),
       CX8(8), APIC(9), SEP(11), MTRR(12), PGE(13), MCA(14), CMOV(15),
       PAT(16), PSE36(17), CLFLUSH(19), MMX(23), FXSR(24), SSE(25), SSE2(26), HT(28) */
    raw->basic[1][REG_EDX] = 0x1F8BFBFF;

    /* Leaf 4 (intel cache): 6 cores => (5 << 26) | rest */
    raw->intel_cache[0][REG_EAX] = (5u << 26) | 0x0001;

    /* Leaf 7: AVX2(5), BMI1(3), BMI2(8), SHA(29), RDSEED(18), ADX(19) */
    raw->basic[7][REG_EBX] = 0x209C03A9;
    raw->basic[7][REG_ECX] = 0x00000000;

    /* Extended leaves */
    raw->ext[0][REG_EAX] = 0x80000008;
    /* ext leaf 1: LAHF_LM(0), LM(29 of EDX) */
    raw->ext[1][REG_ECX] = 0x00000001;
    raw->ext[1][REG_EDX] = 0x2C100800; /* syscall, rdtscp, lm */

    /* Brand string: "Intel(R) Core(TM) i7-8700K CPU @ 3.70GHz" */
    const char brand[] = "Intel(R) Core(TM) i7-8700K CPU @ 3.70GHz\0\0\0\0\0\0\0";
    memcpy(&raw->ext[2][REG_EAX], brand + 0, 16);
    memcpy(&raw->ext[3][REG_EAX], brand + 16, 16);
    memcpy(&raw->ext[4][REG_EAX], brand + 32, 16);

    /* 80000007: constant_tsc(8) */
    raw->ext[7][REG_EDX] = 0x00000100;

    /* 80000008: cores = 11 (12-1) in ECX[7:0] */
    raw->ext[8][REG_ECX] = 0x0000000B;
}

/* Helper: create raw data simulating an AMD Ryzen */
static void fill_amd_raw(struct cpu_raw_data_t* raw)
{
    memset(raw, 0, sizeof(*raw));

    /* Leaf 0: vendor = "AuthenticAMD" */
    raw->basic[0][REG_EAX] = 0x00000010;
    raw->basic[0][REG_EBX] = 0x68747541; /* "Auth" */
    raw->basic[0][REG_EDX] = 0x69746E65; /* "enti" */
    raw->basic[0][REG_ECX] = 0x444D4163; /* "cAMD" */

    /* Leaf 1: Family=0xF, ExtFamily=8, Model=1, ExtModel=0 => Zen
       family_ext = 0xF + 8 = 23, model_ext = (0 << 4) | 1 = 1
       Actually for Zen3: EAX = 0x00A50F00
       family=0xF, ext_family=0xA-0 = actually: bits 27:20 = 0x0A, bits 19:16=5
       Let's use 0x00A50F00: stepping=0, model=0, family=0xF, ext_model=5, ext_family=0xA
       ext_family = 0xF + 0xA = 25, ext_model = (5<<4)|0 = 0x50
       Hmm that doesn't match. Let me calculate properly:
       0x00A50F00:
       stepping = bits 3:0 = 0
       model = bits 7:4 = 0
       family = bits 11:8 = 0xF
       (unused) = bits 13:12
       ext_model = bits 19:16 = 5
       ext_family = bits 27:20 = 0x0A (=10)
       So: ext_family = 0xF + 10 = 25, ext_model = (5<<4)|0 = 0x50
    */
    raw->basic[1][REG_EAX] = 0x00A50F00;
    raw->basic[1][REG_EBX] = 0x00100800;
    /* ECX: SSE3(0), PCLMUL(1), MONITOR(3), SSSE3(9), FMA3(12), CX16(13),
       SSE4.1(19), SSE4.2(20), POPCNT(23), AES(25), XSAVE(26), OSXSAVE(27), AVX(28) */
    raw->basic[1][REG_ECX] = 0x7ED8320B;
    /* EDX: FPU, VME, DE, PSE, TSC, MSR, PAE, MCE, CX8, APIC, SEP, MTRR, PGE, MCA, CMOV, PAT, PSE36, CLFLUSH, MMX, FXSR, SSE, SSE2, HT */
    raw->basic[1][REG_EDX] = 0x178BFBFF;

    /* Leaf 7: AVX2, BMI1, BMI2, SHA, RDSEED, ADX */
    raw->basic[7][REG_EBX] = 0x219C97A9;
    raw->basic[7][REG_ECX] = 0x0040068C;

    /* Extended */
    raw->ext[0][REG_EAX] = 0x80000023;
    /* ext leaf 1: ECX */
    raw->ext[1][REG_ECX] = 0x75C237FF;
    raw->ext[1][REG_EDX] = 0x2FD3FBFF;

    /* Brand string: "AMD Ryzen 9 5900HS with Radeon Graphics" */
    const char brand[] = "AMD Ryzen 9 5900HS with Radeon Graphics\0\0\0\0\0\0\0\0";
    memcpy(&raw->ext[2][REG_EAX], brand + 0, 16);
    memcpy(&raw->ext[3][REG_EAX], brand + 16, 16);
    memcpy(&raw->ext[4][REG_EAX], brand + 32, 16);

    /* 80000007: constant_tsc(8), cpb(9), aperfmperf(7) */
    raw->ext[7][REG_EDX] = 0x00006799;

    /* 80000008: cores = 15 (16-1) */
    raw->ext[8][REG_ECX] = 0x0000400F;
}

static void test_version(void)
{
    const char* ver = cpuinfo_version();
    ASSERT(ver != NULL);
    ASSERT(strlen(ver) > 0);
    ASSERT_STR_EQ(ver, "0.1.0");
}

static void test_error_reporting(void)
{
    cpuinfo_set_error(ERR_OK);
    ASSERT_STR_EQ(cpuinfo_error(), "No error");

    cpuinfo_set_error(ERR_NO_CPUID);
    ASSERT_STR_EQ(cpuinfo_error(), "CPUID instruction not supported");

    cpuinfo_set_error(ERR_NO_MEM);
    ASSERT_STR_EQ(cpuinfo_error(), "Memory allocation failed");

    cpuinfo_set_error(ERR_OPEN);
    ASSERT_STR_EQ(cpuinfo_error(), "File open failed");
}

static void test_vendor_detection(void)
{
    ASSERT_EQ_INT(cpuinfo_detect_vendor("GenuineIntel"), VENDOR_INTEL);
    ASSERT_EQ_INT(cpuinfo_detect_vendor("AuthenticAMD"), VENDOR_AMD);
    ASSERT_EQ_INT(cpuinfo_detect_vendor("CyrixInstead"), VENDOR_CYRIX);
    ASSERT_EQ_INT(cpuinfo_detect_vendor("NexGenDriven"), VENDOR_NEXGEN);
    ASSERT_EQ_INT(cpuinfo_detect_vendor("GenuineTMx86"), VENDOR_TRANSMETA);
    ASSERT_EQ_INT(cpuinfo_detect_vendor("HygonGenuine"), VENDOR_HYGON);
    ASSERT_EQ_INT(cpuinfo_detect_vendor("UnknownVendo"), VENDOR_UNKNOWN);
}

static void test_vendor_str(void)
{
    ASSERT_STR_EQ(cpuinfo_vendor_str(VENDOR_INTEL), "Intel");
    ASSERT_STR_EQ(cpuinfo_vendor_str(VENDOR_AMD), "AMD");
    ASSERT_STR_EQ(cpuinfo_vendor_str(VENDOR_UNKNOWN), "unknown");
}

static void test_arch_str(void)
{
    ASSERT_STR_EQ(cpuinfo_arch_str(ARCH_X86), "x86");
    ASSERT_STR_EQ(cpuinfo_arch_str(ARCH_ARM), "ARM");
    ASSERT_STR_EQ(cpuinfo_arch_str(ARCH_UNKNOWN), "unknown");
}

static void test_feature_str(void)
{
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_FPU), "fpu");
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_SSE), "sse");
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_SSE2), "sse2");
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_AVX), "avx");
    ASSERT_STR_EQ(cpuinfo_feature_str(FEAT_AVX2), "avx2");
}

static void test_identify_intel(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;

    fill_intel_raw(&raw);
    int ret = cpuinfo_identify(&raw, &id);

    ASSERT_EQ_INT(ret, ERR_OK);
    ASSERT_EQ_INT(id.architecture, ARCH_X86);
    ASSERT_EQ_INT(id.vendor, VENDOR_INTEL);
    ASSERT_STR_EQ(id.vendor_str, "GenuineIntel");
    ASSERT_EQ_INT(id.family, 6);
    ASSERT_EQ_INT(id.stepping, 0xA);
    ASSERT_EQ_INT(id.ext_family, 6);
    ASSERT_EQ_INT(id.ext_model, 0x9E);

    /* Check brand string */
    ASSERT(strstr(id.brand_str, "Intel") != NULL);
    ASSERT(strstr(id.brand_str, "i7-8700K") != NULL);

    /* Check features */
    ASSERT(id.flags[FEAT_FPU]);
    ASSERT(id.flags[FEAT_TSC]);
    ASSERT(id.flags[FEAT_MMX]);
    ASSERT(id.flags[FEAT_SSE]);
    ASSERT(id.flags[FEAT_SSE2]);
    ASSERT(id.flags[FEAT_SSE3]);
    ASSERT(id.flags[FEAT_SSSE3]);
    ASSERT(id.flags[FEAT_SSE41]);
    ASSERT(id.flags[FEAT_SSE42]);
    ASSERT(id.flags[FEAT_AVX]);
    ASSERT(id.flags[FEAT_AES]);
    ASSERT(id.flags[FEAT_HT]);
    ASSERT(id.flags[FEAT_FMA3]);
    ASSERT(id.flags[FEAT_AVX2]);

    /* Core count */
    ASSERT_EQ_INT(id.num_cores, 6);
}

static void test_identify_amd(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;

    fill_amd_raw(&raw);
    int ret = cpuinfo_identify(&raw, &id);

    ASSERT_EQ_INT(ret, ERR_OK);
    ASSERT_EQ_INT(id.architecture, ARCH_X86);
    ASSERT_EQ_INT(id.vendor, VENDOR_AMD);
    ASSERT_STR_EQ(id.vendor_str, "AuthenticAMD");
    ASSERT_EQ_INT(id.family, 0xF);
    ASSERT_EQ_INT(id.ext_family, 25);
    ASSERT_EQ_INT(id.ext_model, 0x50);

    /* Check brand string */
    ASSERT(strstr(id.brand_str, "AMD Ryzen 9") != NULL);

    /* Check features */
    ASSERT(id.flags[FEAT_FPU]);
    ASSERT(id.flags[FEAT_SSE]);
    ASSERT(id.flags[FEAT_SSE2]);
    ASSERT(id.flags[FEAT_SSE3]);
    ASSERT(id.flags[FEAT_AVX]);
    ASSERT(id.flags[FEAT_AVX2]);
    ASSERT(id.flags[FEAT_CONSTANT_TSC]);

    /* Core count from ext leaf 8 */
    ASSERT(id.num_cores == 16);
}

static void test_identify_null_raw(void)
{
    /* Passing NULL for raw should try to read actual CPU - this just tests it doesn't crash */
    struct cpu_id_t id;
    int ret = cpuinfo_identify(NULL, &id);
    /* On x86 it should succeed, on other architectures it may fail with NO_CPUID */
    ASSERT(ret == ERR_OK || ret == ERR_NO_CPUID);
}

static void test_identify_null_id(void)
{
    struct cpu_raw_data_t raw;
    memset(&raw, 0, sizeof(raw));
    int ret = cpuinfo_identify(&raw, NULL);
    ASSERT_EQ_INT(ret, ERR_REQUEST);
}

static void test_init_id(void)
{
    struct cpu_id_t id;
    cpuinfo_init_id(&id);

    ASSERT_EQ_INT(id.architecture, ARCH_UNKNOWN);
    ASSERT_EQ_INT(id.vendor, VENDOR_UNKNOWN);
    ASSERT_EQ_INT(id.l1_data_cache, -1);
    ASSERT_EQ_INT(id.l2_cache, -1);
    ASSERT_EQ_INT(id.l3_cache, -1);
    ASSERT_EQ_INT(id.sse_size, -1);
    ASSERT_EQ_INT(id.family, 0);
    ASSERT_EQ_INT(id.flags[FEAT_SSE], 0);
}

static void test_empty_raw_returns_error(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;
    memset(&raw, 0, sizeof(raw));
    int ret = cpuinfo_identify(&raw, &id);
    ASSERT_EQ_INT(ret, ERR_NO_CPUID);
}

int main(void)
{
    printf("Running CPU identification tests...\n");

    TEST(test_version);
    TEST(test_error_reporting);
    TEST(test_vendor_detection);
    TEST(test_vendor_str);
    TEST(test_arch_str);
    TEST(test_feature_str);
    TEST(test_identify_intel);
    TEST(test_identify_amd);
    TEST(test_identify_null_raw);
    TEST(test_identify_null_id);
    TEST(test_init_id);
    TEST(test_empty_raw_returns_error);

    printf("\n%d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
