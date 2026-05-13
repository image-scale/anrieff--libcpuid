#include "cpuinfo.h"
#include "cache.h"
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

static void make_intel_cache_leaf(uint32_t regs[4],
    uint32_t level, uint32_t type,
    uint32_t ways, uint32_t partitions, uint32_t linesize, uint32_t sets,
    uint32_t num_sharing)
{
    regs[REG_EAX] = (type & 0x1F)
                  | ((level & 0x7) << 5)
                  | (((num_sharing - 1) & 0xFFF) << 14);
    regs[REG_EBX] = ((linesize - 1) & 0xFFF)
                  | (((partitions - 1) & 0x3FF) << 12)
                  | (((ways - 1) & 0x3FF) << 22);
    regs[REG_ECX] = sets - 1;
    regs[REG_EDX] = 0;
}

static int test_intel_l1_data_cache(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;
    memset(&raw, 0, sizeof(raw));
    cpuinfo_init_id(&id);
    id.vendor = VENDOR_INTEL;
    id.num_logical_cpus = 8;

    /* L1 data: type=1, level=1, 8-way, 1 partition, 64B line, 64 sets, shared by 2 */
    /* size = 8 * 1 * 64 * 64 / 1024 = 32 KB */
    make_intel_cache_leaf(raw.intel_cache[0], 1, 1, 8, 1, 64, 64, 2);

    int ret = cpuinfo_decode_caches(&raw, &id);
    ASSERT(ret == 0, "should return OK");
    ASSERT(id.l1_data_cache == 32, "L1D should be 32 KB");
    ASSERT(id.l1_data_assoc == 8, "L1D assoc should be 8");
    ASSERT(id.l1_data_cacheline == 64, "L1D line should be 64");
    ASSERT(id.l1_data_instances == 4, "L1D instances should be 8/2=4");
    return 1;
}

static int test_intel_l1_instruction_cache(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;
    memset(&raw, 0, sizeof(raw));
    cpuinfo_init_id(&id);
    id.vendor = VENDOR_INTEL;
    id.num_logical_cpus = 4;

    /* L1 instruction: type=2, level=1, 8-way, 1 partition, 64B line, 64 sets, shared by 2 */
    /* size = 8 * 1 * 64 * 64 / 1024 = 32 KB */
    make_intel_cache_leaf(raw.intel_cache[0], 1, 2, 8, 1, 64, 64, 2);

    int ret = cpuinfo_decode_caches(&raw, &id);
    ASSERT(ret == 0, "should return OK");
    ASSERT(id.l1_instruction_cache == 32, "L1I should be 32 KB");
    ASSERT(id.l1_instruction_assoc == 8, "L1I assoc should be 8");
    ASSERT(id.l1_instruction_cacheline == 64, "L1I line should be 64");
    ASSERT(id.l1_instruction_instances == 2, "L1I instances should be 4/2=2");
    return 1;
}

static int test_intel_l2_cache(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;
    memset(&raw, 0, sizeof(raw));
    cpuinfo_init_id(&id);
    id.vendor = VENDOR_INTEL;
    id.num_logical_cpus = 8;

    /* L2 unified: type=3, level=2, 4-way, 1 partition, 64B line, 1024 sets, shared by 2 */
    /* size = 4 * 1 * 64 * 1024 / 1024 = 256 KB */
    make_intel_cache_leaf(raw.intel_cache[0], 2, 3, 4, 1, 64, 1024, 2);

    int ret = cpuinfo_decode_caches(&raw, &id);
    ASSERT(ret == 0, "should return OK");
    ASSERT(id.l2_cache == 256, "L2 should be 256 KB");
    ASSERT(id.l2_assoc == 4, "L2 assoc should be 4");
    ASSERT(id.l2_cacheline == 64, "L2 line should be 64");
    ASSERT(id.l2_instances == 4, "L2 instances should be 8/2=4");
    return 1;
}

static int test_intel_l3_cache(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;
    memset(&raw, 0, sizeof(raw));
    cpuinfo_init_id(&id);
    id.vendor = VENDOR_INTEL;
    id.num_logical_cpus = 12;

    /* L3 unified: type=3, level=3, 16-way, 1 partition, 64B line, 12288 sets, shared by 12 */
    /* size = 16 * 1 * 64 * 12288 / 1024 = 12288 KB = 12 MB */
    make_intel_cache_leaf(raw.intel_cache[0], 3, 3, 16, 1, 64, 12288, 12);

    int ret = cpuinfo_decode_caches(&raw, &id);
    ASSERT(ret == 0, "should return OK");
    ASSERT(id.l3_cache == 12288, "L3 should be 12288 KB");
    ASSERT(id.l3_assoc == 16, "L3 assoc should be 16");
    ASSERT(id.l3_cacheline == 64, "L3 line should be 64");
    ASSERT(id.l3_instances == 1, "L3 instances should be 12/12=1");
    return 1;
}

static int test_intel_full_hierarchy(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;
    memset(&raw, 0, sizeof(raw));
    cpuinfo_init_id(&id);
    id.vendor = VENDOR_INTEL;
    id.num_logical_cpus = 16;

    /* Typical i7 hierarchy:
       L1D: 32KB, 8-way, 64B, 64 sets, shared by 2
       L1I: 32KB, 8-way, 64B, 64 sets, shared by 2
       L2:  256KB, 4-way, 64B, 1024 sets, shared by 2
       L3:  16MB, 16-way, 64B, 16384 sets, shared by 16
    */
    make_intel_cache_leaf(raw.intel_cache[0], 1, 1, 8, 1, 64, 64, 2);
    make_intel_cache_leaf(raw.intel_cache[1], 1, 2, 8, 1, 64, 64, 2);
    make_intel_cache_leaf(raw.intel_cache[2], 2, 3, 4, 1, 64, 1024, 2);
    make_intel_cache_leaf(raw.intel_cache[3], 3, 3, 16, 1, 64, 16384, 16);

    int ret = cpuinfo_decode_caches(&raw, &id);
    ASSERT(ret == 0, "should return OK");

    ASSERT(id.l1_data_cache == 32, "L1D = 32 KB");
    ASSERT(id.l1_data_assoc == 8, "L1D assoc = 8");
    ASSERT(id.l1_data_cacheline == 64, "L1D line = 64");
    ASSERT(id.l1_data_instances == 8, "L1D instances = 16/2 = 8");

    ASSERT(id.l1_instruction_cache == 32, "L1I = 32 KB");
    ASSERT(id.l1_instruction_assoc == 8, "L1I assoc = 8");
    ASSERT(id.l1_instruction_instances == 8, "L1I instances = 16/2 = 8");

    ASSERT(id.l2_cache == 256, "L2 = 256 KB");
    ASSERT(id.l2_assoc == 4, "L2 assoc = 4");
    ASSERT(id.l2_instances == 8, "L2 instances = 16/2 = 8");

    ASSERT(id.l3_cache == 16384, "L3 = 16384 KB");
    ASSERT(id.l3_assoc == 16, "L3 assoc = 16");
    ASSERT(id.l3_instances == 1, "L3 instances = 16/16 = 1");
    return 1;
}

static int test_amd_legacy_l1(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;
    memset(&raw, 0, sizeof(raw));
    cpuinfo_init_id(&id);
    id.vendor = VENDOR_AMD;
    id.num_logical_cpus = 8;

    /* ext leaf 0: report support up through 0x80000006 */
    raw.ext[0][REG_EAX] = 0x80000006;

    /* Leaf 0x80000005 ECX: L1 data cache
       bits[31:24] = size in KB = 64
       bits[23:16] = assoc = 4
       bits[7:0]   = line size = 64 */
    raw.ext[5][REG_ECX] = (64u << 24) | (4u << 16) | 64u;

    /* Leaf 0x80000005 EDX: L1 instruction cache
       bits[31:24] = size in KB = 32
       bits[23:16] = assoc = 2
       bits[7:0]   = line size = 64 */
    raw.ext[5][REG_EDX] = (32u << 24) | (2u << 16) | 64u;

    int ret = cpuinfo_decode_caches(&raw, &id);
    ASSERT(ret == 0, "should return OK");
    ASSERT(id.l1_data_cache == 64, "L1D = 64 KB");
    ASSERT(id.l1_data_assoc == 4, "L1D assoc = 4");
    ASSERT(id.l1_data_cacheline == 64, "L1D line = 64");
    ASSERT(id.l1_instruction_cache == 32, "L1I = 32 KB");
    ASSERT(id.l1_instruction_assoc == 2, "L1I assoc = 2");
    ASSERT(id.l1_instruction_cacheline == 64, "L1I line = 64");
    return 1;
}

static int test_amd_legacy_l2_l3(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;
    memset(&raw, 0, sizeof(raw));
    cpuinfo_init_id(&id);
    id.vendor = VENDOR_AMD;
    id.num_logical_cpus = 8;

    raw.ext[0][REG_EAX] = 0x80000006;

    /* Leaf 0x80000006 ECX: L2 cache
       bits[31:16] = size in KB = 512
       bits[15:12] = assoc encoding: 4 => 4-way
       bits[7:0]   = line size = 64 */
    raw.ext[6][REG_ECX] = (512u << 16) | (4u << 12) | 64u;

    /* Leaf 0x80000006 EDX: L3 cache
       bits[31:18] = size in 512KB units = 16 (=> 8192 KB)
       bits[15:12] = assoc encoding: 10 => 32-way
       bits[7:0]   = line size = 64 */
    raw.ext[6][REG_EDX] = (16u << 18) | (10u << 12) | 64u;

    int ret = cpuinfo_decode_caches(&raw, &id);
    ASSERT(ret == 0, "should return OK");
    ASSERT(id.l2_cache == 512, "L2 = 512 KB");
    ASSERT(id.l2_assoc == 4, "L2 assoc = 4");
    ASSERT(id.l2_cacheline == 64, "L2 line = 64");
    ASSERT(id.l3_cache == 8192, "L3 = 8192 KB");
    ASSERT(id.l3_assoc == 32, "L3 assoc = 32");
    ASSERT(id.l3_cacheline == 64, "L3 line = 64");
    return 1;
}

static int test_null_params(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;
    memset(&raw, 0, sizeof(raw));
    cpuinfo_init_id(&id);

    ASSERT(cpuinfo_decode_caches(NULL, &id) != 0, "null raw should fail");
    ASSERT(cpuinfo_decode_caches(&raw, NULL) != 0, "null id should fail");
    return 1;
}

static int test_empty_cache_data(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;
    memset(&raw, 0, sizeof(raw));
    cpuinfo_init_id(&id);
    id.vendor = VENDOR_INTEL;

    int ret = cpuinfo_decode_caches(&raw, &id);
    ASSERT(ret == 0, "should return OK even with no data");
    ASSERT(id.l1_data_cache == -1, "L1D should remain -1");
    ASSERT(id.l2_cache == -1, "L2 should remain -1");
    ASSERT(id.l3_cache == -1, "L3 should remain -1");
    return 1;
}

static int test_intel_l4_cache(void)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;
    memset(&raw, 0, sizeof(raw));
    cpuinfo_init_id(&id);
    id.vendor = VENDOR_INTEL;
    id.num_logical_cpus = 8;

    /* L4 unified: type=3, level=4, 16-way, 1 partition, 64B line, 131072 sets, shared by 8 */
    /* size = 16 * 1 * 64 * 131072 / 1024 = 131072 KB = 128 MB */
    make_intel_cache_leaf(raw.intel_cache[0], 4, 3, 16, 1, 64, 131072, 8);

    int ret = cpuinfo_decode_caches(&raw, &id);
    ASSERT(ret == 0, "should return OK");
    ASSERT(id.l4_cache == 131072, "L4 = 131072 KB");
    ASSERT(id.l4_assoc == 16, "L4 assoc = 16");
    ASSERT(id.l4_cacheline == 64, "L4 line = 64");
    ASSERT(id.l4_instances == 1, "L4 instances = 8/8 = 1");
    return 1;
}

int main(void)
{
    printf("=== Cache Topology Tests ===\n");

    RUN_TEST(test_intel_l1_data_cache);
    RUN_TEST(test_intel_l1_instruction_cache);
    RUN_TEST(test_intel_l2_cache);
    RUN_TEST(test_intel_l3_cache);
    RUN_TEST(test_intel_full_hierarchy);
    RUN_TEST(test_amd_legacy_l1);
    RUN_TEST(test_amd_legacy_l2_l3);
    RUN_TEST(test_null_params);
    RUN_TEST(test_empty_cache_data);
    RUN_TEST(test_intel_l4_cache);

    printf("\nResults: %d/%d tests passed\n", tests_passed, tests_run);
    if (tests_passed != tests_run) {
        printf("SOME TESTS FAILED\n");
        return 1;
    }
    printf("ALL TESTS PASSED\n");
    return 0;
}
