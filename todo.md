# Todo

## Plan
Implement the library bottom-up by functionality: first the core types and enumerations with basic identification, then feature decoding, then serialization/deserialization of raw data, then CPU codename matching with a database, then cache topology decoding, then clock measurement, and finally a command-line tool that exercises the library. Each task delivers testable user-facing behavior.

## Tasks
- [ ] Task 1: Core CPU identification — reading raw CPUID data from registers, detecting vendor string, extracting family/model/stepping, and basic feature flag detection (src/cpuinfo.h, src/cpuinfo.c + tests/test_identify.c)
- [ ] Task 2: Feature flag detection — decode the full set of CPU feature flags from CPUID leaves and provide string names for each feature (src/features.h, src/features.c + tests/test_features.c)
- [ ] Task 3: Serialization and deserialization — write raw CPUID data to a text file and read it back, enabling offline CPU identification (src/serialize.c + tests/test_serialize.c)
- [ ] Task 4: CPU codename matching — maintain a database of known CPUs and match family/model/stepping/cache/brand to determine the CPU codename and technology node (src/cpudb.h, src/cpudb.c + tests/test_codename.c)
- [ ] Task 5: Cache topology detection — decode cache hierarchy information (L1/L2/L3/L4 sizes, associativity, line sizes, instances) from CPUID deterministic cache leaves (src/cache.c + tests/test_cache.c)
- [ ] Task 6: CPU clock measurement — provide multiple methods to determine CPU frequency: OS-reported, RDTSC-based measurement, and TSC-from-CPUID leaf (src/clock.c + tests/test_clock.c)
- [ ] Task 7: Command-line tool — a cpuinfo_tool program that uses the library to load raw data, identify CPUs, and print results in various formats suitable for integration testing (tools/cputool.c + tests/test_tool.c)
