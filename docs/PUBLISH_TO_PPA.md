# Publishing minlab to Ubuntu PPA - Step by Step

This guide will help you publish minlab so users can install it with:
```bash
sudo add-apt-repository ppa:YOUR_USERNAME/PPA_NAME
sudo apt-get update
sudo apt-get install minlab
```

## Prerequisites

### 1. Install Required Tools

```bash
sudo apt-get update
sudo apt-get install -y devscripts dput ubuntu-dev-tools
```

### 2. Create Launchpad Account

1. Go to https://launchpad.net/
2. Click "Log in / Register"
3. Create an account (or log in if you have one)
4. Verify your email address

### 3. Set Up GPG Key

You need a GPG key to sign your packages:

```bash
# Check if you already have a GPG key
gpg --list-secret-keys

# If not, create one
gpg --gen-key
# Choose: (1) RSA and RSA
# Key size: 4096
# Expiration: 0 (no expiration) or your preference
# Enter your name and email (MUST match your Launchpad account email)
# Enter a passphrase

# Get your key ID
gpg --list-secret-keys --keyid-format LONG
# Look for a line like: sec   rsa4096/ABCD1234EFGH5678 2025-01-01 [SC]

# Export your public key
gpg --armor --export YOUR_KEY_ID > ~/my-public-key.txt
cat ~/my-public-key.txt

# Upload to keyserver
gpg --send-keys --keyserver keyserver.ubuntu.com YOUR_KEY_ID
```

### 4. Upload GPG Key to Launchpad

1. Go to https://launchpad.net/~YOUR_USERNAME/+editpgpkeys
2. Paste your public key (from `~/my-public-key.txt`) or click "Import from keyserver"
3. Launchpad will send you an encrypted email
4. Decrypt the email:
```bash
# Save the email content to a file
gpg --decrypt email.txt
```
5. Copy the decrypted text and paste it back on Launchpad to verify

### 5. Create a PPA

1. Go to https://launchpad.net/~YOUR_USERNAME/+archive/ubuntu/ppa
2. Click "Create a new PPA"
3. Fill in:
   - **PPA name**: `minlab` (or `ppa` for default)
   - **Display name**: `minlab`
   - **Description**: `Interactive HDL puzzle game and circuit simulator`
4. Accept the terms and create

Your PPA URL will be: `ppa:YOUR_USERNAME/PPA_NAME`

## Publishing Steps

### Step 1: Update Package Information

Edit these files with your information:

```bash
# Update debian/control
nano debian/control
# Change: Maintainer, Homepage, Vcs URLs

# Update debian/copyright
nano debian/copyright
# Change: Copyright holder, Source URL

# Update debian/changelog
nano debian/changelog
# Change: Maintainer name and email
```

### Step 2: Update Version for PPA

```bash
cd /home/newang/git/cpp_mhrd

# Update changelog for PPA (use ~ppa1 suffix)
dch -v 1.0.0-1~ppa1 "Initial PPA release"

# Edit the changelog if needed
dch -e
```

The changelog should look like:
```
minlab (1.0.0-1~ppa1) focal; urgency=medium

  * Initial PPA release
  * Interactive game mode with level editor
  * Terminal UI with tabbed interface

 -- Your Name <your.email@example.com>  Mon, 08 Jan 2025 12:00:00 +0000
```

### Step 3: Build Source Package

```bash
# Clean previous builds
rm -rf ../minlab_*.*.*/
rm -f ../minlab_*.{deb,dsc,changes,build,upload}

# Build source package
debuild -S -sa

# This creates:
# - ../minlab_1.0.0-1~ppa1_source.changes
# - ../minlab_1.0.0-1~ppa1.dsc
# - ../minlab_1.0.0-1~ppa1.tar.xz
```

### Step 4: Sign the Package

```bash
# Sign the .changes file (you'll be prompted for your GPG passphrase)
debsign ../minlab_1.0.0-1~ppa1_source.changes
```

### Step 5: Upload to PPA

```bash
# Upload to your PPA
dput ppa:YOUR_USERNAME/PPA_NAME ../minlab_1.0.0-1~ppa1_source.changes

# Example:
# dput ppa:newang/minlab ../minlab_1.0.0-1~ppa1_source.changes
```

### Step 6: Monitor Build Status

1. Go to your PPA: https://launchpad.net/~YOUR_USERNAME/+archive/ubuntu/PPA_NAME
2. Click on the "Builds" tab
3. Wait for builds to complete (usually 10-30 minutes)
4. Check for any build errors

## Users Can Now Install

Once the build completes successfully, users can install with:

```bash
sudo add-apt-repository ppa:YOUR_USERNAME/PPA_NAME
sudo apt-get update
sudo apt-get install minlab
```

## Quick Script

You can also use the provided script:

```bash
./scripts/upload-ppa.sh YOUR_USERNAME/PPA_NAME 1.0.0-1~ppa1
```

## Updating the Package

When you want to release a new version:

```bash
# Update changelog
dch -i  # Increment version
# or
dch -v 1.0.1-1~ppa1 "New features"

# Edit changelog
dch -e

# Rebuild and upload
debuild -S -sa
debsign ../minlab_1.0.1-1~ppa1_source.changes
dput ppa:YOUR_USERNAME/PPA_NAME ../minlab_1.0.1-1~ppa1_source.changes
```

## Troubleshooting

### "GPG key not found"
- Make sure your GPG key is uploaded to Launchpad
- Verify the key is associated with your Launchpad account email

### "Invalid signature"
- Re-sign with `debsign`
- Make sure you're using the correct GPG key

### "PPA name wrong"
- Verify your PPA name: https://launchpad.net/~YOUR_USERNAME/+archives
- Use the exact name shown (case-sensitive)

### Build fails on Launchpad
- Check the build logs on Launchpad
- Common issues: missing dependencies, compilation errors
- Fix and upload a new version

## Next Steps After Publishing

1. **Update README.md** with installation instructions
2. **Create a release** on GitHub/GitLab (if applicable)
3. **Announce** the availability
4. **Monitor** for user feedback and issues

