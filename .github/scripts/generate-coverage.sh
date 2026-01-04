#!/bin/bash
set -euo pipefail

cd build

echo "Running tests with coverage instrumentation..."
export LLVM_PROFILE_FILE="caesar-%p.profraw"
./caesar_test

echo "Checking for profraw files..."
if ! ls caesar-*.profraw 1> /dev/null 2>&1; then
  echo "ERROR: No .profraw files found!"
  exit 1
fi

echo "Merging profile data..."
/usr/bin/llvm-profdata-21 merge -sparse caesar-*.profraw -o caesar.profdata

echo "Generating coverage report (text summary)..."
/usr/bin/llvm-cov-21 report ./caesar_test \
  -instr-profile=caesar.profdata \
  -ignore-filename-regex='test/.*' \
  -ignore-filename-regex='.*/Catch2/.*'

echo "Generating detailed HTML report..."
/usr/bin/llvm-cov-21 show ./caesar_test \
  -instr-profile=caesar.profdata \
  -format=html \
  -output-dir=coverage-report \
  -ignore-filename-regex='test/.*' \
  -ignore-filename-regex='.*/Catch2/.*' \
  -show-line-counts-or-regions \
  -show-instantiations

echo "Generating per-file coverage summary..."
echo "### Per-File Coverage Summary" > coverage-summary.txt
/usr/bin/llvm-cov-21 report ./caesar_test \
  -instr-profile=caesar.profdata \
  -ignore-filename-regex='test/.*' \
  -ignore-filename-regex='.*/Catch2/.*' \
  >> coverage-summary.txt

echo "Coverage report generated successfully"
cat coverage-summary.txt
