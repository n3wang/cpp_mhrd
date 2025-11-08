#!/bin/bash
# Build script for minlab Debian package
# Usage: ./scripts/build-package.sh [version] [distribution]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

VERSION="${1:-1.0.0-1}"
DISTRIBUTION="${2:-unstable}"

echo "=========================================="
echo "Building minlab Debian Package"
echo "=========================================="
echo "Version: $VERSION"
echo "Distribution: $DISTRIBUTION"
echo ""

# Clean previous builds
echo "Cleaning previous builds..."
rm -rf build/
rm -rf ../minlab_*.*.*/
rm -f ../minlab_*.{deb,dsc,changes,build,upload} 2>/dev/null || true

# Update changelog if version provided
if [ "$1" != "" ]; then
    echo "Updating changelog..."
    dch -v "$VERSION" -D "$DISTRIBUTION" "Package build"
fi

# Build package
echo "Building package..."
dpkg-buildpackage -us -uc

echo ""
echo "=========================================="
echo "Build complete!"
echo "=========================================="
echo ""
echo "Package files created in parent directory:"
ls -lh ../minlab_*.*.* 2>/dev/null || true
ls -lh ../minlab_*.deb 2>/dev/null || true
ls -lh ../minlab_*.dsc 2>/dev/null || true
ls -lh ../minlab_*.changes 2>/dev/null || true

echo ""
echo "To install locally:"
echo "  sudo dpkg -i ../minlab_${VERSION}_*.deb"
echo "  sudo apt-get install -f"
echo ""
echo "To build source package for PPA:"
echo "  debuild -S -sa"
echo ""

