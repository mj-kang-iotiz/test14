#!/bin/bash
###############################################################################
# Host Test Runner - GUGU MACHINE GUIDANCE V2
#
# Usage:
#   bash test/run_tests.sh          # Build and run all tests
#   bash test/run_tests.sh clean    # Clean build directory
#   bash test/run_tests.sh verbose  # Run with verbose output
#
# AI Usage:
#   Claude should run `bash test/run_tests.sh` after any lib/ code changes.
#   Exit code 0 = all tests passed, non-zero = failure.
###############################################################################

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

# Handle arguments
case "${1:-}" in
    clean)
        echo "==== Clean ===="
        rm -rf "${BUILD_DIR}"
        echo "Build directory removed."
        exit 0
        ;;
    verbose)
        CTEST_FLAGS="--output-on-failure --verbose"
        ;;
    *)
        CTEST_FLAGS="--output-on-failure"
        ;;
esac

echo "==== Configure ===="
cmake -B "${BUILD_DIR}" -S "${SCRIPT_DIR}" -DCMAKE_BUILD_TYPE=Debug 2>&1

echo ""
echo "==== Build ===="
cmake --build "${BUILD_DIR}" 2>&1

echo ""
echo "==== Run Tests ===="
ctest --test-dir "${BUILD_DIR}" ${CTEST_FLAGS} 2>&1

echo ""
echo "==== All tests passed ===="
