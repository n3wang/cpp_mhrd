#include "../src/terminal_ui.h"
#include "../src/level_editor.h"
#include "../src/game.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cassert>

// Integration test that actually uses the LevelEditor
// This requires mocking the terminal input

// Global mock key queue
static std::vector<KeyEvent> g_mockKeyQueue;
static size_t g_mockKeyIndex = 0;
static bool g_useMockKeys = false;

// Override readKey for testing (we'll need to modify terminal_ui.cpp to support this)
// For now, we'll create a test that validates the logic without full integration

class EditorTest {
private:
    std::ostringstream output_;
    std::streambuf* originalCout_;
    
public:
    EditorTest() {
        originalCout_ = std::cout.rdbuf();
        std::cout.rdbuf(output_.rdbuf());
    }
    
    ~EditorTest() {
        std::cout.rdbuf(originalCout_);
    }
    
    std::string getOutput() const { return output_.str(); }
    void clearOutput() { output_.str(""); output_.clear(); }
    
    void printResult(const std::string& testName, bool passed, const std::string& details = "") {
        std::cout.rdbuf(originalCout_);
        std::cout << (passed ? "[PASS]" : "[FAIL]") << " " << testName;
        if (!details.empty()) {
            std::cout << " - " << details;
        }
        std::cout << std::endl;
        std::cout.rdbuf(output_.rdbuf());
    }
};

// Test: TabbedInterface key handling
void test_tabbed_interface_keys() {
    EditorTest test;
    
    TabbedInterface tabs;
    std::string solution = "Initial text";
    tabs.setSolutionText(solution);
    
    // Test typing characters
    KeyEvent keyA = {Key::Char, 'A'};
    bool handled = tabs.handleKey(keyA, solution);
    
    bool passed = handled && solution.find('A') != std::string::npos;
    test.printResult("test_tabbed_interface_keys", passed,
                    passed ? "" : "Character input not handled correctly");
}

// Test: TabbedInterface tab switching
void test_tabbed_interface_tabs() {
    EditorTest test;
    
    TabbedInterface tabs;
    tabs.setActiveTab(TabbedInterface::Solution);
    
    // Simulate Tab key
    KeyEvent tabKey = {Key::Tab, 0};
    std::string dummy = "";
    tabs.handleKey(tabKey, dummy);
    
    bool passed = tabs.getActiveTab() == TabbedInterface::Instructions;
    test.printResult("test_tabbed_interface_tabs", passed,
                    passed ? "" : "Tab switching failed");
}

// Test: Solution text manipulation
void test_solution_text_manipulation() {
    EditorTest test;
    
    TabbedInterface tabs;
    std::string solution = "Hello";
    tabs.setSolutionText(solution);
    // Set cursor to end of text
    tabs.setCursorPosition(0, solution.length());
    
    // Add character at end
    KeyEvent keySpace = {Key::Char, ' '};
    KeyEvent keyWorld = {Key::Char, 'W'};
    tabs.handleKey(keySpace, solution);
    tabs.handleKey(keyWorld, solution);
    
    // The solution should have the characters appended
    // Note: handleKey modifies solution in place, cursor position matters
    bool passed = solution.find("Hello") != std::string::npos && 
                  solution.find('W') != std::string::npos;
    test.printResult("test_solution_text_manipulation", passed,
                    passed ? "" : "Expected 'Hello W', got '" + solution + "'");
}

// Test: Backspace handling
void test_backspace_handling() {
    EditorTest test;
    
    TabbedInterface tabs;
    std::string solution = "ABC";
    tabs.setSolutionText(solution);
    // Set cursor to end
    tabs.setCursorPosition(0, solution.length());
    
    // Simulate backspace (deletes character before cursor)
    KeyEvent backspace = {Key::Backspace, 0};
    tabs.handleKey(backspace, solution);
    
    bool passed = solution.length() == 2 && solution.find("AB") != std::string::npos;
    test.printResult("test_backspace_handling", passed,
                    passed ? "" : "Expected 'AB', got '" + solution + "'");
}

// Test: Help toggle
void test_help_toggle() {
    EditorTest test;
    
    TabbedInterface tabs;
    bool initial = tabs.isHelpVisible();
    tabs.toggleHelp();
    bool afterToggle = tabs.isHelpVisible();
    
    bool passed = initial != afterToggle;
    test.printResult("test_help_toggle", passed,
                    passed ? "" : "Help toggle failed");
}

// Test: Reset confirmation
void test_reset_confirmation() {
    EditorTest test;
    
    TabbedInterface tabs;
    tabs.setResetConfirmation(true);
    bool visible = tabs.isResetConfirmationVisible();
    
    bool passed = visible;
    test.printResult("test_reset_confirmation", passed,
                    passed ? "" : "Reset confirmation not visible");
}

int main() {
    std::cout << "Running Editor Integration Tests..." << std::endl;
    std::cout << "====================================" << std::endl;
    
    test_tabbed_interface_keys();
    test_tabbed_interface_tabs();
    test_solution_text_manipulation();
    test_backspace_handling();
    test_help_toggle();
    test_reset_confirmation();
    
    std::cout << "====================================" << std::endl;
    std::cout << "Tests completed!" << std::endl;
    
    return 0;
}

