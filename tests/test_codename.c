#include "cpuinfo.h"
#include "cpudb.h"
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

static void init_test_id(struct cpu_id_t* id)
{
    cpuinfo_init_id(id);
}

static int test_intel_coffee_lake(void)
{
    struct cpu_id_t id;
    init_test_id(&id);
    id.vendor = VENDOR_INTEL;
    id.family = 6;
    id.model = -1;
    id.stepping = -1;
    id.ext_family = 6;
    id.ext_model = 0x9E;
    id.num_cores = 6;
    id.l2_cache = -1;
    id.l3_cache = -1;
    strcpy(id.brand_str, "Intel(R) Core(TM) i7-8700K CPU @ 3.70GHz");

    int ret = cpudb_identify_codename(&id);
    ASSERT(ret == 0, "cpudb_identify_codename should return 0");
    ASSERT(strcmp(id.cpu_codename, "Coffee Lake") == 0, id.cpu_codename);
    ASSERT(strcmp(id.technology, "14 nm") == 0, id.technology);
    return 1;
}

static int test_intel_skylake(void)
{
    struct cpu_id_t id;
    init_test_id(&id);
    id.vendor = VENDOR_INTEL;
    id.family = 6;
    id.model = -1;
    id.stepping = -1;
    id.ext_family = 6;
    id.ext_model = 0x5E;
    id.num_cores = 4;
    id.l2_cache = -1;
    id.l3_cache = -1;
    strcpy(id.brand_str, "Intel(R) Core(TM) i5-6600K CPU @ 3.50GHz");

    int ret = cpudb_identify_codename(&id);
    ASSERT(ret == 0, "cpudb_identify_codename should return 0");
    ASSERT(strcmp(id.cpu_codename, "Skylake") == 0, id.cpu_codename);
    ASSERT(strcmp(id.technology, "14 nm") == 0, id.technology);
    return 1;
}

static int test_intel_alder_lake(void)
{
    struct cpu_id_t id;
    init_test_id(&id);
    id.vendor = VENDOR_INTEL;
    id.family = 6;
    id.model = -1;
    id.stepping = -1;
    id.ext_family = 6;
    id.ext_model = 0x97;
    id.num_cores = 8;
    id.l2_cache = -1;
    id.l3_cache = -1;
    strcpy(id.brand_str, "12th Gen Intel(R) Core(TM) i7-12700K");

    int ret = cpudb_identify_codename(&id);
    ASSERT(ret == 0, "cpudb_identify_codename should return 0");
    ASSERT(strcmp(id.cpu_codename, "Alder Lake") == 0, id.cpu_codename);
    ASSERT(strcmp(id.technology, "Intel 7") == 0, id.technology);
    return 1;
}

static int test_intel_haswell(void)
{
    struct cpu_id_t id;
    init_test_id(&id);
    id.vendor = VENDOR_INTEL;
    id.family = 6;
    id.model = -1;
    id.stepping = -1;
    id.ext_family = 6;
    id.ext_model = 0x3C;
    id.num_cores = 4;
    id.l2_cache = -1;
    id.l3_cache = -1;
    strcpy(id.brand_str, "Intel(R) Core(TM) i7-4770K CPU @ 3.50GHz");

    int ret = cpudb_identify_codename(&id);
    ASSERT(ret == 0, "cpudb_identify_codename should return 0");
    ASSERT(strcmp(id.cpu_codename, "Haswell") == 0, id.cpu_codename);
    ASSERT(strcmp(id.technology, "22 nm") == 0, id.technology);
    return 1;
}

static int test_amd_zen3_vermeer(void)
{
    struct cpu_id_t id;
    init_test_id(&id);
    id.vendor = VENDOR_AMD;
    id.family = 15;
    id.model = -1;
    id.stepping = -1;
    id.ext_family = 25;
    id.ext_model = 0x21;
    id.num_cores = 8;
    id.l2_cache = -1;
    id.l3_cache = -1;
    strcpy(id.brand_str, "AMD Ryzen 7 5800X 8-Core Processor");

    int ret = cpudb_identify_codename(&id);
    ASSERT(ret == 0, "cpudb_identify_codename should return 0");
    ASSERT(strcmp(id.cpu_codename, "Ryzen (Vermeer)") == 0, id.cpu_codename);
    ASSERT(strcmp(id.technology, "TSMC N7FF") == 0, id.technology);
    return 1;
}

static int test_amd_zen2_matisse(void)
{
    struct cpu_id_t id;
    init_test_id(&id);
    id.vendor = VENDOR_AMD;
    id.family = 15;
    id.model = -1;
    id.stepping = -1;
    id.ext_family = 23;
    id.ext_model = 0x71;
    id.num_cores = 12;
    id.l2_cache = -1;
    id.l3_cache = -1;
    strcpy(id.brand_str, "AMD Ryzen 9 3900X 12-Core Processor");

    int ret = cpudb_identify_codename(&id);
    ASSERT(ret == 0, "cpudb_identify_codename should return 0");
    ASSERT(strcmp(id.cpu_codename, "Ryzen (Matisse)") == 0, id.cpu_codename);
    ASSERT(strcmp(id.technology, "TSMC N7FF") == 0, id.technology);
    return 1;
}

static int test_amd_zen_plus(void)
{
    struct cpu_id_t id;
    init_test_id(&id);
    id.vendor = VENDOR_AMD;
    id.family = 15;
    id.model = -1;
    id.stepping = -1;
    id.ext_family = 23;
    id.ext_model = 0x08;
    id.num_cores = 8;
    id.l2_cache = -1;
    id.l3_cache = -1;
    strcpy(id.brand_str, "AMD Ryzen 7 2700X Eight-Core Processor");

    int ret = cpudb_identify_codename(&id);
    ASSERT(ret == 0, "cpudb_identify_codename should return 0");
    ASSERT(strcmp(id.cpu_codename, "Ryzen (Zen+)") == 0, id.cpu_codename);
    ASSERT(strcmp(id.technology, "12 nm") == 0, id.technology);
    return 1;
}

static int test_amd_bulldozer(void)
{
    struct cpu_id_t id;
    init_test_id(&id);
    id.vendor = VENDOR_AMD;
    id.family = 15;
    id.model = -1;
    id.stepping = -1;
    id.ext_family = 21;
    id.ext_model = -1;
    id.num_cores = 8;
    id.l2_cache = -1;
    id.l3_cache = -1;
    strcpy(id.brand_str, "AMD FX(tm)-8150 Eight-Core Processor");

    int ret = cpudb_identify_codename(&id);
    ASSERT(ret == 0, "cpudb_identify_codename should return 0");
    ASSERT(strcmp(id.cpu_codename, "Bulldozer") == 0, id.cpu_codename);
    ASSERT(strcmp(id.technology, "32 nm") == 0, id.technology);
    return 1;
}

static int test_unknown_vendor(void)
{
    struct cpu_id_t id;
    init_test_id(&id);
    id.vendor = VENDOR_CYRIX;
    id.family = 6;
    id.model = 1;
    id.stepping = 0;
    id.ext_family = -1;
    id.ext_model = -1;

    int ret = cpudb_identify_codename(&id);
    ASSERT(ret == 0, "cpudb_identify_codename should return 0");
    ASSERT(strcmp(id.cpu_codename, "Unknown") == 0, id.cpu_codename);
    ASSERT(strcmp(id.technology, "unknown") == 0, id.technology);
    return 1;
}

static int test_null_id(void)
{
    int ret = cpudb_identify_codename(NULL);
    ASSERT(ret != 0, "cpudb_identify_codename(NULL) should return error");
    return 1;
}

static int test_brand_pattern_match(void)
{
    struct cpu_id_t id;
    init_test_id(&id);
    id.vendor = VENDOR_AMD;
    id.family = 15;
    id.model = -1;
    id.stepping = -1;
    id.ext_family = 25;
    id.ext_model = 0x50;
    id.num_cores = 8;
    id.l2_cache = -1;
    id.l3_cache = -1;
    strcpy(id.brand_str, "AMD Ryzen 9 5900HX with Radeon Graphics");

    int ret = cpudb_identify_codename(&id);
    ASSERT(ret == 0, "cpudb_identify_codename should return 0");
    ASSERT(strcmp(id.cpu_codename, "Ryzen 9 (Cezanne)") == 0, id.cpu_codename);
    ASSERT(strcmp(id.technology, "TSMC N7FF") == 0, id.technology);
    return 1;
}

static int test_get_list_intel(void)
{
    const char** names = NULL;
    int count = 0;
    cpudb_get_list(VENDOR_INTEL, &names, &count);
    ASSERT(count > 0, "Intel list should have entries");
    ASSERT(names != NULL, "Intel names should not be NULL");

    int found_skylake = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(names[i], "Skylake") == 0) found_skylake = 1;
    }
    ASSERT(found_skylake, "Intel list should contain Skylake");
    return 1;
}

static int test_get_list_amd(void)
{
    const char** names = NULL;
    int count = 0;
    cpudb_get_list(VENDOR_AMD, &names, &count);
    ASSERT(count > 0, "AMD list should have entries");
    ASSERT(names != NULL, "AMD names should not be NULL");

    int found_zen = 0;
    for (int i = 0; i < count; i++) {
        if (strstr(names[i], "Zen") || strstr(names[i], "Ryzen")) found_zen = 1;
    }
    ASSERT(found_zen, "AMD list should contain Zen/Ryzen entries");
    return 1;
}

static int test_hygon_uses_amd_db(void)
{
    struct cpu_id_t id;
    init_test_id(&id);
    id.vendor = VENDOR_HYGON;
    id.family = 15;
    id.model = -1;
    id.stepping = -1;
    id.ext_family = 24;
    id.ext_model = -1;
    id.num_cores = 8;
    id.l2_cache = -1;
    id.l3_cache = -1;
    strcpy(id.brand_str, "Hygon C86 7185 32-core Processor");

    int ret = cpudb_identify_codename(&id);
    ASSERT(ret == 0, "cpudb_identify_codename should return 0 for Hygon");
    ASSERT(id.cpu_codename[0] != '\0', "Hygon should get a codename from AMD DB");
    return 1;
}

int main(void)
{
    printf("=== CPU Codename Database Tests ===\n");

    RUN_TEST(test_intel_coffee_lake);
    RUN_TEST(test_intel_skylake);
    RUN_TEST(test_intel_alder_lake);
    RUN_TEST(test_intel_haswell);
    RUN_TEST(test_amd_zen3_vermeer);
    RUN_TEST(test_amd_zen2_matisse);
    RUN_TEST(test_amd_zen_plus);
    RUN_TEST(test_amd_bulldozer);
    RUN_TEST(test_unknown_vendor);
    RUN_TEST(test_null_id);
    RUN_TEST(test_brand_pattern_match);
    RUN_TEST(test_get_list_intel);
    RUN_TEST(test_get_list_amd);
    RUN_TEST(test_hygon_uses_amd_db);

    printf("\nResults: %d/%d tests passed\n", tests_passed, tests_run);
    if (tests_passed != tests_run) {
        printf("SOME TESTS FAILED\n");
        return 1;
    }
    printf("ALL TESTS PASSED\n");
    return 0;
}
