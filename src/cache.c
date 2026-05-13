#include "cache.h"
#include <string.h>

#define EXTRACT_BITS(reg, hi, lo) \
    (((reg) >> (lo)) & ((1u << ((hi) - (lo) + 1)) - 1))

static void decode_intel_deterministic(struct cpu_raw_data_t* raw, struct cpu_id_t* id)
{
    for (int i = 0; i < MAX_INTEL_CACHE_LEAVES; i++) {
        uint32_t eax = raw->intel_cache[i][REG_EAX];
        uint32_t ebx = raw->intel_cache[i][REG_EBX];
        uint32_t ecx = raw->intel_cache[i][REG_ECX];

        uint32_t cache_type = EXTRACT_BITS(eax, 4, 0);
        uint32_t cache_level = EXTRACT_BITS(eax, 7, 5);

        if (cache_type == 0 || cache_level == 0)
            break;

        uint32_t ways = EXTRACT_BITS(ebx, 31, 22) + 1;
        uint32_t partitions = EXTRACT_BITS(ebx, 21, 12) + 1;
        uint32_t linesize = EXTRACT_BITS(ebx, 11, 0) + 1;
        uint32_t sets = ecx + 1;
        int size = (int)(ways * partitions * linesize * sets / 1024);
        int assoc = (int)ways;
        int line = (int)linesize;

        uint32_t num_sharing = EXTRACT_BITS(eax, 25, 14) + 1;
        int instances = -1;
        if (id->num_logical_cpus > 0 && num_sharing > 0) {
            instances = (int)((uint32_t)id->num_logical_cpus / num_sharing);
            if (instances < 1) instances = 1;
        }

        if (cache_level == 1 && cache_type == 1) {
            id->l1_data_cache = size;
            id->l1_data_assoc = assoc;
            id->l1_data_cacheline = line;
            id->l1_data_instances = instances;
        } else if (cache_level == 1 && cache_type == 2) {
            id->l1_instruction_cache = size;
            id->l1_instruction_assoc = assoc;
            id->l1_instruction_cacheline = line;
            id->l1_instruction_instances = instances;
        } else if (cache_level == 2 && (cache_type == 3 || cache_type == 1)) {
            id->l2_cache = size;
            id->l2_assoc = assoc;
            id->l2_cacheline = line;
            id->l2_instances = instances;
        } else if (cache_level == 3 && cache_type == 3) {
            id->l3_cache = size;
            id->l3_assoc = assoc;
            id->l3_cacheline = line;
            id->l3_instances = instances;
        } else if (cache_level == 4 && cache_type == 3) {
            id->l4_cache = size;
            id->l4_assoc = assoc;
            id->l4_cacheline = line;
            id->l4_instances = instances;
        }
    }
}

static void decode_amd_legacy_cache(struct cpu_raw_data_t* raw, struct cpu_id_t* id)
{
    if (raw->ext[0][REG_EAX] < 0x80000005)
        return;

    uint32_t ecx5 = raw->ext[5][REG_ECX];
    uint32_t edx5 = raw->ext[5][REG_EDX];

    if (ecx5) {
        id->l1_data_cache = (int)EXTRACT_BITS(ecx5, 31, 24);
        id->l1_data_assoc = (int)EXTRACT_BITS(ecx5, 23, 16);
        id->l1_data_cacheline = (int)EXTRACT_BITS(ecx5, 7, 0);
    }
    if (edx5) {
        id->l1_instruction_cache = (int)EXTRACT_BITS(edx5, 31, 24);
        id->l1_instruction_assoc = (int)EXTRACT_BITS(edx5, 23, 16);
        id->l1_instruction_cacheline = (int)EXTRACT_BITS(edx5, 7, 0);
    }

    if (raw->ext[0][REG_EAX] < 0x80000006)
        return;

    uint32_t ecx6 = raw->ext[6][REG_ECX];
    uint32_t edx6 = raw->ext[6][REG_EDX];

    if (ecx6) {
        id->l2_cache = (int)EXTRACT_BITS(ecx6, 31, 16);
        int assoc_encoded = (int)EXTRACT_BITS(ecx6, 15, 12);
        static const int assoc_table[] = {
            0, 1, 2, 0, 4, 0, 8, 0, 16, 0, 32, 48, 64, 96, 128, 255
        };
        if (assoc_encoded >= 0 && assoc_encoded <= 15)
            id->l2_assoc = assoc_table[assoc_encoded];
        id->l2_cacheline = (int)EXTRACT_BITS(ecx6, 7, 0);
    }

    if (edx6) {
        id->l3_cache = (int)(EXTRACT_BITS(edx6, 31, 18) * 512);
        int assoc_encoded = (int)EXTRACT_BITS(edx6, 15, 12);
        static const int assoc_table[] = {
            0, 1, 2, 0, 4, 0, 8, 0, 16, 0, 32, 48, 64, 96, 128, 255
        };
        if (assoc_encoded >= 0 && assoc_encoded <= 15)
            id->l3_assoc = assoc_table[assoc_encoded];
        id->l3_cacheline = (int)EXTRACT_BITS(edx6, 7, 0);
    }
}

int cpuinfo_decode_caches(struct cpu_raw_data_t* raw, struct cpu_id_t* id)
{
    if (!raw || !id)
        return cpuinfo_set_error(ERR_REQUEST);

    int has_intel_det = 0;
    for (int i = 0; i < MAX_INTEL_CACHE_LEAVES; i++) {
        if (raw->intel_cache[i][REG_EAX] != 0) {
            has_intel_det = 1;
            break;
        }
    }

    if (has_intel_det) {
        decode_intel_deterministic(raw, id);
    } else if (id->vendor == VENDOR_AMD || id->vendor == VENDOR_HYGON) {
        decode_amd_legacy_cache(raw, id);
    }

    return (int)ERR_OK;
}
