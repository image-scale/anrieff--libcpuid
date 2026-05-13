#include "clock.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static void sys_precise_clock(uint64_t* result)
{
    LARGE_INTEGER freq, counter;
    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&freq);
    *result = (uint64_t)((double)counter.QuadPart * 1000000.0 / (double)freq.QuadPart);
}
#else
#include <sys/time.h>
static void sys_precise_clock(uint64_t* result)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *result = (uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec;
}
#endif

static inline uint64_t rdtsc_value(void)
{
#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#ifdef _MSC_VER
    return __rdtsc();
#else
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
#endif
#else
    return 0;
#endif
}

int cpu_clock_by_os(void)
{
#ifdef _WIN32
    HKEY key;
    DWORD result;
    DWORD size = 4;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"),
        0, KEY_READ, &key) != ERROR_SUCCESS)
        return -1;
    if (RegQueryValueEx(key, TEXT("~MHz"), NULL, NULL,
        (LPBYTE)&result, (LPDWORD)&size) != ERROR_SUCCESS) {
        RegCloseKey(key);
        return -1;
    }
    RegCloseKey(key);
    return (int)result;
#elif defined(__linux__)
    FILE* f;
    char line[1024], *s;
    int result;
    f = fopen("/proc/cpuinfo", "rt");
    if (!f) return -1;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "cpu MHz", 7) == 0) {
            s = strchr(line, ':');
            if (s && sscanf(s, ":%d.", &result) == 1) {
                fclose(f);
                return result;
            }
        }
    }
    fclose(f);
    return -1;
#else
    return -1;
#endif
}

void cpu_tsc_mark(struct cpu_mark_t* mark)
{
    if (!mark) return;
    mark->tsc = rdtsc_value();
    sys_precise_clock(&mark->sys_clock);
}

void cpu_tsc_unmark(struct cpu_mark_t* mark)
{
    if (!mark) return;
    struct cpu_mark_t end;
    end.tsc = rdtsc_value();
    sys_precise_clock(&end.sys_clock);
    mark->tsc = end.tsc - mark->tsc;
    mark->sys_clock = end.sys_clock - mark->sys_clock;
}

int cpu_clock_by_mark(struct cpu_mark_t* mark)
{
    if (!mark) return -1;
    if ((mark->tsc >> 63) != 0 || (mark->sys_clock >> 63) != 0)
        return -1;
    if (mark->sys_clock == 0)
        return -1;
    uint64_t result = mark->tsc / mark->sys_clock;
    if (result > 0x7FFFFFFF)
        return -1;
    return (int)result;
}

static volatile int busy_sink = 0;

static void busy_loop(int cycles)
{
    volatile int s = 0;
    for (int i = 0; i < cycles; i++)
        for (int j = 0; j < 1000; j++)
            s += j * i;
    busy_sink = s;
}

int cpu_clock_measure(int millis)
{
    if (millis < 1) return -1;

    struct cpu_mark_t mark;
    uint64_t target_us = (uint64_t)millis * 1000;
    uint64_t start, now;
    int cycles = 1;

    cpu_tsc_mark(&mark);
    sys_precise_clock(&start);

    while (1) {
        busy_loop(cycles);
        sys_precise_clock(&now);
        if (now - start >= target_us)
            break;
        if (now - start < target_us / 8)
            cycles *= 2;
    }

    cpu_tsc_unmark(&mark);
    return cpu_clock_by_mark(&mark);
}

int cpu_clock_by_tsc(struct cpu_raw_data_t* raw)
{
    if (!raw)
        return -1;

    if (raw->basic[0][REG_EAX] < 0x15)
        return -1;

    uint32_t denominator = raw->basic[0x15][REG_EAX];
    uint32_t numerator = raw->basic[0x15][REG_EBX];
    uint32_t crystal_hz = raw->basic[0x15][REG_ECX];

    if (numerator == 0 || denominator == 0)
        return -1;

    uint32_t nominal_freq_khz = crystal_hz / 1000;

    if (nominal_freq_khz == 0 && raw->basic[0][REG_EAX] >= 0x16) {
        uint32_t base_freq_mhz = raw->basic[0x16][REG_EAX] & 0xFFFF;
        if (base_freq_mhz > 0)
            nominal_freq_khz = base_freq_mhz * 1000 * denominator / numerator;
    }

    if (nominal_freq_khz == 0)
        return -1;

    return (int)((uint64_t)nominal_freq_khz * numerator / denominator / 1000);
}
