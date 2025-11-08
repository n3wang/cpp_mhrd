# mhrd-cli

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
g++ -std=c++17 -O2 -pipe -o build/mhrd-cli src/mhrd_cli.cpp
```

### Windows (Visual Studio/MSVC)

```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

The executable will be at `build\Release\mhrd-cli.exe`.

Or using MinGW (if installed):

```bash
mkdir -p build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

## Usage

### Linux/Unix

```bash
./build/mhrd-cli examples/not.hdl
```

### Windows

```cmd
build\Release\mhrd-cli.exe examples\not.hdl
```

Or from the build\Release directory:

```cmd
cd build\Release
mhrd-cli.exe ..\..\examples\not.hdl
```

Example output:
```
in {in:0} -> out {out:1}
in {in:1} -> out {out:0}
```

## HDL Format

The HDL format is simple and consists of four sections:

```
Inputs: in1, in2;
Outputs: out;
Parts: gate1:and, gate2:or;
Wires: in1->gate1.in1, in2->gate1.in2, gate1.out->gate2.in1, gate2.out->out;
```

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
../mhrd-cli_0.1.0-1_*.deb
```

### Installing the Package

```bash
sudo dpkg -i ../mhrd-cli_0.1.0-1_*.deb
```

After installation, you can run:
```bash
mhrd-cli /usr/share/mhrd-cli/examples/not.hdl
```

## Publishing via PPA (Ubuntu)

### 1. Create a PPA at Launchpad

Visit https://launchpad.net/ and create a new PPA.

### 2. Update Changelog for PPA

```bash
dch -v 0.1.0-1ppa1 -D noble "PPA build"
```

### 3. Build Source Package

```bash
debuild -S -sa
```

### 4. Upload to PPA

```bash
sudo apt-get install -y dput
dput ppa:YOURLPID/YOURPPA ../mhrd-cli_0.1.0-1ppa1_source.changes
```

Replace `YOURLPID` and `YOURPPA` with your Launchpad ID and PPA name.

### 5. Users Install via apt

```bash
sudo add-apt-repository ppa:YOURLPID/YOURPPA
sudo apt-get update
sudo apt-get install mhrd-cli
```

## Project Structure

```
mhrd-cli/
├── src/
│   └── mhrd_cli.cpp      # Main C++ source
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

## License

[Add your license here]

## Contributing

[Add contribution guidelines here]

