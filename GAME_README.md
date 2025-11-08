# MHRD CLI Game

An interactive puzzle game where you build digital circuits using HDL (Hardware Description Language).

## Quick Start

Run the game without arguments to enter interactive mode:

```bash
./build/mhrd-cli
```

Or use the legacy mode to test a single HDL file:

```bash
./build/mhrd-cli examples/not.hdl
```

## Game Features

### Interactive Mode

When you run `mhrd-cli` without arguments, you'll see a menu with all available levels:

```
╔══════════════════════════════════════════════════════════╗
║              MHRD CLI - Level Selector                   ║
╚══════════════════════════════════════════════════════════╝

  [1] ✓ NOT Gate (Difficulty: 1) [COMPLETED]
  [2]   AND Gate (Difficulty: 1)
  [3]   XOR Gate (Difficulty: 2)
  [4]   Half Adder (Difficulty: 3)
  [5]   Majority Gate (Difficulty: 3)

  [0] Exit
```

### Level System

Each level provides:
- **Description**: What circuit you need to build
- **Inputs/Outputs**: The interface your circuit must have
- **Available Gates**: Which gates you can use
- **Expected Truth Table**: The correct output for all input combinations

### Solving Levels

1. Select a level from the menu
2. Choose to enter your solution directly or load from a file
3. Your solution is validated against the expected truth table
4. If correct, the level is marked as completed and saved

### Progress Tracking

Your progress is automatically saved to `.mhrd_progress.json` in the project root. Completed levels are marked with a ✓ in the menu.

## Levels

### Level 1: NOT Gate
Build a simple NOT gate that inverts the input.

**Available Gates**: `not`

### Level 2: AND Gate
Build an AND gate that outputs 1 only when both inputs are 1.

**Available Gates**: `and`

### Level 3: XOR Gate
Build an XOR gate using only AND, OR, and NOT gates. The output should be 1 when inputs differ.

**Available Gates**: `and`, `or`, `not`

### Level 4: Half Adder
Build a half adder with two outputs: `sum` (XOR of inputs) and `carry` (AND of inputs).

**Available Gates**: `and`, `or`, `not`, `xor`

### Level 5: Majority Gate
Build a majority gate with 3 inputs. Output is 1 when at least 2 inputs are 1.

**Available Gates**: `and`, `or`, `not`

## HDL Format

The HDL format is simple:

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

## Test Solutions

Example solutions for all levels are available in the `tests/` directory:
- `tests/level01_solution.hdl`
- `tests/level02_solution.hdl`
- `tests/level03_solution.hdl`
- `tests/level04_solution.hdl`
- `tests/level05_solution.hdl`

You can test these solutions using the "Test solution from file" option in the game menu.

## Adding New Levels

To add a new level, create a JSON file in the `levels/` directory:

```json
{
  "id": "level06",
  "name": "Level Name",
  "description": "Description of what to build",
  "difficulty": 2,
  "available_gates": ["and", "or", "not"],
  "inputs": ["a", "b"],
  "outputs": ["out"],
  "expected": [
    {"in": {"a": 0, "b": 0}, "out": {"out": 0}},
    {"in": {"a": 0, "b": 1}, "out": {"out": 1}},
    {"in": {"a": 1, "b": 0}, "out": {"out": 1}},
    {"in": {"a": 1, "b": 1}, "out": {"out": 0}}
  ]
}
```

The game will automatically load all `.json` files from the `levels/` directory.

