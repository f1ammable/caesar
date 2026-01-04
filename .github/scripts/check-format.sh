#!/bin/bash
set -euo pipefail

echo "Finding changed C++ files against base branch: main"
CHANGED_FILES=$(git diff --name-only --diff-filter=AM "origin/main...HEAD" | \
                grep -E '\.(cpp|hpp|h)$' || true)

# Filter to only files that exist (not deleted)
EXISTING_FILES=""
for file in $CHANGED_FILES; do
  if [ -f "$file" ]; then
    EXISTING_FILES="$EXISTING_FILES $file"
  fi
done

if [ -z "$EXISTING_FILES" ]; then
  echo "No C++ files changed, skipping format check"
  exit 0
fi

echo "Checking formatting for: $EXISTING_FILES"

# shellcheck disable=SC2086
clang-format --dry-run --Werror $EXISTING_FILES

echo "All files are properly formatted!"
