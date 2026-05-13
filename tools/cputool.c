#include "cpuinfo.h"
#include "cpudb.h"
#include "cache.h"
#include "clock.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void usage(const char* prog)
{
    fprintf(stderr, "Usage: %s [OPTIONS]\n", prog);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  --load FILE    Load raw CPUID data from file\n");
    fprintf(stderr, "  --save FILE    Save raw CPUID data to file\n");
    fprintf(stderr, "  --report       Print full CPU report\n");
    fprintf(stderr, "  --vendorstr    Print vendor string\n");
    fprintf(stderr, "  --brandstr     Print brand string\n");
    fprintf(stderr, "  --family       Print CPU family\n");
    fprintf(stderr, "  --model        Print CPU model\n");
    fprintf(stderr, "  --stepping     Print CPU stepping\n");
    fprintf(stderr, "  --extfamily    Print extended family\n");
    fprintf(stderr, "  --extmodel     Print extended model\n");
    fprintf(stderr, "  --cores        Print number of cores\n");
    fprintf(stderr, "  --logical      Print number of logical CPUs\n");
    fprintf(stderr, "  --codename     Print CPU codename\n");
    fprintf(stderr, "  --technology   Print manufacturing technology\n");
    fprintf(stderr, "  --cache        Print L2 cache size (KB)\n");
    fprintf(stderr, "  --l1d-cache    Print L1 data cache (KB)\n");
    fprintf(stderr, "  --l1i-cache    Print L1 instruction cache (KB)\n");
    fprintf(stderr, "  --l2-cache     Print L2 cache size (KB)\n");
    fprintf(stderr, "  --l3-cache     Print L3 cache size (KB)\n");
    fprintf(stderr, "  --flags        Print CPU feature flags\n");
    fprintf(stderr, "  --clock        Print CPU clock (MHz, OS-reported)\n");
    fprintf(stderr, "  --clock-rdtsc  Print CPU clock (MHz, RDTSC-measured)\n");
    fprintf(stderr, "  --version      Print library version\n");
    fprintf(stderr, "  --help         Print this help\n");
}

static void print_report(struct cpu_id_t* id)
{
    printf("CPU Vendor: %s (%s)\n", id->vendor_str, cpuinfo_vendor_str(id->vendor));
    printf("Brand:      %s\n", id->brand_str);
    printf("Family:     %d\n", (int)id->family);
    printf("Model:      %d\n", (int)id->model);
    printf("Stepping:   %d\n", (int)id->stepping);
    printf("Ext.Family: %d\n", (int)id->ext_family);
    printf("Ext.Model:  %d (0x%02X)\n", (int)id->ext_model, (unsigned)id->ext_model);
    printf("Cores:      %d\n", (int)id->num_cores);
    printf("Logical:    %d\n", (int)id->num_logical_cpus);
    printf("Codename:   %s\n", id->cpu_codename);
    printf("Technology: %s\n", id->technology);
    printf("L1D Cache:  %d KB\n", (int)id->l1_data_cache);
    printf("L1I Cache:  %d KB\n", (int)id->l1_instruction_cache);
    printf("L2 Cache:   %d KB\n", (int)id->l2_cache);
    printf("L3 Cache:   %d KB\n", (int)id->l3_cache);

    printf("Features:  ");
    int count = 0;
    for (int i = 0; i < NUM_FEATURES; i++) {
        if (id->flags[i]) {
            if (count > 0) printf(" ");
            printf("%s", cpuinfo_feature_str((cpu_feature_t)i));
            count++;
        }
    }
    printf("\n");
}

int main(int argc, char** argv)
{
    struct cpu_raw_data_t raw;
    struct cpu_id_t id;
    const char* load_file = NULL;
    const char* save_file = NULL;
    int want_report = 0;
    int need_identify = 0;

    enum {
        Q_NONE = 0, Q_VENDORSTR, Q_BRANDSTR, Q_FAMILY, Q_MODEL,
        Q_STEPPING, Q_EXTFAMILY, Q_EXTMODEL, Q_CORES, Q_LOGICAL,
        Q_CODENAME, Q_TECHNOLOGY, Q_CACHE, Q_L1D, Q_L1I, Q_L2, Q_L3,
        Q_FLAGS, Q_CLOCK, Q_CLOCK_RDTSC, Q_VERSION
    };

    int queries[64];
    int num_queries = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--load") == 0) {
            if (++i >= argc) { fprintf(stderr, "Missing argument for --load\n"); return 1; }
            load_file = argv[i];
        } else if (strcmp(argv[i], "--save") == 0) {
            if (++i >= argc) { fprintf(stderr, "Missing argument for --save\n"); return 1; }
            save_file = argv[i];
        } else if (strcmp(argv[i], "--report") == 0) {
            want_report = 1;
            need_identify = 1;
        } else if (strcmp(argv[i], "--vendorstr") == 0) {
            queries[num_queries++] = Q_VENDORSTR; need_identify = 1;
        } else if (strcmp(argv[i], "--brandstr") == 0) {
            queries[num_queries++] = Q_BRANDSTR; need_identify = 1;
        } else if (strcmp(argv[i], "--family") == 0) {
            queries[num_queries++] = Q_FAMILY; need_identify = 1;
        } else if (strcmp(argv[i], "--model") == 0) {
            queries[num_queries++] = Q_MODEL; need_identify = 1;
        } else if (strcmp(argv[i], "--stepping") == 0) {
            queries[num_queries++] = Q_STEPPING; need_identify = 1;
        } else if (strcmp(argv[i], "--extfamily") == 0) {
            queries[num_queries++] = Q_EXTFAMILY; need_identify = 1;
        } else if (strcmp(argv[i], "--extmodel") == 0) {
            queries[num_queries++] = Q_EXTMODEL; need_identify = 1;
        } else if (strcmp(argv[i], "--cores") == 0) {
            queries[num_queries++] = Q_CORES; need_identify = 1;
        } else if (strcmp(argv[i], "--logical") == 0) {
            queries[num_queries++] = Q_LOGICAL; need_identify = 1;
        } else if (strcmp(argv[i], "--codename") == 0) {
            queries[num_queries++] = Q_CODENAME; need_identify = 1;
        } else if (strcmp(argv[i], "--technology") == 0) {
            queries[num_queries++] = Q_TECHNOLOGY; need_identify = 1;
        } else if (strcmp(argv[i], "--cache") == 0 || strcmp(argv[i], "--l2-cache") == 0) {
            queries[num_queries++] = Q_L2; need_identify = 1;
        } else if (strcmp(argv[i], "--l1d-cache") == 0) {
            queries[num_queries++] = Q_L1D; need_identify = 1;
        } else if (strcmp(argv[i], "--l1i-cache") == 0) {
            queries[num_queries++] = Q_L1I; need_identify = 1;
        } else if (strcmp(argv[i], "--l3-cache") == 0) {
            queries[num_queries++] = Q_L3; need_identify = 1;
        } else if (strcmp(argv[i], "--flags") == 0) {
            queries[num_queries++] = Q_FLAGS; need_identify = 1;
        } else if (strcmp(argv[i], "--clock") == 0) {
            queries[num_queries++] = Q_CLOCK;
        } else if (strcmp(argv[i], "--clock-rdtsc") == 0) {
            queries[num_queries++] = Q_CLOCK_RDTSC;
        } else if (strcmp(argv[i], "--version") == 0) {
            queries[num_queries++] = Q_VERSION;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            usage(argv[0]);
            return 1;
        }
    }

    if (argc == 1) {
        want_report = 1;
        need_identify = 1;
    }

    memset(&raw, 0, sizeof(raw));

    if (load_file) {
        if (cpuinfo_deserialize_raw(&raw, load_file) != ERR_OK) {
            fprintf(stderr, "Error: cannot load '%s': %s\n", load_file, cpuinfo_error());
            return 1;
        }
    } else {
        if (cpuinfo_get_raw_data(&raw) != ERR_OK) {
            fprintf(stderr, "Error: cannot get raw CPUID data: %s\n", cpuinfo_error());
            return 1;
        }
    }

    if (save_file) {
        if (cpuinfo_serialize_raw(&raw, save_file) != ERR_OK) {
            fprintf(stderr, "Error: cannot save '%s': %s\n", save_file, cpuinfo_error());
            return 1;
        }
    }

    if (need_identify) {
        if (cpuinfo_identify(&raw, &id) != ERR_OK) {
            fprintf(stderr, "Error: cannot identify CPU: %s\n", cpuinfo_error());
            return 1;
        }
        cpuinfo_decode_caches(&raw, &id);
        cpudb_identify_codename(&id);
    }

    if (want_report) {
        print_report(&id);
        return 0;
    }

    for (int i = 0; i < num_queries; i++) {
        switch (queries[i]) {
            case Q_VENDORSTR:   printf("%s\n", id.vendor_str); break;
            case Q_BRANDSTR:    printf("%s\n", id.brand_str); break;
            case Q_FAMILY:      printf("%d\n", (int)id.family); break;
            case Q_MODEL:       printf("%d\n", (int)id.model); break;
            case Q_STEPPING:    printf("%d\n", (int)id.stepping); break;
            case Q_EXTFAMILY:   printf("%d\n", (int)id.ext_family); break;
            case Q_EXTMODEL:    printf("%d\n", (int)id.ext_model); break;
            case Q_CORES:       printf("%d\n", (int)id.num_cores); break;
            case Q_LOGICAL:     printf("%d\n", (int)id.num_logical_cpus); break;
            case Q_CODENAME:    printf("%s\n", id.cpu_codename); break;
            case Q_TECHNOLOGY:  printf("%s\n", id.technology); break;
            case Q_L1D:         printf("%d\n", (int)id.l1_data_cache); break;
            case Q_L1I:         printf("%d\n", (int)id.l1_instruction_cache); break;
            case Q_L2:          printf("%d\n", (int)id.l2_cache); break;
            case Q_L3:          printf("%d\n", (int)id.l3_cache); break;
            case Q_FLAGS:
                for (int f = 0; f < NUM_FEATURES; f++) {
                    if (id.flags[f])
                        printf("%s\n", cpuinfo_feature_str((cpu_feature_t)f));
                }
                break;
            case Q_CLOCK:
                printf("%d\n", cpu_clock_by_os());
                break;
            case Q_CLOCK_RDTSC:
                printf("%d\n", cpu_clock_measure(100));
                break;
            case Q_VERSION:
                printf("%s\n", cpuinfo_version());
                break;
        }
    }

    return 0;
}
