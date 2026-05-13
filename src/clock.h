#ifndef CLOCK_H
#define CLOCK_H

#include "cpuinfo.h"

struct cpu_mark_t {
    uint64_t tsc;
    uint64_t sys_clock;
};

int cpu_clock_by_os(void);
int cpu_clock_by_tsc(struct cpu_raw_data_t* raw);
void cpu_tsc_mark(struct cpu_mark_t* mark);
void cpu_tsc_unmark(struct cpu_mark_t* mark);
int cpu_clock_by_mark(struct cpu_mark_t* mark);
int cpu_clock_measure(int millis);

#endif /* CLOCK_H */
