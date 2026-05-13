#include "cpudb.h"
#include <string.h>
#include <ctype.h>

static const struct codename_entry_t intel_db[] = {
    { -1, -1, -1, -1, -1, -1, -1, -1, "", 0, "Unknown Intel CPU", "unknown" },
    {  4, -1, -1, -1, -1, -1, -1, -1, "", 0, "i486", "unknown" },
    {  5,  1, -1, -1, -1, -1, -1, -1, "", 0, "Pentium 1", "0.8 um" },
    {  5,  2, -1, -1, -1, -1, -1, -1, "", 0, "Pentium 1", "0.35 um" },
    {  5,  8, -1, -1, -1, -1, -1, -1, "", 0, "Pentium MMX", "0.25 um" },
    {  6,  1, -1, -1, -1, -1, -1, -1, "", 0, "Pentium Pro", "0.35 um" },
    {  6,  3, -1, -1, -1, -1, -1, -1, "", 0, "Pentium II (Klamath)", "0.35 um" },
    {  6,  5, -1, -1, -1, -1, -1, -1, "", 0, "Pentium II (Deschutes)", "0.25 um" },
    {  6,  7, -1, -1, -1, -1, -1, -1, "", 0, "Pentium III (Katmai)", "0.25 um" },
    {  6,  8, -1, -1, -1, -1, -1, -1, "", 0, "Pentium III (Coppermine)", "0.18 um" },
    {  6, 11, -1, -1, -1, -1, -1, -1, "", 0, "Pentium III (Tualatin)", "0.13 um" },
    {  6, -1, -1,  6, 0x1C, -1, -1, -1, "Atom", 2, "Atom (Bonnell)", "45 nm" },
    {  6, -1, -1,  6, 0x26, -1, -1, -1, "Atom", 2, "Atom (Bonnell)", "45 nm" },
    {  6, -1, -1,  6, 0x37, -1, -1, -1, "Atom", 2, "Atom (Silvermont)", "22 nm" },
    {  6, -1, -1,  6, 0x0F, -1, -1, -1, "Core", 2, "Conroe (Core 2 Duo)", "65 nm" },
    {  6, -1, -1,  6, 0x16, -1, -1, -1, "Core", 2, "Merom (Core 2 Duo)", "65 nm" },
    {  6, -1, -1,  6, 0x17, -1, -1, -1, "Core", 2, "Penryn (Core 2 Duo)", "45 nm" },
    {  6, -1, -1,  6, 0x1A, -1, -1, -1, "", 0, "Nehalem", "45 nm" },
    {  6, -1, -1,  6, 0x1E, -1, -1, -1, "", 0, "Nehalem", "45 nm" },
    {  6, -1, -1,  6, 0x25, -1, -1, -1, "", 0, "Westmere", "32 nm" },
    {  6, -1, -1,  6, 0x2C, -1, -1, -1, "", 0, "Westmere", "32 nm" },
    {  6, -1, -1,  6, 0x2A, -1, -1, -1, "", 0, "Sandy Bridge", "32 nm" },
    {  6, -1, -1,  6, 0x2D, -1, -1, -1, "", 0, "Sandy Bridge-E", "32 nm" },
    {  6, -1, -1,  6, 0x3A, -1, -1, -1, "", 0, "Ivy Bridge", "22 nm" },
    {  6, -1, -1,  6, 0x3E, -1, -1, -1, "", 0, "Ivy Bridge-E", "22 nm" },
    {  6, -1, -1,  6, 0x3C, -1, -1, -1, "", 0, "Haswell", "22 nm" },
    {  6, -1, -1,  6, 0x3F, -1, -1, -1, "", 0, "Haswell-E", "22 nm" },
    {  6, -1, -1,  6, 0x45, -1, -1, -1, "", 0, "Haswell", "22 nm" },
    {  6, -1, -1,  6, 0x46, -1, -1, -1, "", 0, "Haswell", "22 nm" },
    {  6, -1, -1,  6, 0x3D, -1, -1, -1, "", 0, "Broadwell", "14 nm" },
    {  6, -1, -1,  6, 0x47, -1, -1, -1, "", 0, "Broadwell", "14 nm" },
    {  6, -1, -1,  6, 0x4F, -1, -1, -1, "", 0, "Broadwell-E", "14 nm" },
    {  6, -1, -1,  6, 0x56, -1, -1, -1, "", 0, "Broadwell-DE", "14 nm" },
    {  6, -1, -1,  6, 0x4E, -1, -1, -1, "", 0, "Skylake", "14 nm" },
    {  6, -1, -1,  6, 0x5E, -1, -1, -1, "", 0, "Skylake", "14 nm" },
    {  6, -1, -1,  6, 0x55, -1, -1, -1, "", 0, "Skylake-X", "14 nm" },
    {  6, -1, -1,  6, 0x8E, -1, -1, -1, "", 0, "Kaby Lake", "14 nm" },
    {  6, -1, -1,  6, 0x9E, -1, -1, -1, "", 0, "Coffee Lake", "14 nm" },
    {  6, -1, -1,  6, 0xA5, -1, -1, -1, "", 0, "Comet Lake", "14 nm" },
    {  6, -1, -1,  6, 0xA6, -1, -1, -1, "", 0, "Comet Lake", "14 nm" },
    {  6, -1, -1,  6, 0x7E, -1, -1, -1, "", 0, "Ice Lake", "10 nm" },
    {  6, -1, -1,  6, 0x7D, -1, -1, -1, "", 0, "Ice Lake", "10 nm" },
    {  6, -1, -1,  6, 0x8C, -1, -1, -1, "", 0, "Tiger Lake", "10 nm" },
    {  6, -1, -1,  6, 0x8D, -1, -1, -1, "", 0, "Tiger Lake", "10 nm" },
    {  6, -1, -1,  6, 0x97, -1, -1, -1, "", 0, "Alder Lake", "Intel 7" },
    {  6, -1, -1,  6, 0x9A, -1, -1, -1, "", 0, "Alder Lake", "Intel 7" },
    {  6, -1, -1,  6, 0xB7, -1, -1, -1, "", 0, "Raptor Lake", "Intel 7" },
    {  6, -1, -1,  6, 0xBA, -1, -1, -1, "", 0, "Raptor Lake", "Intel 7" },
    { 15, -1, -1, 15, -1, -1, -1, -1, "", 0, "Pentium 4", "unknown" },
};

static const struct codename_entry_t amd_db[] = {
    { -1, -1, -1, -1, -1, -1, -1, -1, "", 0, "Unknown AMD CPU", "unknown" },
    {  5, -1, -1, -1, -1, -1, -1, -1, "", 0, "K5/K6", "unknown" },
    {  6, -1, -1, -1, -1, -1, -1, -1, "", 0, "K7 (Athlon/Duron)", "unknown" },
    { 15, -1, -1, 15, -1, -1, -1, -1, "", 0, "K8 (Athlon 64)", "unknown" },
    { 15, -1, -1, 16, -1, -1, -1, -1, "", 0, "K10 (Phenom)", "65 nm" },
    { 15, -1, -1, 17, -1, -1, -1, -1, "", 0, "Turion X2", "65 nm" },
    { 15, -1, -1, 20, -1, -1, -1, -1, "", 0, "Bobcat", "40 nm" },
    { 15, -1, -1, 21, -1, -1, -1, -1, "", 0, "Bulldozer", "32 nm" },
    { 15, -1, -1, 21, 0x02, -1, -1, -1, "", 0, "Piledriver", "32 nm" },
    { 15, -1, -1, 21, 0x13, -1, -1, -1, "", 0, "Piledriver", "32 nm" },
    { 15, -1, -1, 21, 0x30, -1, -1, -1, "", 0, "Steamroller", "28 nm" },
    { 15, -1, -1, 21, 0x38, -1, -1, -1, "", 0, "Steamroller", "28 nm" },
    { 15, -1, -1, 21, 0x60, -1, -1, -1, "", 0, "Excavator", "28 nm" },
    { 15, -1, -1, 21, 0x65, -1, -1, -1, "", 0, "Excavator", "28 nm" },
    { 15, -1, -1, 21, 0x70, -1, -1, -1, "", 0, "Excavator", "28 nm" },
    { 15, -1, -1, 22, -1, -1, -1, -1, "", 0, "Jaguar", "28 nm" },
    { 15, -1, -1, 23, 0x01, -1, -1, -1, "Ryzen", 2, "Ryzen (Zen)", "14 nm" },
    { 15, -1, -1, 23, 0x08, -1, -1, -1, "Ryzen", 2, "Ryzen (Zen+)", "12 nm" },
    { 15, -1, -1, 23, 0x11, -1, -1, -1, "", 0, "Zen (Raven Ridge)", "14 nm" },
    { 15, -1, -1, 23, 0x18, -1, -1, -1, "", 0, "Zen+ (Picasso)", "12 nm" },
    { 15, -1, -1, 23, 0x31, -1, -1, -1, "EPYC", 2, "EPYC (Rome)", "TSMC N7FF" },
    { 15, -1, -1, 23, 0x60, -1, -1, -1, "Ryzen", 2, "Ryzen (Renoir)", "TSMC N7FF" },
    { 15, -1, -1, 23, 0x71, -1, -1, -1, "Ryzen", 2, "Ryzen (Matisse)", "TSMC N7FF" },
    { 15, -1, -1, 25, 0x21, -1, -1, -1, "Ryzen", 2, "Ryzen (Vermeer)", "TSMC N7FF" },
    { 15, -1, -1, 25, 0x50, -1, -1, -1, "Ryzen", 2, "Ryzen 9 (Cezanne)", "TSMC N7FF" },
    { 15, -1, -1, 25, 0x44, -1, -1, -1, "Ryzen", 2, "Ryzen (Rembrandt)", "TSMC N6" },
    { 15, -1, -1, 25, 0x61, -1, -1, -1, "Ryzen", 2, "Ryzen (Raphael)", "TSMC N5" },
    { 15, -1, -1, 25, 0x74, -1, -1, -1, "Ryzen", 2, "Ryzen (Phoenix)", "TSMC N4" },
    { 15, -1, -1, 25, -1, -1, -1, -1, "", 0, "Zen 3/4", "TSMC N7FF" },
    { 15, -1, -1, 26, -1, -1, -1, -1, "", 0, "Zen 5", "TSMC N4" },
};

static int pattern_match(const char* haystack, const char* pattern)
{
    if (!pattern || pattern[0] == '\0')
        return 0;
    if (!haystack)
        return 0;

    const char* h = haystack;
    while (*h) {
        const char* hp = h;
        const char* pp = pattern;
        int matched = 1;
        while (*pp && *hp) {
            if (*pp == '.') {
                pp++;
                hp++;
            } else if (*pp == '#') {
                if (!isdigit((unsigned char)*hp)) { matched = 0; break; }
                pp++;
                hp++;
            } else {
                if (tolower((unsigned char)*pp) != tolower((unsigned char)*hp)) { matched = 0; break; }
                pp++;
                hp++;
            }
        }
        if (matched && *pp == '\0')
            return 1;
        h++;
    }
    return 0;
}

static int compute_score(const struct codename_entry_t* entry, const struct cpu_id_t* id)
{
    int score = 0;

    if (entry->family >= 0 && entry->family == id->family)
        score += 2;
    if (entry->model >= 0 && entry->model == id->model)
        score += 2;
    if (entry->stepping >= 0 && entry->stepping == id->stepping)
        score += 2;
    if (entry->ext_family >= 0 && entry->ext_family == id->ext_family)
        score += 2;
    if (entry->ext_model >= 0 && entry->ext_model == id->ext_model)
        score += 4;
    if (entry->num_cores >= 0 && entry->num_cores == id->num_cores)
        score += 2;
    if (entry->l2_cache >= 0 && entry->l2_cache == id->l2_cache)
        score += 1;
    if (entry->l3_cache >= 0 && entry->l3_cache == id->l3_cache)
        score += 1;

    if (entry->brand_score > 0 && entry->brand_pattern[0] != '\0') {
        if (pattern_match(id->brand_str, entry->brand_pattern))
            score += entry->brand_score;
    }

    return score;
}

static void match_against_db(const struct codename_entry_t* db, int db_count,
                             struct cpu_id_t* id)
{
    int best_score = -1;
    int best_idx = 0;

    for (int i = 0; i < db_count; i++) {
        int s = compute_score(&db[i], id);
        if (s > best_score) {
            best_score = s;
            best_idx = i;
        }
    }

    strncpy(id->cpu_codename, db[best_idx].codename, CODENAME_MAX - 1);
    id->cpu_codename[CODENAME_MAX - 1] = '\0';
    strncpy(id->technology, db[best_idx].technology, TECHNOLOGY_MAX - 1);
    id->technology[TECHNOLOGY_MAX - 1] = '\0';
}

int cpudb_identify_codename(struct cpu_id_t* id)
{
    if (!id)
        return cpuinfo_set_error(ERR_REQUEST);

    switch (id->vendor) {
        case VENDOR_INTEL:
            match_against_db(intel_db,
                            (int)(sizeof(intel_db) / sizeof(intel_db[0])), id);
            break;
        case VENDOR_AMD:
        case VENDOR_HYGON:
            match_against_db(amd_db,
                            (int)(sizeof(amd_db) / sizeof(amd_db[0])), id);
            break;
        default:
            strncpy(id->cpu_codename, "Unknown", CODENAME_MAX - 1);
            strncpy(id->technology, "unknown", TECHNOLOGY_MAX - 1);
            break;
    }

    return (int)ERR_OK;
}

void cpudb_get_list(cpu_vendor_t vendor, const char*** names, int* count)
{
    static const char* intel_names[64];
    static const char* amd_names[64];
    static int intel_count = 0;
    static int amd_count = 0;
    static int initialized = 0;

    if (!initialized) {
        int db_size = (int)(sizeof(intel_db) / sizeof(intel_db[0]));
        intel_count = 0;
        for (int i = 0; i < db_size && intel_count < 64; i++) {
            if (strstr(intel_db[i].codename, "Unknown")) continue;
            int dup = 0;
            for (int j = 0; j < intel_count; j++) {
                if (strcmp(intel_names[j], intel_db[i].codename) == 0) { dup = 1; break; }
            }
            if (!dup) intel_names[intel_count++] = intel_db[i].codename;
        }

        db_size = (int)(sizeof(amd_db) / sizeof(amd_db[0]));
        amd_count = 0;
        for (int i = 0; i < db_size && amd_count < 64; i++) {
            if (strstr(amd_db[i].codename, "Unknown")) continue;
            int dup = 0;
            for (int j = 0; j < amd_count; j++) {
                if (strcmp(amd_names[j], amd_db[i].codename) == 0) { dup = 1; break; }
            }
            if (!dup) amd_names[amd_count++] = amd_db[i].codename;
        }
        initialized = 1;
    }

    if (vendor == VENDOR_INTEL) {
        *names = intel_names;
        *count = intel_count;
    } else if (vendor == VENDOR_AMD) {
        *names = amd_names;
        *count = amd_count;
    } else {
        *names = NULL;
        *count = 0;
    }
}
