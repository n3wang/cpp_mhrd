# UI Control Testing

This directory contains automated tests for the UI editor controls. These tests allow you to verify that keyboard shortcuts, text editing, and UI interactions work correctly without manually testing each feature.

## Quick Start

Run all tests with a single command:

```bash
./tests/run_tests.sh
```

Or run tests individually:

### Basic UI Control Tests
Tests the core UI control logic (key handling, text manipulation, etc.):

```bash
cd build
./test-ui-controls
```

### Editor Integration Tests
Tests the TabbedInterface and LevelEditor integration:

```bash
cd build
./test-editor-integration
```

## Test Coverage

The tests cover:

1. **Basic Text Input**: Character insertion
2. **Backspace Deletion**: Character deletion
3. **Tab Navigation**: Switching between tabs
4. **Word Deletion**: Alt+Backspace functionality
5. **Line Movement**: Alt+Up/Down for moving lines
6. **Reset Confirmation**: F12 reset with confirmation
7. **Help Toggle**: F6 help screen toggle
8. **Compile Shortcut**: F5 compilation trigger
9. **Escape Handling**: Proper escape key behavior

## Adding New Tests

To add a new test:

1. Add a test function in `test_ui_controls.cpp` or `test_editor_integration.cpp`
2. Use the `UITestFramework` or `EditorTest` helper classes
3. Call the test function from `main()`
4. Rebuild and run

Example:

```cpp
void test_my_feature() {
    UITestFramework test;
    
    // Setup test
    auto keys = makeKeySequence({
        {Key::Char, 'A'},
        {Key::Char, 'B'}
    });
    
    // Execute test logic
    // ...
    
    // Verify results
    bool passed = /* your check */;
    test.printTestResult("test_my_feature", passed);
}
```

## Future Improvements

- Mock terminal input/output for full integration testing
- Visual regression testing (capture and compare screen output)
- Performance testing (measure response time for key presses)
- Stress testing (rapid key sequences, large text input)

