# minlab

A terminal-based HDL (Hardware Description Language) puzzle simulator inspired by MHRD. This minimal CLI tool parses a small HDL format and prints truth tables for combinational circuits.

## Features

- Parses a minimal HDL format with inputs, outputs, parts (gates), and wires
- Supports basic gates: NOT, AND, OR, XOR, NAND, NOR
- Simulates combinational circuits and generates truth tables
- Ready for Debian/Ubuntu packaging and distribution via apt

## Building

### Linux/Unix (GCC/Clang)

```bash
mkdir -p build
cd build
cmake ..
make
```

Or using g++ directly:

```bash
mkdir -p build
g++ -std=c++17 -O2 -pipe -o build/minlab src/minlab.cpp
```

Compiling and uploading

cd /home/newang/git/cpp_mhrd
sudo dpkg -i ../minlab_1.0.0-1_amd64.deb
sudo apt-get install -f  # Fix any dependency issues if needed
minlab  # Test it


### Supported Gates

- `not` - NOT gate (1 input: `in`, 1 output: `out`)
- `and` - AND gate (2 inputs: `in1`, `in2`, 1 output: `out`)
- `or` - OR gate (2 inputs: `in1`, `in2`, 1 output: `out`)
- `xor` - XOR gate (2 inputs: `in1`, `in2`, 1 output: `out`)
- `nand` - NAND gate (2 inputs: `in1`, `in2`, 1 output: `out`)
- `nor` - NOR gate (2 inputs: `in1`, `in2`, 1 output: `out`)

## Debian Packaging

### Prerequisites

```bash
sudo apt-get update
sudo apt-get install -y build-essential debhelper cmake devscripts
```

### Building a .deb Package

```bash
dpkg-buildpackage -us -uc
```

This will create a `.deb` file in the parent directory:
```
../minlab_0.1.0-1_*.deb
```

### Installing the Package

```bash
sudo dpkg -i ../minlab_0.1.0-1_*.deb
```

After installation, you can run:
```bash
minlab /usr/share/minlab/examples/not.hdl
```

## Publishing via PPA (Ubuntu)

### Quick Start

See [PPA_QUICK_START.md](PPA_QUICK_START.md) for a condensed guide.

### Detailed Instructions

See [DEPLOYMENT.md](DEPLOYMENT.md) for comprehensive deployment instructions.

### Quick Commands

```bash
# Build package
./scripts/build-package.sh 1.0.0-1

# Upload to PPA
./scripts/upload-ppa.sh YOUR_USERNAME/PPA_NAME 1.0.0-1~ppa1

# Users install
sudo add-apt-repository ppa:YOUR_USERNAME/PPA_NAME
sudo apt-get update
sudo apt-get install minlab
```

## Project Structure

```
minlab/
├── src/
│   └── minlab.cpp      # Main C++ source
├── examples/
│   └── not.hdl           # Example HDL file
├── debian/
│   ├── control           # Package metadata
│   ├── rules             # Build rules
│   ├── changelog         # Version history
│   ├── install           # Installation rules
│   └── source/
│       └── format        # Source format
├── CMakeLists.txt        # CMake build configuration
└── README.md             # This file
```

