#ifndef CACHE_H
#define CACHE_H

#include "cpuinfo.h"

int cpuinfo_decode_caches(struct cpu_raw_data_t* raw, struct cpu_id_t* id);

#endif /* CACHE_H */
