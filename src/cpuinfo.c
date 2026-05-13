#define _GNU_SOURCE
#include "cpuinfo.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __linux__
#include <unistd.h>
#endif

static int g_last_error = ERR_OK;

static const char* error_messages[] = {
    "No error",
    "CPUID instruction not supported",
    "RDTSC instruction not supported",
    "Memory allocation failed",
    "File open failed",
    "Bad file format",
    "Not implemented",
    "Unknown CPU",
    "RDMSR not supported",
    "No MSR driver",
    "No permissions",
    "",
    "Bad handle",
    "",
    "",
    "",
    "Invalid range",
    "Not found",
    "",
    "Invalid request"
};

const char* cpuinfo_version(void)
{
    return CPUINFO_VERSION;
}

int cpuinfo_set_error(cpu_error_t err)
{
    g_last_error = (int)err;
    return (int)err;
}

const char* cpuinfo_error(void)
{
    int idx = -g_last_error;
    if (idx < 0 || idx >= 20)
        return "Unknown error";
    return error_messages[idx];
}

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#define CPUID_SUPPORTED 1
#else
#define CPUID_SUPPORTED 0
#endif

int cpuinfo_present(void)
{
#if CPUID_SUPPORTED
    return 1;
#else
    return 0;
#endif
}

void cpuinfo_exec(uint32_t leaf, uint32_t regs[NUM_REGISTERS])
{
#if CPUID_SUPPORTED
#ifdef _MSC_VER
    int info[4];
    __cpuid(info, (int)leaf);
    regs[REG_EAX] = (uint32_t)info[0];
    regs[REG_EBX] = (uint32_t)info[1];
    regs[REG_ECX] = (uint32_t)info[2];
    regs[REG_EDX] = (uint32_t)info[3];
#else
    __asm__ volatile(
        "cpuid"
        : "=a"(regs[REG_EAX]), "=b"(regs[REG_EBX]),
          "=c"(regs[REG_ECX]), "=d"(regs[REG_EDX])
        : "a"(leaf), "c"(0)
    );
#endif
#else
    (void)leaf;
    memset(regs, 0, sizeof(uint32_t) * NUM_REGISTERS);
#endif
}

void cpuinfo_exec_ext(uint32_t regs[NUM_REGISTERS])
{
#if CPUID_SUPPORTED
#ifdef _MSC_VER
    int info[4];
    __cpuidex(info, (int)regs[REG_EAX], (int)regs[REG_ECX]);
    regs[REG_EAX] = (uint32_t)info[0];
    regs[REG_EBX] = (uint32_t)info[1];
    regs[REG_ECX] = (uint32_t)info[2];
    regs[REG_EDX] = (uint32_t)info[3];
#else
    __asm__ volatile(
        "cpuid"
        : "=a"(regs[REG_EAX]), "=b"(regs[REG_EBX]),
          "=c"(regs[REG_ECX]), "=d"(regs[REG_EDX])
        : "a"(regs[REG_EAX]), "b"(regs[REG_EBX]),
          "c"(regs[REG_ECX]), "d"(regs[REG_EDX])
    );
#endif
#else
    memset(regs, 0, sizeof(uint32_t) * NUM_REGISTERS);
#endif
}

int cpuinfo_get_raw_data(struct cpu_raw_data_t* raw)
{
    int i;
    uint32_t regs[NUM_REGISTERS];

    if (!raw)
        return cpuinfo_set_error(ERR_REQUEST);

    memset(raw, 0, sizeof(struct cpu_raw_data_t));

    if (!cpuinfo_present())
        return cpuinfo_set_error(ERR_NO_CPUID);

    cpuinfo_exec(0, regs);
    int max_basic = (int)regs[REG_EAX];
    if (max_basic > MAX_BASIC_LEAVES - 1)
        max_basic = MAX_BASIC_LEAVES - 1;

    for (i = 0; i <= max_basic; i++) {
        cpuinfo_exec((uint32_t)i, raw->basic[i]);
    }

    cpuinfo_exec(0x80000000, regs);
    int max_ext = (int)(regs[REG_EAX] - 0x80000000);
    if (max_ext > MAX_EXT_LEAVES - 1)
        max_ext = MAX_EXT_LEAVES - 1;
    if (max_ext < 0)
        max_ext = 0;

    for (i = 0; i <= max_ext; i++) {
        cpuinfo_exec(0x80000000 + (uint32_t)i, raw->ext[i]);
    }

    return (int)ERR_OK;
}

cpu_vendor_t cpuinfo_detect_vendor(const char* vendor_str)
{
    struct { const char* str; cpu_vendor_t vendor; } known[] = {
        { "GenuineIntel", VENDOR_INTEL },
        { "AuthenticAMD", VENDOR_AMD },
        { "CyrixInstead", VENDOR_CYRIX },
        { "NexGenDriven", VENDOR_NEXGEN },
        { "GenuineTMx86", VENDOR_TRANSMETA },
        { "TransmetaCPU", VENDOR_TRANSMETA },
        { "UMC UMC UMC ", VENDOR_UMC },
        { "CentaurHauls", VENDOR_CENTAUR },
        { "RiseRiseRise", VENDOR_RISE },
        { "SiS SiS SiS ", VENDOR_SIS },
        { "Geode by NSC", VENDOR_NSC },
        { "HygonGenuine", VENDOR_HYGON },
        { "  Shanghai  ", VENDOR_CENTAUR },
    };
    int count = (int)(sizeof(known) / sizeof(known[0]));
    for (int i = 0; i < count; i++) {
        if (strncmp(vendor_str, known[i].str, 12) == 0)
            return known[i].vendor;
    }
    return VENDOR_UNKNOWN;
}

const char* cpuinfo_vendor_str(cpu_vendor_t vendor)
{
    static const char* names[] = {
        "Intel", "AMD", "Cyrix", "NexGen", "Transmeta",
        "UMC", "Centaur", "Rise", "SiS", "NSC", "Hygon"
    };
    if (vendor >= 0 && vendor < NUM_VENDORS)
        return names[vendor];
    return "unknown";
}

const char* cpuinfo_arch_str(cpu_arch_t arch)
{
    switch (arch) {
        case ARCH_X86: return "x86";
        case ARCH_ARM: return "ARM";
        default: return "unknown";
    }
}

static const char* feature_names[] = {
    "fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce",
    "cx8", "apic", "mtrr", "sep", "pge", "mca", "cmov", "pat",
    "pse36", "pn", "clflush", "dts", "acpi", "mmx", "fxsr", "sse",
    "sse2", "ss", "ht", "tm", "ia64", "pbe",
    "sse3", "pclmul", "dts64", "monitor", "ds_cpl", "vmx", "smx", "est",
    "tm2", "ssse3", "cid", "cx16", "xtpr", "pdcm", "dca", "sse4_1",
    "sse4_2", "syscall", "xd", "movbe", "popcnt", "aes", "xsave", "osxsave",
    "avx", "mmxext", "3dnow", "3dnowext", "nx", "fxsr_opt", "rdtscp", "lm",
    "lahf_lm", "cmp_legacy", "svm", "abm", "misalignsse", "sse4a",
    "3dnowprefetch", "osvw", "ibs", "skinit", "wdt", "constant_tsc",
    "xop", "fma3", "fma4", "tbm", "f16c", "rdrand", "x2apic", "cpb",
    "aperfmperf", "avx2", "bmi1", "bmi2", "hle", "rtm",
    "avx512f", "avx512dq", "avx512pf", "avx512er", "avx512cd", "sha_ni",
    "avx512bw", "avx512vl", "sgx", "rdseed", "adx", "avx512vnni",
    "avx512vbmi", "avx512vbmi2", "hypervisor",
    "fsgsbase", "erms", "invpcid", "vaes", "vpclmulqdq", "avx512bitalg"
};

const char* cpuinfo_feature_str(cpu_feature_t feature)
{
    if (feature >= 0 && feature < NUM_FEATURES)
        return feature_names[feature];
    return "";
}

void cpuinfo_init_id(struct cpu_id_t* id)
{
    memset(id, 0, sizeof(struct cpu_id_t));
    id->architecture = ARCH_UNKNOWN;
    id->vendor = VENDOR_UNKNOWN;
    id->l1_data_cache = -1;
    id->l1_instruction_cache = -1;
    id->l2_cache = -1;
    id->l3_cache = -1;
    id->l4_cache = -1;
    id->l1_data_assoc = -1;
    id->l1_instruction_assoc = -1;
    id->l2_assoc = -1;
    id->l3_assoc = -1;
    id->l4_assoc = -1;
    id->l1_data_cacheline = -1;
    id->l1_instruction_cacheline = -1;
    id->l2_cacheline = -1;
    id->l3_cacheline = -1;
    id->l4_cacheline = -1;
    id->l1_data_instances = -1;
    id->l1_instruction_instances = -1;
    id->l2_instances = -1;
    id->l3_instances = -1;
    id->l4_instances = -1;
    id->sse_size = -1;
}

static void extract_vendor_string(struct cpu_raw_data_t* raw, char* out)
{
    uint32_t* r = raw->basic[0];
    memcpy(out + 0, &r[REG_EBX], 4);
    memcpy(out + 4, &r[REG_EDX], 4);
    memcpy(out + 8, &r[REG_ECX], 4);
    out[12] = '\0';
}

static void extract_brand_string(struct cpu_raw_data_t* raw, char* out)
{
    uint32_t* leaves[3] = { raw->ext[2], raw->ext[3], raw->ext[4] };
    for (int i = 0; i < 3; i++) {
        memcpy(out + i * 16 + 0,  &leaves[i][REG_EAX], 4);
        memcpy(out + i * 16 + 4,  &leaves[i][REG_EBX], 4);
        memcpy(out + i * 16 + 8,  &leaves[i][REG_ECX], 4);
        memcpy(out + i * 16 + 12, &leaves[i][REG_EDX], 4);
    }
    out[48] = '\0';
    /* trim trailing spaces */
    int len = (int)strlen(out);
    while (len > 0 && out[len - 1] == ' ')
        out[--len] = '\0';
}

static void extract_family_model(struct cpu_raw_data_t* raw, struct cpu_id_t* id)
{
    uint32_t eax = raw->basic[1][REG_EAX];
    id->family   = (int32_t)((eax >> 8) & 0xF);
    id->model    = (int32_t)((eax >> 4) & 0xF);
    id->stepping = (int32_t)(eax & 0xF);

    int32_t ext_fam = (int32_t)((eax >> 20) & 0xFF);
    int32_t ext_mod = (int32_t)((eax >> 16) & 0xF);

    if (id->family == 0xF)
        id->ext_family = id->family + ext_fam;
    else
        id->ext_family = id->family;

    if (id->family == 0xF || id->family == 0x6)
        id->ext_model = (ext_mod << 4) | id->model;
    else
        id->ext_model = id->model;
}

struct feat_map {
    unsigned bit;
    cpu_feature_t feat;
};

static void match_feature_bits(const struct feat_map* table, int count,
                               uint32_t reg, struct cpu_id_t* id)
{
    for (int i = 0; i < count; i++) {
        if (reg & (1u << table[i].bit))
            id->flags[table[i].feat] = 1;
    }
}

static void detect_features(struct cpu_raw_data_t* raw, struct cpu_id_t* id)
{
    static const struct feat_map leaf1_edx[] = {
        {  0, FEAT_FPU },    {  1, FEAT_VME },     {  2, FEAT_DE },
        {  3, FEAT_PSE },    {  4, FEAT_TSC },     {  5, FEAT_MSR },
        {  6, FEAT_PAE },    {  7, FEAT_MCE },     {  8, FEAT_CX8 },
        {  9, FEAT_APIC },   { 11, FEAT_SEP },     { 12, FEAT_MTRR },
        { 13, FEAT_PGE },    { 14, FEAT_MCA },     { 15, FEAT_CMOV },
        { 16, FEAT_PAT },    { 17, FEAT_PSE36 },   { 18, FEAT_PN },
        { 19, FEAT_CLFLUSH },{ 21, FEAT_DTS },     { 22, FEAT_ACPI },
        { 23, FEAT_MMX },    { 24, FEAT_FXSR },    { 25, FEAT_SSE },
        { 26, FEAT_SSE2 },   { 27, FEAT_SS },      { 28, FEAT_HT },
        { 29, FEAT_TM },     { 30, FEAT_IA64 },    { 31, FEAT_PBE },
    };

    static const struct feat_map leaf1_ecx[] = {
        {  0, FEAT_SSE3 },   {  1, FEAT_PCLMUL },  {  2, FEAT_DTS64 },
        {  3, FEAT_MONITOR },{  4, FEAT_DS_CPL },   {  5, FEAT_VMX },
        {  6, FEAT_SMX },    {  7, FEAT_EST },      {  8, FEAT_TM2 },
        {  9, FEAT_SSSE3 },  { 10, FEAT_CID },     { 13, FEAT_CX16 },
        { 14, FEAT_XTPR },   { 15, FEAT_PDCM },    { 18, FEAT_DCA },
        { 19, FEAT_SSE41 },  { 20, FEAT_SSE42 },   { 22, FEAT_MOVBE },
        { 23, FEAT_POPCNT }, { 25, FEAT_AES },      { 26, FEAT_XSAVE },
        { 27, FEAT_OSXSAVE },{ 28, FEAT_AVX },     { 31, FEAT_HYPERVISOR },
    };

    static const struct feat_map ext1_edx[] = {
        { 11, FEAT_SYSCALL },{ 20, FEAT_NX },      { 22, FEAT_MMXEXT },
        { 25, FEAT_FXSR_OPT },{ 27, FEAT_RDTSCP }, { 29, FEAT_LM },
        { 30, FEAT_3DNOWEXT },{ 31, FEAT_3DNOW },
    };

    static const struct feat_map ext1_ecx[] = {
        {  0, FEAT_LAHF_LM },  {  1, FEAT_CMP_LEGACY },
        {  2, FEAT_SVM },      {  3, FEAT_ABM },
        {  6, FEAT_SSE4A },    {  7, FEAT_MISALIGNSSE },
        {  8, FEAT_3DNOW_PREFETCH },{ 9, FEAT_OSVW },
        { 10, FEAT_IBS },      { 12, FEAT_SKINIT },
        { 13, FEAT_WDT },      { 11, FEAT_XOP },
        { 16, FEAT_FMA4 },     { 21, FEAT_TBM },
    };

    match_feature_bits(leaf1_edx, (int)(sizeof(leaf1_edx)/sizeof(leaf1_edx[0])),
                       raw->basic[1][REG_EDX], id);
    match_feature_bits(leaf1_ecx, (int)(sizeof(leaf1_ecx)/sizeof(leaf1_ecx[0])),
                       raw->basic[1][REG_ECX], id);

    if (raw->ext[0][REG_EAX] >= 0x80000001) {
        match_feature_bits(ext1_edx, (int)(sizeof(ext1_edx)/sizeof(ext1_edx[0])),
                           raw->ext[1][REG_EDX], id);
        match_feature_bits(ext1_ecx, (int)(sizeof(ext1_ecx)/sizeof(ext1_ecx[0])),
                           raw->ext[1][REG_ECX], id);
    }

    /* leaf 7 features */
    if (raw->basic[0][REG_EAX] >= 7) {
        uint32_t ebx7 = raw->basic[7][REG_EBX];
        uint32_t ecx7 = raw->basic[7][REG_ECX];

        static const struct feat_map leaf7_ebx[] = {
            {  0, FEAT_FSGSBASE }, {  3, FEAT_BMI1 },    {  5, FEAT_AVX2 },
            {  8, FEAT_BMI2 },    {  4, FEAT_HLE },     { 11, FEAT_RTM },
            {  9, FEAT_ERMS },    { 10, FEAT_INVPCID },
            { 16, FEAT_AVX512F }, { 17, FEAT_AVX512DQ },{ 18, FEAT_RDSEED },
            { 19, FEAT_ADX },     { 26, FEAT_AVX512PF },{ 27, FEAT_AVX512ER },
            { 28, FEAT_AVX512CD },{ 29, FEAT_SHA },     { 30, FEAT_AVX512BW },
            { 31, FEAT_AVX512VL },{ 2, FEAT_SGX },
        };
        static const struct feat_map leaf7_ecx[] = {
            {  1, FEAT_AVX512VBMI }, { 6, FEAT_AVX512VBMI2 },
            {  8, FEAT_VAES },       { 10, FEAT_VPCLMULQDQ },
            { 11, FEAT_AVX512VNNI }, { 12, FEAT_AVX512BITALG },
        };

        match_feature_bits(leaf7_ebx, (int)(sizeof(leaf7_ebx)/sizeof(leaf7_ebx[0])),
                           ebx7, id);
        match_feature_bits(leaf7_ecx, (int)(sizeof(leaf7_ecx)/sizeof(leaf7_ecx[0])),
                           ecx7, id);
    }

    /* FMA3 from leaf 1 ECX bit 12 */
    if (raw->basic[1][REG_ECX] & (1u << 12))
        id->flags[FEAT_FMA3] = 1;
    /* F16C from leaf 1 ECX bit 29 */
    if (raw->basic[1][REG_ECX] & (1u << 29))
        id->flags[FEAT_F16C] = 1;
    /* RDRAND from leaf 1 ECX bit 30 */
    if (raw->basic[1][REG_ECX] & (1u << 30))
        id->flags[FEAT_RDRAND] = 1;
    /* x2APIC from leaf 1 ECX bit 21 */
    if (raw->basic[1][REG_ECX] & (1u << 21))
        id->flags[FEAT_X2APIC] = 1;

    /* AMD-specific constant_tsc: from cpuid 80000007 EDX bit 8 */
    if (raw->ext[0][REG_EAX] >= 0x80000007) {
        if (raw->ext[7][REG_EDX] & (1u << 8))
            id->flags[FEAT_CONSTANT_TSC] = 1;
    }

    /* CPB: from cpuid 80000007 EDX bit 9 */
    if (raw->ext[0][REG_EAX] >= 0x80000007) {
        if (raw->ext[7][REG_EDX] & (1u << 9))
            id->flags[FEAT_CPB] = 1;
    }

    /* APERFMPERF from 80000007 EDX bit 7 */
    if (raw->ext[0][REG_EAX] >= 0x80000007) {
        if (raw->ext[7][REG_EDX] & (1u << 7))
            id->flags[FEAT_APERFMPERF] = 1;
    }
}

static void extract_core_count(struct cpu_raw_data_t* raw, struct cpu_id_t* id)
{
    id->num_cores = 1;
    id->num_logical_cpus = 1;

    if (id->flags[FEAT_HT]) {
        id->num_logical_cpus = (int32_t)((raw->basic[1][REG_EBX] >> 16) & 0xFF);
        if (id->num_logical_cpus == 0)
            id->num_logical_cpus = 1;
    }

    if (id->vendor == VENDOR_INTEL) {
        if (raw->basic[0][REG_EAX] >= 4) {
            id->num_cores = (int32_t)(((raw->intel_cache[0][REG_EAX] >> 26) & 0x3F) + 1);
        }
    } else if (id->vendor == VENDOR_AMD) {
        if (raw->ext[0][REG_EAX] >= 0x80000008) {
            id->num_cores = (int32_t)((raw->ext[8][REG_ECX] & 0xFF) + 1);
        }
    }

    if (id->num_cores > id->num_logical_cpus)
        id->num_cores = id->num_logical_cpus;

    id->total_logical_cpus = id->num_logical_cpus;
}

int cpuinfo_identify(struct cpu_raw_data_t* raw, struct cpu_id_t* id)
{
    struct cpu_raw_data_t local_raw;

    if (!id)
        return cpuinfo_set_error(ERR_REQUEST);

    cpuinfo_init_id(id);

    if (!raw) {
        int err = cpuinfo_get_raw_data(&local_raw);
        if (err != ERR_OK)
            return err;
        raw = &local_raw;
    }

    if (raw->basic[0][REG_EAX] == 0 && raw->basic[0][REG_EBX] == 0)
        return cpuinfo_set_error(ERR_NO_CPUID);

    id->architecture = ARCH_X86;

    extract_vendor_string(raw, id->vendor_str);
    id->vendor = cpuinfo_detect_vendor(id->vendor_str);

    extract_family_model(raw, id);

    if (raw->ext[0][REG_EAX] >= 0x80000004) {
        extract_brand_string(raw, id->brand_str);
    }

    detect_features(raw, id);
    extract_core_count(raw, id);

    return (int)ERR_OK;
}

int cpuinfo_get_total_cpus(void)
{
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwNumberOfProcessors;
#elif defined(__linux__)
    int n = (int)sysconf(_SC_NPROCESSORS_ONLN);
    return n > 0 ? n : 1;
#else
    return 1;
#endif
}
