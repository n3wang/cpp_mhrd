# Testing the minlab Package

## Quick Test Instructions

### 1. Install the Package

```bash
cd /home/newang/git/cpp_mhrd
sudo dpkg -i ../minlab_1.0.0-1_amd64.deb
```

If you get dependency errors, fix them with:
```bash
sudo apt-get install -f
```

### 2. Verify Installation

```bash
# Check if minlab is installed
which minlab

# Check package info
dpkg -l minlab

# List installed files
dpkg -L minlab
```

### 3. Test the Application

```bash
# Run minlab (interactive mode)
minlab

# Or test with an example file
minlab /usr/share/minlab/examples/not.hdl
```

### 4. Verify Files Are in Place

```bash
# Check binary
ls -lh /usr/bin/minlab

# Check examples
ls -lh /usr/share/minlab/examples/

# Check levels
ls -lh /usr/share/minlab/levels/
```

### 5. Uninstall (if needed)

```bash
sudo apt-get remove minlab
```

## Expected Behavior

When you run `minlab`:
- You should see a menu with level selection
- You can navigate with arrow keys or numbers
- You can select a level to play
- The game should work with all keyboard shortcuts

## Troubleshooting

### Package won't install
- Check dependencies: `dpkg-deb -I ../minlab_1.0.0-1_amd64.deb | grep Depends`
- Install missing dependencies manually if needed

### Application doesn't run
- Check if it's in PATH: `which minlab`
- Check permissions: `ls -l /usr/bin/minlab`
- Try running directly: `/usr/bin/minlab`

### Missing files
- Verify installation: `dpkg -L minlab`
- Check if files exist: `ls -la /usr/share/minlab/`

