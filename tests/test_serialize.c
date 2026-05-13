#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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

#define ASSERT_EQ_U32(a, b) do { \
    uint32_t _a = (a), _b = (b); \
    if (_a != _b) { \
        printf("FAIL at %s:%d: 0x%08x != 0x%08x\n", __FILE__, __LINE__, _a, _b); \
        exit(1); \
    } \
} while(0)

static void fill_test_raw(struct cpu_raw_data_t* raw)
{
    memset(raw, 0, sizeof(*raw));

    raw->basic[0][REG_EAX] = 0x00000016;
    raw->basic[0][REG_EBX] = 0x756E6547;
    raw->basic[0][REG_EDX] = 0x49656E69;
    raw->basic[0][REG_ECX] = 0x6C65746E;
    raw->basic[1][REG_EAX] = 0x000906EA;
    raw->basic[1][REG_EBX] = 0x000C0800;
    raw->basic[1][REG_ECX] = 0x7FFAFBBF;
    raw->basic[1][REG_EDX] = 0x1F8BFBFF;
    raw->basic[7][REG_EBX] = 0x209C03A9;

    raw->ext[0][REG_EAX] = 0x80000008;
    raw->ext[1][REG_EDX] = 0x2C100800;
    raw->ext[2][REG_EAX] = 0x65746E49;
    raw->ext[2][REG_EBX] = 0x2952286C;

    raw->intel_cache[0][REG_EAX] = 0x14000121;
    raw->intel_cache[0][REG_EBX] = 0x01C0003F;
    raw->intel_cache[1][REG_EAX] = 0x14000122;

    raw->intel_topology[0][REG_EAX] = 0x00000001;
    raw->intel_topology[0][REG_EBX] = 0x00000002;
}

static void test_serialize_and_deserialize(void)
{
    struct cpu_raw_data_t raw_orig, raw_loaded;
    const char* tmpfile = "/tmp/cpuinfo_test_serialize.txt";

    fill_test_raw(&raw_orig);

    int ret = cpuinfo_serialize_raw(&raw_orig, tmpfile);
    ASSERT_EQ_INT(ret, ERR_OK);

    ret = cpuinfo_deserialize_raw(&raw_loaded, tmpfile);
    ASSERT_EQ_INT(ret, ERR_OK);

    /* Verify basic leaves match */
    ASSERT_EQ_U32(raw_loaded.basic[0][REG_EAX], raw_orig.basic[0][REG_EAX]);
    ASSERT_EQ_U32(raw_loaded.basic[0][REG_EBX], raw_orig.basic[0][REG_EBX]);
    ASSERT_EQ_U32(raw_loaded.basic[0][REG_ECX], raw_orig.basic[0][REG_ECX]);
    ASSERT_EQ_U32(raw_loaded.basic[0][REG_EDX], raw_orig.basic[0][REG_EDX]);
    ASSERT_EQ_U32(raw_loaded.basic[1][REG_EAX], raw_orig.basic[1][REG_EAX]);
    ASSERT_EQ_U32(raw_loaded.basic[1][REG_ECX], raw_orig.basic[1][REG_ECX]);
    ASSERT_EQ_U32(raw_loaded.basic[7][REG_EBX], raw_orig.basic[7][REG_EBX]);

    /* Verify ext leaves match */
    ASSERT_EQ_U32(raw_loaded.ext[0][REG_EAX], raw_orig.ext[0][REG_EAX]);
    ASSERT_EQ_U32(raw_loaded.ext[1][REG_EDX], raw_orig.ext[1][REG_EDX]);
    ASSERT_EQ_U32(raw_loaded.ext[2][REG_EAX], raw_orig.ext[2][REG_EAX]);

    /* Verify intel_cache leaves */
    ASSERT_EQ_U32(raw_loaded.intel_cache[0][REG_EAX], raw_orig.intel_cache[0][REG_EAX]);
    ASSERT_EQ_U32(raw_loaded.intel_cache[0][REG_EBX], raw_orig.intel_cache[0][REG_EBX]);
    ASSERT_EQ_U32(raw_loaded.intel_cache[1][REG_EAX], raw_orig.intel_cache[1][REG_EAX]);

    /* Verify intel_topology leaves */
    ASSERT_EQ_U32(raw_loaded.intel_topology[0][REG_EAX], raw_orig.intel_topology[0][REG_EAX]);
    ASSERT_EQ_U32(raw_loaded.intel_topology[0][REG_EBX], raw_orig.intel_topology[0][REG_EBX]);

    unlink(tmpfile);
}

static void test_roundtrip_preserves_all_data(void)
{
    struct cpu_raw_data_t raw_orig, raw_loaded;
    const char* tmpfile = "/tmp/cpuinfo_test_roundtrip.txt";

    fill_test_raw(&raw_orig);

    cpuinfo_serialize_raw(&raw_orig, tmpfile);
    cpuinfo_deserialize_raw(&raw_loaded, tmpfile);

    /* Full memory comparison */
    ASSERT(memcmp(&raw_orig, &raw_loaded, sizeof(struct cpu_raw_data_t)) == 0);

    unlink(tmpfile);
}

static void test_identify_from_deserialized(void)
{
    struct cpu_raw_data_t raw_orig, raw_loaded;
    struct cpu_id_t id_orig, id_loaded;
    const char* tmpfile = "/tmp/cpuinfo_test_identify.txt";

    fill_test_raw(&raw_orig);

    cpuinfo_identify(&raw_orig, &id_orig);
    cpuinfo_serialize_raw(&raw_orig, tmpfile);
    cpuinfo_deserialize_raw(&raw_loaded, tmpfile);
    cpuinfo_identify(&raw_loaded, &id_loaded);

    ASSERT_EQ_INT(id_orig.vendor, id_loaded.vendor);
    ASSERT_EQ_INT(id_orig.family, id_loaded.family);
    ASSERT_EQ_INT(id_orig.model, id_loaded.model);
    ASSERT_EQ_INT(id_orig.stepping, id_loaded.stepping);
    ASSERT_EQ_INT(id_orig.ext_family, id_loaded.ext_family);
    ASSERT_EQ_INT(id_orig.ext_model, id_loaded.ext_model);
    ASSERT(strcmp(id_orig.vendor_str, id_loaded.vendor_str) == 0);

    /* Feature flags should match */
    for (int i = 0; i < NUM_FEATURES; i++) {
        ASSERT(id_orig.flags[i] == id_loaded.flags[i]);
    }

    unlink(tmpfile);
}

static void test_serialize_null_raw(void)
{
    int ret = cpuinfo_serialize_raw(NULL, "/tmp/test_null.txt");
    ASSERT_EQ_INT(ret, ERR_REQUEST);
}

static void test_deserialize_null_raw(void)
{
    int ret = cpuinfo_deserialize_raw(NULL, "/tmp/test_null.txt");
    ASSERT_EQ_INT(ret, ERR_REQUEST);
}

static void test_deserialize_nonexistent_file(void)
{
    struct cpu_raw_data_t raw;
    int ret = cpuinfo_deserialize_raw(&raw, "/tmp/does_not_exist_xyz123.txt");
    ASSERT_EQ_INT(ret, ERR_OPEN);
}

static void test_serialize_file_format(void)
{
    struct cpu_raw_data_t raw;
    const char* tmpfile = "/tmp/cpuinfo_test_format.txt";
    char line[256];

    fill_test_raw(&raw);
    cpuinfo_serialize_raw(&raw, tmpfile);

    FILE* f = fopen(tmpfile, "r");
    ASSERT(f != NULL);

    /* First line should be version */
    ASSERT(fgets(line, sizeof(line), f) != NULL);
    ASSERT(strstr(line, "version=") != NULL);

    /* Should have basic_cpuid lines */
    int found_basic = 0;
    int found_ext = 0;
    rewind(f);
    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "basic_cpuid[") != NULL) found_basic++;
        if (strstr(line, "ext_cpuid[") != NULL) found_ext++;
    }
    ASSERT_EQ_INT(found_basic, MAX_BASIC_LEAVES);
    ASSERT_EQ_INT(found_ext, MAX_EXT_LEAVES);

    fclose(f);
    unlink(tmpfile);
}

int main(void)
{
    printf("Running serialization tests...\n");

    TEST(test_serialize_and_deserialize);
    TEST(test_roundtrip_preserves_all_data);
    TEST(test_identify_from_deserialized);
    TEST(test_serialize_null_raw);
    TEST(test_deserialize_null_raw);
    TEST(test_deserialize_nonexistent_file);
    TEST(test_serialize_file_format);

    printf("\n%d/%d tests passed.\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
