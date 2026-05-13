#ifndef CPUINFO_H
#define CPUINFO_H

#include <stdint.h>
#include <stdbool.h>

#define CPUINFO_VERSION "0.1.0"

#define VENDOR_STRING_MAX   16
#define BRAND_STRING_MAX    64
#define CODENAME_MAX        64
#define TECHNOLOGY_MAX      16
#define MAX_BASIC_LEAVES    32
#define MAX_EXT_LEAVES      32
#define MAX_INTEL_CACHE_LEAVES 8
#define MAX_INTEL_TOPO_LEAVES  4
#define FEATURE_FLAGS_MAX   256
#define NUM_REGISTERS       4

typedef enum {
    REG_EAX = 0,
    REG_EBX,
    REG_ECX,
    REG_EDX
} cpu_register_t;

typedef enum {
    ARCH_X86 = 0,
    ARCH_ARM,
    NUM_ARCHITECTURES,
    ARCH_UNKNOWN = -1
} cpu_arch_t;

typedef enum {
    VENDOR_INTEL = 0,
    VENDOR_AMD,
    VENDOR_CYRIX,
    VENDOR_NEXGEN,
    VENDOR_TRANSMETA,
    VENDOR_UMC,
    VENDOR_CENTAUR,
    VENDOR_RISE,
    VENDOR_SIS,
    VENDOR_NSC,
    VENDOR_HYGON,
    NUM_VENDORS,
    VENDOR_UNKNOWN = -1
} cpu_vendor_t;

typedef enum {
    FEAT_FPU = 0,
    FEAT_VME,
    FEAT_DE,
    FEAT_PSE,
    FEAT_TSC,
    FEAT_MSR,
    FEAT_PAE,
    FEAT_MCE,
    FEAT_CX8,
    FEAT_APIC,
    FEAT_MTRR,
    FEAT_SEP,
    FEAT_PGE,
    FEAT_MCA,
    FEAT_CMOV,
    FEAT_PAT,
    FEAT_PSE36,
    FEAT_PN,
    FEAT_CLFLUSH,
    FEAT_DTS,
    FEAT_ACPI,
    FEAT_MMX,
    FEAT_FXSR,
    FEAT_SSE,
    FEAT_SSE2,
    FEAT_SS,
    FEAT_HT,
    FEAT_TM,
    FEAT_IA64,
    FEAT_PBE,
    FEAT_SSE3,
    FEAT_PCLMUL,
    FEAT_DTS64,
    FEAT_MONITOR,
    FEAT_DS_CPL,
    FEAT_VMX,
    FEAT_SMX,
    FEAT_EST,
    FEAT_TM2,
    FEAT_SSSE3,
    FEAT_CID,
    FEAT_CX16,
    FEAT_XTPR,
    FEAT_PDCM,
    FEAT_DCA,
    FEAT_SSE41,
    FEAT_SSE42,
    FEAT_SYSCALL,
    FEAT_XD,
    FEAT_MOVBE,
    FEAT_POPCNT,
    FEAT_AES,
    FEAT_XSAVE,
    FEAT_OSXSAVE,
    FEAT_AVX,
    FEAT_MMXEXT,
    FEAT_3DNOW,
    FEAT_3DNOWEXT,
    FEAT_NX,
    FEAT_FXSR_OPT,
    FEAT_RDTSCP,
    FEAT_LM,
    FEAT_LAHF_LM,
    FEAT_CMP_LEGACY,
    FEAT_SVM,
    FEAT_ABM,
    FEAT_MISALIGNSSE,
    FEAT_SSE4A,
    FEAT_3DNOW_PREFETCH,
    FEAT_OSVW,
    FEAT_IBS,
    FEAT_SKINIT,
    FEAT_WDT,
    FEAT_CONSTANT_TSC,
    FEAT_XOP,
    FEAT_FMA3,
    FEAT_FMA4,
    FEAT_TBM,
    FEAT_F16C,
    FEAT_RDRAND,
    FEAT_X2APIC,
    FEAT_CPB,
    FEAT_APERFMPERF,
    FEAT_AVX2,
    FEAT_BMI1,
    FEAT_BMI2,
    FEAT_HLE,
    FEAT_RTM,
    FEAT_AVX512F,
    FEAT_AVX512DQ,
    FEAT_AVX512PF,
    FEAT_AVX512ER,
    FEAT_AVX512CD,
    FEAT_SHA,
    FEAT_AVX512BW,
    FEAT_AVX512VL,
    FEAT_SGX,
    FEAT_RDSEED,
    FEAT_ADX,
    FEAT_AVX512VNNI,
    FEAT_AVX512VBMI,
    FEAT_AVX512VBMI2,
    FEAT_HYPERVISOR,
    FEAT_FSGSBASE,
    FEAT_ERMS,
    FEAT_INVPCID,
    FEAT_VAES,
    FEAT_VPCLMULQDQ,
    FEAT_AVX512BITALG,
    NUM_FEATURES
} cpu_feature_t;

typedef enum {
    ERR_OK        =  0,
    ERR_NO_CPUID  = -1,
    ERR_NO_RDTSC  = -2,
    ERR_NO_MEM    = -3,
    ERR_OPEN      = -4,
    ERR_BADFMT    = -5,
    ERR_NOT_IMPL  = -6,
    ERR_CPU_UNKN  = -7,
    ERR_NO_RDMSR  = -8,
    ERR_NO_DRIVER = -9,
    ERR_NO_PERMS  = -10,
    ERR_HANDLE    = -12,
    ERR_INVRANGE  = -16,
    ERR_NOT_FOUND = -17,
    ERR_REQUEST   = -19
} cpu_error_t;

struct cpu_raw_data_t {
    uint32_t basic[MAX_BASIC_LEAVES][NUM_REGISTERS];
    uint32_t ext[MAX_EXT_LEAVES][NUM_REGISTERS];
    uint32_t intel_cache[MAX_INTEL_CACHE_LEAVES][NUM_REGISTERS];
    uint32_t intel_topology[MAX_INTEL_TOPO_LEAVES][NUM_REGISTERS];
};

struct cpu_id_t {
    cpu_arch_t architecture;
    cpu_vendor_t vendor;
    char vendor_str[VENDOR_STRING_MAX];
    char brand_str[BRAND_STRING_MAX];
    int32_t family;
    int32_t model;
    int32_t stepping;
    int32_t ext_family;
    int32_t ext_model;
    int32_t num_cores;
    int32_t num_logical_cpus;
    int32_t total_logical_cpus;
    int32_t l1_data_cache;
    int32_t l1_instruction_cache;
    int32_t l2_cache;
    int32_t l3_cache;
    int32_t l4_cache;
    int32_t l1_data_assoc;
    int32_t l1_instruction_assoc;
    int32_t l2_assoc;
    int32_t l3_assoc;
    int32_t l4_assoc;
    int32_t l1_data_cacheline;
    int32_t l1_instruction_cacheline;
    int32_t l2_cacheline;
    int32_t l3_cacheline;
    int32_t l4_cacheline;
    int32_t l1_data_instances;
    int32_t l1_instruction_instances;
    int32_t l2_instances;
    int32_t l3_instances;
    int32_t l4_instances;
    int32_t sse_size;
    uint8_t flags[FEATURE_FLAGS_MAX];
    char cpu_codename[CODENAME_MAX];
    char technology[TECHNOLOGY_MAX];
};

const char* cpuinfo_version(void);
const char* cpuinfo_error(void);
int cpuinfo_set_error(cpu_error_t err);

int cpuinfo_present(void);
void cpuinfo_exec(uint32_t leaf, uint32_t regs[NUM_REGISTERS]);
void cpuinfo_exec_ext(uint32_t regs[NUM_REGISTERS]);

int cpuinfo_get_raw_data(struct cpu_raw_data_t* raw);
int cpuinfo_identify(struct cpu_raw_data_t* raw, struct cpu_id_t* id);
void cpuinfo_init_id(struct cpu_id_t* id);

cpu_vendor_t cpuinfo_detect_vendor(const char* vendor_str);
const char* cpuinfo_vendor_str(cpu_vendor_t vendor);
const char* cpuinfo_arch_str(cpu_arch_t arch);
const char* cpuinfo_feature_str(cpu_feature_t feature);

int cpuinfo_get_total_cpus(void);

#endif /* CPUINFO_H */
