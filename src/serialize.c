#define _GNU_SOURCE
#include "cpuinfo.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

int cpuinfo_serialize_raw(struct cpu_raw_data_t* raw, const char* filename)
{
    FILE* f;
    int i;

    if (!raw)
        return cpuinfo_set_error(ERR_REQUEST);

    if (!filename || filename[0] == '\0')
        f = stdout;
    else
        f = fopen(filename, "w");

    if (!f)
        return cpuinfo_set_error(ERR_OPEN);

    fprintf(f, "version=%s\n", CPUINFO_VERSION);

    for (i = 0; i < MAX_BASIC_LEAVES; i++) {
        fprintf(f, "basic_cpuid[%d]=%08" PRIx32 " %08" PRIx32 " %08" PRIx32 " %08" PRIx32 "\n",
                i, raw->basic[i][REG_EAX], raw->basic[i][REG_EBX],
                raw->basic[i][REG_ECX], raw->basic[i][REG_EDX]);
    }

    for (i = 0; i < MAX_EXT_LEAVES; i++) {
        fprintf(f, "ext_cpuid[%d]=%08" PRIx32 " %08" PRIx32 " %08" PRIx32 " %08" PRIx32 "\n",
                i, raw->ext[i][REG_EAX], raw->ext[i][REG_EBX],
                raw->ext[i][REG_ECX], raw->ext[i][REG_EDX]);
    }

    for (i = 0; i < MAX_INTEL_CACHE_LEAVES; i++) {
        fprintf(f, "intel_cache[%d]=%08" PRIx32 " %08" PRIx32 " %08" PRIx32 " %08" PRIx32 "\n",
                i, raw->intel_cache[i][REG_EAX], raw->intel_cache[i][REG_EBX],
                raw->intel_cache[i][REG_ECX], raw->intel_cache[i][REG_EDX]);
    }

    for (i = 0; i < MAX_INTEL_TOPO_LEAVES; i++) {
        fprintf(f, "intel_topology[%d]=%08" PRIx32 " %08" PRIx32 " %08" PRIx32 " %08" PRIx32 "\n",
                i, raw->intel_topology[i][REG_EAX], raw->intel_topology[i][REG_EBX],
                raw->intel_topology[i][REG_ECX], raw->intel_topology[i][REG_EDX]);
    }

    if (f != stdout)
        fclose(f);

    return (int)ERR_OK;
}

static int parse_raw_line(const char* line, struct cpu_raw_data_t* raw)
{
    int idx;
    uint32_t a, b, c, d;

    if (sscanf(line, "basic_cpuid[%d]=%" SCNx32 " %" SCNx32 " %" SCNx32 " %" SCNx32,
               &idx, &a, &b, &c, &d) == 5) {
        if (idx >= 0 && idx < MAX_BASIC_LEAVES) {
            raw->basic[idx][REG_EAX] = a;
            raw->basic[idx][REG_EBX] = b;
            raw->basic[idx][REG_ECX] = c;
            raw->basic[idx][REG_EDX] = d;
        }
        return 1;
    }

    if (sscanf(line, "ext_cpuid[%d]=%" SCNx32 " %" SCNx32 " %" SCNx32 " %" SCNx32,
               &idx, &a, &b, &c, &d) == 5) {
        if (idx >= 0 && idx < MAX_EXT_LEAVES) {
            raw->ext[idx][REG_EAX] = a;
            raw->ext[idx][REG_EBX] = b;
            raw->ext[idx][REG_ECX] = c;
            raw->ext[idx][REG_EDX] = d;
        }
        return 1;
    }

    if (sscanf(line, "intel_cache[%d]=%" SCNx32 " %" SCNx32 " %" SCNx32 " %" SCNx32,
               &idx, &a, &b, &c, &d) == 5) {
        if (idx >= 0 && idx < MAX_INTEL_CACHE_LEAVES) {
            raw->intel_cache[idx][REG_EAX] = a;
            raw->intel_cache[idx][REG_EBX] = b;
            raw->intel_cache[idx][REG_ECX] = c;
            raw->intel_cache[idx][REG_EDX] = d;
        }
        return 1;
    }

    if (sscanf(line, "intel_topology[%d]=%" SCNx32 " %" SCNx32 " %" SCNx32 " %" SCNx32,
               &idx, &a, &b, &c, &d) == 5) {
        if (idx >= 0 && idx < MAX_INTEL_TOPO_LEAVES) {
            raw->intel_topology[idx][REG_EAX] = a;
            raw->intel_topology[idx][REG_EBX] = b;
            raw->intel_topology[idx][REG_ECX] = c;
            raw->intel_topology[idx][REG_EDX] = d;
        }
        return 1;
    }

    return 0;
}

int cpuinfo_deserialize_raw(struct cpu_raw_data_t* raw, const char* filename)
{
    FILE* f;
    char line[256];

    if (!raw)
        return cpuinfo_set_error(ERR_REQUEST);

    memset(raw, 0, sizeof(struct cpu_raw_data_t));

    if (!filename || filename[0] == '\0')
        f = stdin;
    else
        f = fopen(filename, "r");

    if (!f)
        return cpuinfo_set_error(ERR_OPEN);

    while (fgets(line, sizeof(line), f)) {
        /* strip newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        if (line[0] == '\0' || line[0] == '#')
            continue;

        parse_raw_line(line, raw);
    }

    if (f != stdin)
        fclose(f);

    return (int)ERR_OK;
}
