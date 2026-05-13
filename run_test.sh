#!/bin/bash
set -eo pipefail
cd "$(dirname "$0")"

# Build and run all tests
gcc -Wall -Wextra -std=c99 -o tests/test_identify tests/test_identify.c src/cpuinfo.c -I src && ./tests/test_identify
gcc -Wall -Wextra -std=c99 -o tests/test_features tests/test_features.c src/cpuinfo.c -I src && ./tests/test_features
