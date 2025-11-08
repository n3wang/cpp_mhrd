# Deployment Instructions for Ubuntu PPA

This document provides step-by-step instructions for building and publishing `minlab` as an Ubuntu package via Launchpad PPA (Personal Package Archive).

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Initial Setup](#initial-setup)
3. [Preparing the Package](#preparing-the-package)
4. [Building the Package Locally](#building-the-package-locally)
5. [Creating a PPA on Launchpad](#creating-a-ppa-on-launchpad)
6. [Uploading to PPA](#uploading-to-ppa)
7. [Testing the PPA](#testing-the-ppa)
8. [Updating the Package](#updating-the-package)
9. [Troubleshooting](#troubleshooting)

## Prerequisites

### 1. Install Required Tools

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    debhelper \
    cmake \
    devscripts \
    dput \
    dput-ng \
    lintian \
    pbuilder \
    ubuntu-dev-tools
```

### 2. Set Up GPG Key (if not already done)

You need a GPG key to sign your packages:

```bash
# Check if you have a GPG key
gpg --list-secret-keys

# If not, create one
gpg --gen-key
# Choose: (1) RSA and RSA (default)
# Key size: 4096
# Expiration: 0 (no expiration) or your preference
# Enter your name and email (must match Launchpad account)
# Enter a passphrase

# Export your public key
gpg --armor --export YOUR_EMAIL@example.com > ~/my-public-key.txt

# Upload to keyserver
gpg --send-keys --keyserver keyserver.ubuntu.com YOUR_KEY_ID
```

### 3. Launchpad Account Setup

1. Create an account at https://launchpad.net/
2. Upload your GPG key:
   - Go to https://launchpad.net/~YOUR_USERNAME/+editpgpkeys
   - Paste your public key or import from keyserver
   - Launchpad will send you an encrypted email to verify
3. Create a PPA:
   - Go to https://launchpad.net/~YOUR_USERNAME/+archive/ubuntu/ppa
   - Click "Create a new PPA"
   - Name it (e.g., "minlab" or just use the default "ppa")
   - Accept the terms

## Initial Setup

### 1. Update Package Information

Edit `debian/control` and update:
- `Maintainer`: Your name and email
- `Homepage`: Your project URL
- `Description`: Package description

Edit `debian/changelog` and ensure the first entry has:
- Correct version number
- Your name and email
- Current date

### 2. Verify Source Format

Ensure `debian/source/format` contains:
```
3.0 (native)
```

For native packages (no separate upstream tarball), or:
```
3.0 (quilt)
```

For non-native packages.

## Preparing the Package

### 1. Clean the Repository

```bash
# Remove build artifacts
rm -rf build/
rm -rf ../minlab_*.*.*/
rm -f ../minlab_*.{deb,dsc,changes,build,upload}

# Ensure clean git state (optional but recommended)
git status
```

### 2. Update Version in Changelog

For a new release:

```bash
# For native package (version format: X.Y.Z-N)
dch -v 1.0.0-1

# For PPA upload (version format: X.Y.Z-1~ppaN)
dch -v 1.0.0-1~ppa1

# Edit the changelog entry
dch -e
```

The changelog format should be:
```
minlab (1.0.0-1~ppa1) focal; urgency=medium

  * Initial PPA release
  * Interactive game mode with level editor
  * Terminal UI with tabbed interface
  * Comprehensive keyboard shortcuts

 -- Your Name <your.email@example.com>  Mon, 08 Jan 2025 12:00:00 +0000
```

### 3. Update Copyright File

Ensure `debian/copyright` exists and is up to date (see below for template).

## Building the Package Locally

### 1. Build Binary Package (for local testing)

```bash
# Build the .deb package
dpkg-buildpackage -us -uc

# This creates:
# - ../minlab_1.0.0-1~ppa1_amd64.deb (binary package)
# - ../minlab_1.0.0-1~ppa1.dsc (source description)
# - ../minlab_1.0.0-1~ppa1.tar.xz (source tarball)
# - ../minlab_1.0.0-1~ppa1_amd64.changes (changes file)
```

### 2. Test Installation Locally

```bash
# Install the package
sudo dpkg -i ../minlab_1.0.0-1~ppa1_amd64.deb

# Fix any missing dependencies
sudo apt-get install -f

# Test the application
minlab --help
minlab

# Uninstall if needed
sudo apt-get remove minlab
```

### 3. Check Package Quality

```bash
# Run lintian (package checker)
lintian ../minlab_1.0.0-1~ppa1_amd64.deb

# Fix any errors or warnings reported
```

## Creating a PPA on Launchpad

1. **Log in to Launchpad**: https://launchpad.net/
2. **Go to your profile**: https://launchpad.net/~YOUR_USERNAME
3. **Create PPA**: Click "Create a new PPA"
4. **Fill in details**:
   - **PPA name**: e.g., `minlab` or `ppa` (default)
   - **Display name**: e.g., "MHRD CLI"
   - **Description**: Brief description of your package
5. **Save**: The PPA URL will be: `ppa:YOUR_USERNAME/PPA_NAME`

## Uploading to PPA

### 1. Build Source Package

```bash
# Build source package for upload
debuild -S -sa

# This creates:
# - ../minlab_1.0.0-1~ppa1_source.changes
# - ../minlab_1.0.0-1~ppa1.dsc
# - ../minlab_1.0.0-1~ppa1.tar.xz
```

### 2. Sign the Package (if not using -us -uc)

```bash
# Sign the .changes file
debsign ../minlab_1.0.0-1~ppa1_source.changes
```

### 3. Upload to PPA

```bash
# Upload to your PPA
dput ppa:YOUR_USERNAME/PPA_NAME ../minlab_1.0.0-1~ppa1_source.changes

# Example:
# dput ppa:newang/minlab ../minlab_1.0.0-1~ppa1_source.changes
```

### 4. Monitor Build Status

1. Go to your PPA: https://launchpad.net/~YOUR_USERNAME/+archive/ubuntu/PPA_NAME
2. Check the "Builds" tab for build status
3. Wait for builds to complete (usually 10-30 minutes)
4. Check for any build errors

## Testing the PPA

### 1. Add PPA to Test System

```bash
# Add your PPA
sudo add-apt-repository ppa:YOUR_USERNAME/PPA_NAME
sudo apt-get update

# Install the package
sudo apt-get install minlab

# Test it
minlab --help
minlab
```

### 2. Verify Installation

```bash
# Check package info
dpkg -l minlab
dpkg -L minlab

# Check version
minlab --version  # if implemented
```

## Updating the Package

### 1. Make Changes

Make your code changes and commit them.

### 2. Update Changelog

```bash
# Increment version
dch -i  # Increment debian revision
# or
dch -v 1.0.1-1~ppa1  # New version

# Edit changelog
dch -e
```

### 3. Rebuild and Upload

```bash
# Clean previous builds
rm -rf ../minlab_*.*.*/

# Build source package
debuild -S -sa

# Upload
dput ppa:YOUR_USERNAME/PPA_NAME ../minlab_1.0.1-1~ppa1_source.changes
```

## Multi-Distribution Support

To support multiple Ubuntu releases:

### 1. Update Changelog for Each Distribution

```bash
# For each Ubuntu release (focal, jammy, noble, etc.)
dch -D focal -v 1.0.0-1~ppa1
dch -D jammy -v 1.0.0-1~ppa1
dch -D noble -v 1.0.0-1~ppa1
```

### 2. Build for Each Distribution

You can use `pbuilder` or `cowbuilder` to build for different distributions:

```bash
# Install pbuilder
sudo apt-get install pbuilder

# Create base tarball for each distribution
sudo pbuilder create --distribution focal
sudo pbuilder create --distribution jammy
sudo pbuilder create --distribution noble

# Build for each
sudo pbuilder build --distribution focal ../minlab_1.0.0-1~ppa1.dsc
```

Or simply upload once and Launchpad will build for all supported architectures.

## Troubleshooting

### Build Errors

1. **Missing dependencies**: Check `debian/control` `Build-Depends`
2. **CMake errors**: Ensure CMakeLists.txt is correct
3. **Permission errors**: Check file permissions in debian/

### Upload Errors

1. **GPG key not found**: Ensure key is uploaded to Launchpad
2. **Invalid signature**: Re-sign with `debsign`
3. **PPA name wrong**: Verify PPA name in dput command

### Common Issues

1. **"Package already exists"**: Increment version number
2. **"Distribution not found"**: Check Ubuntu codename (focal, jammy, noble, etc.)
3. **"Source format error"**: Check `debian/source/format`

### Getting Help

- Launchpad Help: https://help.launchpad.net/Packaging
- Debian Packaging Guide: https://www.debian.org/doc/manuals/packaging-tutorial/
- Ubuntu Packaging Guide: https://packaging.ubuntu.com/html/

## Quick Reference

```bash
# Full workflow
dch -v 1.0.0-1~ppa1
dch -e  # Edit changelog
debuild -S -sa
dput ppa:YOUR_USERNAME/PPA_NAME ../minlab_1.0.0-1~ppa1_source.changes

# Check build status
# Visit: https://launchpad.net/~YOUR_USERNAME/+archive/ubuntu/PPA_NAME
```

## Next Steps

After successful PPA publication:

1. **Document installation instructions** in README.md
2. **Create release notes** on GitHub/GitLab
3. **Announce the release** to users
4. **Monitor for issues** and prepare updates

