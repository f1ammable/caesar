#!/bin/bash
set -euo pipefail

echo "Finding changed C++ files against base branch: main"
CHANGED_FILES=$(git diff --name-only --diff-filter=AM "origin/main...HEAD" | \
                grep -E '\.(cpp)$' | \
                grep -v '^test/' || true)

# Filter to only files that exist (not deleted)
EXISTING_FILES=""
for file in $CHANGED_FILES; do
  if [ -f "$file" ]; then
    EXISTING_FILES="$EXISTING_FILES $file"
  fi
done

if [ -z "$EXISTING_FILES" ]; then
  echo "No C++ files changed, skipping clang-tidy"
  exit 0
fi

echo "Changed C++ files: $EXISTING_FILES"
echo "Running clang-tidy..."

# Run clang-tidy on changed files
# shellcheck disable=SC2086
clang-tidy -p build $EXISTING_FILES

echo "clang-tidy completed successfully"
