#!/bin/bash
# Upload script for minlab to Launchpad PPA
# Usage: ./scripts/upload-ppa.sh [PPA_NAME] [VERSION]
# Example: ./scripts/upload-ppa.sh newang/minlab 1.0.0-1~ppa1

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

PPA_NAME="${1:-YOUR_USERNAME/PPA_NAME}"
VERSION="${2:-1.0.0-1~ppa1}"

if [ "$PPA_NAME" == "YOUR_USERNAME/PPA_NAME" ]; then
    echo "Error: Please provide your PPA name"
    echo "Usage: $0 YOUR_USERNAME/PPA_NAME [VERSION]"
    echo "Example: $0 newang/minlab 1.0.0-1~ppa1"
    exit 1
fi

echo "=========================================="
echo "Uploading minlab to Launchpad PPA"
echo "=========================================="
echo "PPA: ppa:$PPA_NAME"
echo "Version: $VERSION"
echo ""

# Check if dput is installed
if ! command -v dput &> /dev/null; then
    echo "Error: dput is not installed"
    echo "Install it with: sudo apt-get install dput"
    exit 1
fi

# Clean previous builds
echo "Cleaning previous builds..."
rm -rf build/
rm -rf ../minlab_*.*.*/
rm -f ../minlab_*.{deb,dsc,changes,build,upload} 2>/dev/null || true

# Update changelog
echo "Updating changelog..."
dch -v "$VERSION" "PPA upload"

# Build source package
echo "Building source package..."
debuild -S -sa

# Find the changes file
CHANGES_FILE=$(ls -t ../minlab_*_source.changes 2>/dev/null | head -1)

if [ -z "$CHANGES_FILE" ]; then
    echo "Error: Could not find .changes file"
    exit 1
fi

echo ""
echo "Found changes file: $CHANGES_FILE"
echo ""

# Ask for confirmation
read -p "Upload to ppa:$PPA_NAME? (y/N) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Upload cancelled"
    exit 1
fi

# Upload
echo "Uploading..."
dput "ppa:$PPA_NAME" "$CHANGES_FILE"

echo ""
echo "=========================================="
echo "Upload complete!"
echo "=========================================="
echo ""
echo "Monitor build status at:"
echo "  https://launchpad.net/~${PPA_NAME%%/*}/+archive/ubuntu/${PPA_NAME##*/}"
echo ""
echo "Once built, users can install with:"
echo "  sudo add-apt-repository ppa:$PPA_NAME"
echo "  sudo apt-get update"
echo "  sudo apt-get install minlab"
echo ""

