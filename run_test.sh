#!/bin/bash
set -eo pipefail
cd "$(dirname "$0")"

# Build and run all tests
gcc -Wall -Wextra -std=c99 -o tests/test_identify tests/test_identify.c src/cpuinfo.c src/serialize.c -I src && ./tests/test_identify
gcc -Wall -Wextra -std=c99 -o tests/test_features tests/test_features.c src/cpuinfo.c src/serialize.c -I src && ./tests/test_features
gcc -Wall -Wextra -std=c99 -o tests/test_serialize tests/test_serialize.c src/cpuinfo.c src/serialize.c -I src && ./tests/test_serialize
gcc -Wall -Wextra -std=c99 -o tests/test_codename tests/test_codename.c src/cpuinfo.c src/serialize.c src/cpudb.c -I src && ./tests/test_codename
gcc -Wall -Wextra -std=c99 -o tests/test_cache tests/test_cache.c src/cpuinfo.c src/serialize.c src/cache.c -I src && ./tests/test_cache
gcc -Wall -Wextra -std=c99 -o tests/test_clock tests/test_clock.c src/cpuinfo.c src/serialize.c src/clock.c -I src && ./tests/test_clock
gcc -Wall -Wextra -std=c99 -o tools/cputool tools/cputool.c src/cpuinfo.c src/serialize.c src/cpudb.c src/cache.c src/clock.c -I src && \
gcc -Wall -Wextra -std=c99 -o tests/test_tool tests/test_tool.c src/cpuinfo.c src/serialize.c src/cpudb.c src/cache.c src/clock.c -I src && ./tests/test_tool
