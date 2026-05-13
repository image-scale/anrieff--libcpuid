# Acceptance Criteria

## Task 1: Core CPU identification

### Acceptance Criteria
- [ ] Library defines enumerations for CPU vendors (Intel, AMD, Cyrix, etc.) and architectures (x86, ARM)
- [ ] Library defines a raw data structure that can hold CPUID register results for basic and extended leaves
- [ ] Library defines a cpu_id structure containing vendor string, brand string, family, model, stepping, ext_family, ext_model, and feature flags
- [ ] A function can execute the CPUID instruction and store results in the raw data structure
- [ ] A function can identify the CPU vendor from the raw vendor string (e.g. "GenuineIntel" -> VENDOR_INTEL)
- [ ] A function can extract family/model/stepping from CPUID leaf 1 (including extended family/model computation)
- [ ] A function can extract the brand string from CPUID leaves 0x80000002-0x80000004
- [ ] A function can detect basic feature flags from CPUID leaf 1 (ECX and EDX) — at minimum: FPU, MMX, SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2, AVX, AES, HT
- [ ] The library provides an error reporting mechanism (error codes and string descriptions)
- [ ] The library provides a function to get the library version string
- [ ] All public functions can be tested via unit tests that feed mock/pre-recorded CPUID data
