# Packaging Checklist

Use this checklist before publishing to PPA.

## Pre-Publishing Checklist

### Package Information
- [ ] Update `debian/control`:
  - [ ] Maintainer name and email
  - [ ] Homepage URL
  - [ ] VCS URLs (if applicable)
  - [ ] Package description
- [ ] Update `debian/copyright`:
  - [ ] Copyright holder
  - [ ] License information
  - [ ] Year
- [ ] Update `debian/changelog`:
  - [ ] Version number
  - [ ] Release notes
  - [ ] Maintainer name and email
  - [ ] Date

### Code Quality
- [ ] All tests pass: `./tests/run_tests.sh`
- [ ] Code compiles without warnings
- [ ] No hardcoded paths (use relative paths or install paths)
- [ ] Application works after installation

### Build Verification
- [ ] Package builds successfully: `./scripts/build-package.sh`
- [ ] Lintian passes: `lintian ../minlab_*.deb`
- [ ] Package installs: `sudo dpkg -i ../minlab_*.deb`
- [ ] Application runs: `minlab --help` (or just `minlab`)
- [ ] Files are in correct locations:
  - [ ] Binary in `/usr/bin/minlab`
  - [ ] Examples in `/usr/share/minlab/examples/`
  - [ ] Levels in `/usr/share/minlab/levels/`

### Documentation
- [ ] README.md is up to date
- [ ] DEPLOYMENT.md has correct information
- [ ] PPA_QUICK_START.md is accurate
- [ ] All placeholder values replaced (YOUR_USERNAME, etc.)

### Launchpad Setup
- [ ] Launchpad account created
- [ ] GPG key uploaded to Launchpad
- [ ] GPG key verified (encrypted email received and decrypted)
- [ ] PPA created on Launchpad
- [ ] PPA name noted for upload command

### Final Checks
- [ ] Version number incremented appropriately
- [ ] Changelog entry is descriptive
- [ ] No sensitive information in package
- [ ] Git repository is clean (or .gitignore excludes build artifacts)

## Publishing Steps

1. [ ] Update version: `dch -v X.Y.Z-1~ppa1`
2. [ ] Edit changelog: `dch -e`
3. [ ] Build source package: `debuild -S -sa`
4. [ ] Review generated files
5. [ ] Upload: `dput ppa:USERNAME/PPA_NAME ../minlab_*_source.changes`
6. [ ] Monitor build: https://launchpad.net/~USERNAME/+archive/ubuntu/PPA_NAME
7. [ ] Test installation from PPA on clean system

## Post-Publishing

- [ ] Update README with installation instructions
- [ ] Create release notes (if using GitHub/GitLab)
- [ ] Announce release to users
- [ ] Monitor for issues

