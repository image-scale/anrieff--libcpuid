# Goal

## Project
libcpuid — a C library for CPU identification.

## Description
A C library that provides runtime CPU identification for x86 processors. It executes the CPUID instruction, reads raw register data, decodes CPU features (SSE, AVX, etc.), identifies vendor/family/model/stepping, determines cache sizes and topology, recognizes CPU codenames, and can serialize/deserialize raw data for offline analysis. It also provides CPU clock measurement via RDTSC and related methods.

## Scope
- ~10 production source files to implement (library core + tool)
- ~5 test files to write (unit tests using a custom test framework)
- Reproduce core library functionality: raw CPUID data handling, feature detection, vendor identification, cache decoding, codename matching, serialization, clock measurement, and a command-line tool
