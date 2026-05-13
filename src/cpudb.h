#ifndef CPUDB_H
#define CPUDB_H

#include "cpuinfo.h"

struct codename_entry_t {
    int family, model, stepping, ext_family, ext_model;
    int num_cores, l2_cache, l3_cache;
    const char* brand_pattern;
    int brand_score;
    const char* codename;
    const char* technology;
};

int cpudb_identify_codename(struct cpu_id_t* id);
void cpudb_get_list(cpu_vendor_t vendor, const char*** names, int* count);

#endif /* CPUDB_H */
