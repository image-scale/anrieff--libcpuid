# Acceptance Criteria

## Task 1: Core CPU identification
- [x] All criteria met

## Task 2: Feature flag detection (extended)

### Acceptance Criteria
- [ ] Detects all leaf 7 subleaf 0 EBX features: FSGSBASE, BMI1, AVX2, BMI2, ERMS, INVPCID, AVX512F/DQ/PF/ER/CD/BW/VL, SHA, RDSEED, ADX, SGX
- [ ] Detects leaf 7 subleaf 0 ECX features: AVX512VBMI, AVX512VBMI2, AVX512VNNI, VAES, VPCLMULQDQ, AVX512BITALG
- [ ] Detects extended leaf 80000001 AMD features: LAHF_LM, SVM, ABM/LZCNT, SSE4A, MISALIGN_SSE, 3DNow prefetch, XOP, FMA4, TBM
- [ ] Provides complete string name mapping for every defined feature enum value
- [ ] A test can verify that specific CPUID register patterns produce the expected set of detected features
- [ ] Feature detection is compatible with both Intel and AMD raw data
- [ ] All feature name strings are non-empty for valid feature enum values
