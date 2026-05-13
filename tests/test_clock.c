#include "cpuinfo.h"
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

static int test_clock_by_os(void)
{
    int mhz = cpu_clock_by_os();
    /* On CI or containers this may return -1, which is acceptable */
    ASSERT(mhz == -1 || mhz > 0, "clock_by_os should return -1 or positive MHz");
    if (mhz > 0) {
        ASSERT(mhz < 100000, "clock_by_os should be < 100 GHz");
    }
    return 1;
}

static int test_clock_by_tsc_with_crystal(void)
{
    struct cpu_raw_data_t raw;
    memset(&raw, 0, sizeof(raw));

    /* Simulate Intel CPU with leaf 0x15 support
       max leaf >= 0x15 */
    raw.basic[0][REG_EAX] = 0x16;

    /* Leaf 0x15:
       EAX = denominator = 2
       EBX = numerator = 202
       ECX = crystal clock Hz = 24000000 (24 MHz) */
    raw.basic[0x15][REG_EAX] = 2;
    raw.basic[0x15][REG_EBX] = 202;
    raw.basic[0x15][REG_ECX] = 24000000;

    /* Expected: 24000 kHz * 202 / 2 / 1000 = 2424 MHz */
    int mhz = cpu_clock_by_tsc(&raw);
    ASSERT(mhz == 2424, "TSC with crystal should give 2424 MHz");
    return 1;
}

static int test_clock_by_tsc_with_base_freq(void)
{
    struct cpu_raw_data_t raw;
    memset(&raw, 0, sizeof(raw));

    /* max leaf >= 0x16 */
    raw.basic[0][REG_EAX] = 0x16;

    /* Leaf 0x15: no crystal clock (ECX=0), but ratio given */
    raw.basic[0x15][REG_EAX] = 2;    /* denominator */
    raw.basic[0x15][REG_EBX] = 100;  /* numerator */
    raw.basic[0x15][REG_ECX] = 0;    /* no crystal clock */

    /* Leaf 0x16: base frequency in MHz */
    raw.basic[0x16][REG_EAX] = 3600; /* 3600 MHz base */

    /* nominal_freq_khz = 3600 * 1000 * 2 / 100 = 72000 kHz
       TSC MHz = 72000 * 100 / 2 / 1000 = 3600 MHz */
    int mhz = cpu_clock_by_tsc(&raw);
    ASSERT(mhz == 3600, "TSC with base freq should give 3600 MHz");
    return 1;
}

static int test_clock_by_tsc_no_support(void)
{
    struct cpu_raw_data_t raw;
    memset(&raw, 0, sizeof(raw));

    /* max leaf < 0x15 */
    raw.basic[0][REG_EAX] = 0x0B;

    int mhz = cpu_clock_by_tsc(&raw);
    ASSERT(mhz == -1, "TSC should return -1 when leaf 0x15 not supported");
    return 1;
}

static int test_clock_by_tsc_null(void)
{
    int mhz = cpu_clock_by_tsc(NULL);
    ASSERT(mhz == -1, "TSC should return -1 for NULL raw");
    return 1;
}

static int test_clock_by_tsc_zero_ratio(void)
{
    struct cpu_raw_data_t raw;
    memset(&raw, 0, sizeof(raw));

    raw.basic[0][REG_EAX] = 0x16;
    raw.basic[0x15][REG_EAX] = 0;
    raw.basic[0x15][REG_EBX] = 0;
    raw.basic[0x15][REG_ECX] = 24000000;

    int mhz = cpu_clock_by_tsc(&raw);
    ASSERT(mhz == -1, "TSC should return -1 when ratio is 0/0");
    return 1;
}

static int test_clock_by_mark_zero_time(void)
{
    struct cpu_mark_t mark;
    mark.tsc = 3000000000ULL;
    mark.sys_clock = 0;

    int mhz = cpu_clock_by_mark(&mark);
    ASSERT(mhz == -1, "clock_by_mark should return -1 for zero sys_clock");
    return 1;
}

static int test_clock_by_mark_normal(void)
{
    struct cpu_mark_t mark;
    /* Simulate 3 GHz: 3,000,000,000 ticks in 1,000,000 us */
    mark.tsc = 3000000000ULL;
    mark.sys_clock = 1000000ULL;

    int mhz = cpu_clock_by_mark(&mark);
    ASSERT(mhz == 3000, "clock_by_mark should give 3000 MHz");
    return 1;
}

static int test_clock_by_mark_null(void)
{
    int mhz = cpu_clock_by_mark(NULL);
    ASSERT(mhz == -1, "clock_by_mark(NULL) should return -1");
    return 1;
}

static int test_clock_measure_invalid(void)
{
    int mhz = cpu_clock_measure(0);
    ASSERT(mhz == -1, "clock_measure(0) should return -1");
    mhz = cpu_clock_measure(-1);
    ASSERT(mhz == -1, "clock_measure(-1) should return -1");
    return 1;
}

static int test_tsc_mark_unmark(void)
{
    struct cpu_mark_t mark;
    cpu_tsc_mark(&mark);
    ASSERT(mark.sys_clock > 0, "sys_clock should be positive after mark");

    /* busy work */
    volatile int x = 0;
    for (int i = 0; i < 100000; i++) x += i;
    (void)x;

    cpu_tsc_unmark(&mark);
    ASSERT(mark.sys_clock > 0, "elapsed sys_clock should be positive");
    return 1;
}

int main(void)
{
    printf("=== CPU Clock Measurement Tests ===\n");

    RUN_TEST(test_clock_by_os);
    RUN_TEST(test_clock_by_tsc_with_crystal);
    RUN_TEST(test_clock_by_tsc_with_base_freq);
    RUN_TEST(test_clock_by_tsc_no_support);
    RUN_TEST(test_clock_by_tsc_null);
    RUN_TEST(test_clock_by_tsc_zero_ratio);
    RUN_TEST(test_clock_by_mark_zero_time);
    RUN_TEST(test_clock_by_mark_normal);
    RUN_TEST(test_clock_by_mark_null);
    RUN_TEST(test_clock_measure_invalid);
    RUN_TEST(test_tsc_mark_unmark);

    printf("\nResults: %d/%d tests passed\n", tests_passed, tests_run);
    if (tests_passed != tests_run) {
        printf("SOME TESTS FAILED\n");
        return 1;
    }
    printf("ALL TESTS PASSED\n");
    return 0;
}
