# Quick Start: Publishing to Ubuntu PPA

This is a condensed guide for quickly publishing minlab to Launchpad PPA.

## Prerequisites

```bash
sudo apt-get install -y build-essential debhelper cmake devscripts dput
```

## One-Time Setup

1. **Create Launchpad account**: https://launchpad.net/
2. **Create PPA**: https://launchpad.net/~YOUR_USERNAME/+archive/ubuntu/ppa
3. **Set up GPG key** (if needed):
   ```bash
   gpg --gen-key
   gpg --send-keys --keyserver keyserver.ubuntu.com YOUR_KEY_ID
   # Upload key to Launchpad: https://launchpad.net/~YOUR_USERNAME/+editpgpkeys
   ```

## Update Package Info

Edit these files with your information:
- `debian/control`: Update Maintainer, Homepage
- `debian/copyright`: Update copyright holder
- `debian/changelog`: Update maintainer name/email

## Build and Upload

```bash
# Option 1: Use the upload script
./scripts/upload-ppa.sh YOUR_USERNAME/PPA_NAME 1.0.0-1~ppa1

# Option 2: Manual steps
dch -v 1.0.0-1~ppa1 "PPA upload"
debuild -S -sa
dput ppa:YOUR_USERNAME/PPA_NAME ../minlab_1.0.0-1~ppa1_source.changes
```

## Monitor Build

Visit: https://launchpad.net/~YOUR_USERNAME/+archive/ubuntu/PPA_NAME

## Users Install

```bash
sudo add-apt-repository ppa:YOUR_USERNAME/PPA_NAME
sudo apt-get update
sudo apt-get install minlab
```

## Update Package

```bash
# Increment version
dch -i  # or dch -v 1.0.1-1~ppa1

# Rebuild and upload
./scripts/upload-ppa.sh YOUR_USERNAME/PPA_NAME 1.0.1-1~ppa1
```

For detailed instructions, see [DEPLOYMENT.md](DEPLOYMENT.md).

