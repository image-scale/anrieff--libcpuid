#define _GNU_SOURCE
#include "cpuinfo.h"
#include "cpudb.h"
#include "cache.h"
#include "clock.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT(cond, msg) do { \
    tests_run++; \
    if (!(cond)) { \
        printf("FAIL: %s (line %d): %s\n", __func__, __LINE__, msg); \
        return 0; \
    } \
    tests_passed++; \
} while(0)

#define RUN_TEST(fn) do { \
    printf("  Running %s...\n", #fn); \
    fn(); \
} while(0)

static const char* TEST_RAW_FILE = "/tmp/test_tool_raw.txt";

static void create_test_raw_file(void)
{
    struct cpu_raw_data_t raw;
    memset(&raw, 0, sizeof(raw));

    /* Intel i7 Coffee Lake mock data */
    /* leaf 0: max_basic=0x16, vendor=GenuineIntel */
    raw.basic[0][REG_EAX] = 0x16;
    raw.basic[0][REG_EBX] = 0x756E6547; /* Genu */
    raw.basic[0][REG_EDX] = 0x49656E69; /* ineI */
    raw.basic[0][REG_ECX] = 0x6C65746E; /* ntel */

    /* leaf 1: family=6, model=14, stepping=10 => ext_family=6, ext_model=0x9E */
    /* EAX format: [27:20]=ext_fam, [19:16]=ext_mod, [11:8]=fam, [7:4]=model, [3:0]=stepping */
    /* family=6, model=0xE, stepping=0xA, ext_model=0x9, ext_family=0 */
    raw.basic[1][REG_EAX] = (0 << 20) | (0x9 << 16) | (6 << 8) | (0xE << 4) | 0xA;
    raw.basic[1][REG_EBX] = (12 << 16); /* 12 logical cpus reported */
    raw.basic[1][REG_EDX] = (1u << 28) | (1u << 26) | (1u << 25) | (1u << 23) | (1u << 0); /* HT, SSE2, SSE, MMX, FPU */
    raw.basic[1][REG_ECX] = (1u << 28) | (1u << 0); /* AVX, SSE3 */

    /* ext leaf 0 */
    raw.ext[0][REG_EAX] = 0x80000008;

    /* ext leaf 2-4: brand string "Intel(R) Core(TM) i7-8700K CPU @ 3.70GHz" */
    const char* brand = "Intel(R) Core(TM) i7-8700K CPU @ 3.70GHz\0\0\0\0\0\0\0";
    memcpy(&raw.ext[2][0], brand + 0, 16);
    memcpy(&raw.ext[3][0], brand + 16, 16);
    memcpy(&raw.ext[4][0], brand + 32, 16);

    /* Intel cache leaf 4 data: L1D 32KB 8-way 64B 64sets */
    raw.intel_cache[0][REG_EAX] = 1 | (1 << 5) | ((2 - 1) << 14);
    raw.intel_cache[0][REG_EBX] = (64 - 1) | ((1 - 1) << 12) | ((8 - 1) << 22);
    raw.intel_cache[0][REG_ECX] = 64 - 1;

    /* L2 256KB 4-way 64B 1024sets */
    raw.intel_cache[1][REG_EAX] = 3 | (2 << 5) | ((2 - 1) << 14);
    raw.intel_cache[1][REG_EBX] = (64 - 1) | ((1 - 1) << 12) | ((4 - 1) << 22);
    raw.intel_cache[1][REG_ECX] = 1024 - 1;

    /* L3 12MB 16-way 64B 12288sets shared by 12 */
    raw.intel_cache[2][REG_EAX] = 3 | (3 << 5) | ((12 - 1) << 14);
    raw.intel_cache[2][REG_EBX] = (64 - 1) | ((1 - 1) << 12) | ((16 - 1) << 22);
    raw.intel_cache[2][REG_ECX] = 12288 - 1;

    cpuinfo_serialize_raw(&raw, TEST_RAW_FILE);
}

static int run_tool(const char* args, char* output, int output_size)
{
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "./tools/cputool --load %s %s 2>&1", TEST_RAW_FILE, args);
    FILE* f = popen(cmd, "r");
    if (!f) return -1;
    int total = 0;
    while (total < output_size - 1 && fgets(output + total, output_size - total, f)) {
        total = (int)strlen(output);
    }
    return pclose(f);
}

static int test_tool_version(void)
{
    char output[256] = {0};
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "./tools/cputool --version 2>&1");
    FILE* f = popen(cmd, "r");
    if (!f) { ASSERT(0, "popen failed"); return 0; }
    fgets(output, sizeof(output), f);
    pclose(f);
    ASSERT(strstr(output, "0.1.0") != NULL, "version should contain 0.1.0");
    return 1;
}

static int test_tool_vendorstr(void)
{
    char output[256] = {0};
    int ret = run_tool("--vendorstr", output, sizeof(output));
    ASSERT(ret == 0, "tool should exit 0");
    ASSERT(strstr(output, "GenuineIntel") != NULL, "should show GenuineIntel");
    return 1;
}

static int test_tool_brandstr(void)
{
    char output[512] = {0};
    int ret = run_tool("--brandstr", output, sizeof(output));
    ASSERT(ret == 0, "tool should exit 0");
    ASSERT(strstr(output, "i7-8700K") != NULL, "should show i7-8700K");
    return 1;
}

static int test_tool_family(void)
{
    char output[64] = {0};
    int ret = run_tool("--family", output, sizeof(output));
    ASSERT(ret == 0, "tool should exit 0");
    ASSERT(strstr(output, "6") != NULL, "family should be 6");
    return 1;
}

static int test_tool_extmodel(void)
{
    char output[64] = {0};
    int ret = run_tool("--extmodel", output, sizeof(output));
    ASSERT(ret == 0, "tool should exit 0");
    ASSERT(strstr(output, "158") != NULL, "ext_model 0x9E = 158");
    return 1;
}

static int test_tool_codename(void)
{
    char output[256] = {0};
    int ret = run_tool("--codename", output, sizeof(output));
    ASSERT(ret == 0, "tool should exit 0");
    ASSERT(strstr(output, "Coffee Lake") != NULL, output);
    return 1;
}

static int test_tool_technology(void)
{
    char output[64] = {0};
    int ret = run_tool("--technology", output, sizeof(output));
    ASSERT(ret == 0, "tool should exit 0");
    ASSERT(strstr(output, "14 nm") != NULL, output);
    return 1;
}

static int test_tool_cache(void)
{
    char output[64] = {0};
    int ret = run_tool("--cache", output, sizeof(output));
    ASSERT(ret == 0, "tool should exit 0");
    ASSERT(strstr(output, "256") != NULL, "L2 should be 256");
    return 1;
}

static int test_tool_l3_cache(void)
{
    char output[64] = {0};
    int ret = run_tool("--l3-cache", output, sizeof(output));
    ASSERT(ret == 0, "tool should exit 0");
    ASSERT(strstr(output, "12288") != NULL, "L3 should be 12288 KB");
    return 1;
}

static int test_tool_flags(void)
{
    char output[4096] = {0};
    int ret = run_tool("--flags", output, sizeof(output));
    ASSERT(ret == 0, "tool should exit 0");
    ASSERT(strstr(output, "fpu") != NULL, "should have fpu");
    ASSERT(strstr(output, "sse2") != NULL, "should have sse2");
    ASSERT(strstr(output, "avx") != NULL, "should have avx");
    return 1;
}

static int test_tool_report(void)
{
    char output[4096] = {0};
    int ret = run_tool("--report", output, sizeof(output));
    ASSERT(ret == 0, "tool should exit 0");
    ASSERT(strstr(output, "GenuineIntel") != NULL, "report should show vendor");
    ASSERT(strstr(output, "Coffee Lake") != NULL, "report should show codename");
    ASSERT(strstr(output, "L2 Cache") != NULL, "report should show cache info");
    return 1;
}

static int test_tool_save_load_roundtrip(void)
{
    const char* save_path = "/tmp/test_tool_save.txt";
    char cmd[512];
    char output[4096] = {0};

    snprintf(cmd, sizeof(cmd), "./tools/cputool --load %s --save %s 2>&1", TEST_RAW_FILE, save_path);
    FILE* f = popen(cmd, "r");
    if (f) {
        int total = 0;
        while (total < (int)sizeof(output) - 1 && fgets(output + total, (int)(sizeof(output) - total), f))
            total = (int)strlen(output);
        pclose(f);
    }

    char output2[256] = {0};
    snprintf(cmd, sizeof(cmd), "./tools/cputool --load %s --family 2>&1", save_path);
    f = popen(cmd, "r");
    if (!f) { ASSERT(0, "popen failed"); return 0; }
    fgets(output2, sizeof(output2), f);
    pclose(f);
    ASSERT(strstr(output2, "6") != NULL, "re-loaded family should be 6");
    remove(save_path);
    return 1;
}

static int test_tool_unknown_option(void)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "./tools/cputool --bogus 2>&1");
    FILE* f = popen(cmd, "r");
    if (!f) { ASSERT(0, "popen failed"); return 0; }
    char output[4096] = {0};
    int total = 0;
    while (total < (int)sizeof(output) - 1 && fgets(output + total, (int)(sizeof(output) - total), f)) {
        total = (int)strlen(output);
    }
    int ret = pclose(f);
    ASSERT(ret != 0, "tool should exit non-zero for unknown option");
    ASSERT(strstr(output, "Unknown option") != NULL, "should show error");
    return 1;
}

int main(void)
{
    printf("=== CPU Tool Integration Tests ===\n");

    create_test_raw_file();

    RUN_TEST(test_tool_version);
    RUN_TEST(test_tool_vendorstr);
    RUN_TEST(test_tool_brandstr);
    RUN_TEST(test_tool_family);
    RUN_TEST(test_tool_extmodel);
    RUN_TEST(test_tool_codename);
    RUN_TEST(test_tool_technology);
    RUN_TEST(test_tool_cache);
    RUN_TEST(test_tool_l3_cache);
    RUN_TEST(test_tool_flags);
    RUN_TEST(test_tool_report);
    RUN_TEST(test_tool_save_load_roundtrip);
    RUN_TEST(test_tool_unknown_option);

    remove(TEST_RAW_FILE);

    printf("\nResults: %d/%d tests passed\n", tests_passed, tests_run);
    if (tests_passed != tests_run) {
        printf("SOME TESTS FAILED\n");
        return 1;
    }
    printf("ALL TESTS PASSED\n");
    return 0;
}
