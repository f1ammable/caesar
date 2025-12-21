#!/bin/bash
# Script to generate code coverage reports

set -e

BUILD_DIR="build"
COVERAGE_DIR="coverage"

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}==> Building with coverage enabled...${NC}"
cmake -B ${BUILD_DIR} -DENABLE_COVERAGE=ON -DCAESAR_BUILD_TESTS=ON
cmake --build ${BUILD_DIR}

echo -e "${BLUE}==> Running tests...${NC}"
cd ${BUILD_DIR}
./caesar_test

# Detect compiler and generate appropriate coverage report
if [[ $(cmake --system-information | grep CMAKE_CXX_COMPILER_ID) =~ "Clang" ]] || command -v llvm-cov &> /dev/null; then
    echo -e "${BLUE}==> Generating coverage report with llvm-cov...${NC}"

    # Merge profiling data
    xcrun llvm-profdata merge -sparse default.profraw -o coverage.profdata

    # Generate HTML report
    mkdir -p ../${COVERAGE_DIR}
    xcrun llvm-cov show ./caesar_test \
        -instr-profile=coverage.profdata \
        -format=html \
        -output-dir=../${COVERAGE_DIR} \
        -ignore-filename-regex="(test/|vcpkg/|build/)" \
        -show-line-counts-or-regions

    # Generate summary
    xcrun llvm-cov report ./caesar_test \
        -instr-profile=coverage.profdata \
        -ignore-filename-regex="(test/|vcpkg/|build/)"

    echo -e "${GREEN}==> Coverage report generated in ${COVERAGE_DIR}/index.html${NC}"
    echo -e "${GREEN}==> Open with: open ${COVERAGE_DIR}/index.html${NC}"

elif command -v gcov &> /dev/null && command -v lcov &> /dev/null; then
    echo -e "${BLUE}==> Generating coverage report with lcov...${NC}"

    # Generate coverage data
    lcov --capture --directory . --output-file coverage.info

    # Filter out system headers and test files
    lcov --remove coverage.info '/usr/*' '*/test/*' '*/vcpkg/*' --output-file coverage_filtered.info

    # Generate HTML report
    mkdir -p ../${COVERAGE_DIR}
    genhtml coverage_filtered.info --output-directory ../${COVERAGE_DIR}

    echo -e "${GREEN}==> Coverage report generated in ${COVERAGE_DIR}/index.html${NC}"
    echo -e "${GREEN}==> Open with: open ${COVERAGE_DIR}/index.html${NC}"
else
    echo "Error: No coverage tool found. Install llvm-cov or lcov/gcov"
    exit 1
fi

cd ..
