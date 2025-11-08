#!/bin/bash

# Test runner script for UI controls
# Usage: ./tests/run_tests.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"

cd "$PROJECT_ROOT"

echo "Building test executables..."
cd "$BUILD_DIR"
cmake .. > /dev/null 2>&1
make test-ui-controls test-editor-integration > /dev/null 2>&1

echo ""
echo "=========================================="
echo "Running UI Control Tests"
echo "=========================================="
./test-ui-controls

echo ""
echo "=========================================="
echo "Running Editor Integration Tests"
echo "=========================================="
./test-editor-integration

echo ""
echo "=========================================="
echo "All tests completed!"
echo "=========================================="

