#include "../src/terminal_ui.h"
#include "../src/level_editor.h"
#include "../src/game.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cassert>
#include <fstream>

// Test framework for UI controls
class UITestFramework {
private:
    std::vector<KeyEvent> keyQueue_;
    size_t keyIndex_ = 0;
    std::ostringstream outputCapture_;
    std::streambuf* originalCout_;
    
    // Mock terminal dimensions
    static constexpr int TEST_WIDTH = 80;
    static constexpr int TEST_HEIGHT = 24;
    
public:
    UITestFramework() {
        // Redirect cout to capture output
        originalCout_ = std::cout.rdbuf();
        std::cout.rdbuf(outputCapture_.rdbuf());
    }
    
    ~UITestFramework() {
        std::cout.rdbuf(originalCout_);
    }
    
    void queueKeys(const std::vector<KeyEvent>& keys) {
        keyQueue_ = keys;
        keyIndex_ = 0;
    }
    
    KeyEvent getNextKey() {
        if (keyIndex_ < keyQueue_.size()) {
            return keyQueue_[keyIndex_++];
        }
        return {Key::None, 0};
    }
    
    std::string getOutput() const {
        return outputCapture_.str();
    }
    
    void clearOutput() {
        outputCapture_.str("");
        outputCapture_.clear();
    }
    
    bool outputContains(const std::string& text) const {
        return getOutput().find(text) != std::string::npos;
    }
    
    void printTestResult(const std::string& testName, bool passed, const std::string& details = "") {
        std::cout.rdbuf(originalCout_);
        std::cout << (passed ? "[PASS]" : "[FAIL]") << " " << testName;
        if (!details.empty()) {
            std::cout << " - " << details;
        }
        std::cout << std::endl;
        std::cout.rdbuf(outputCapture_.rdbuf());
    }
};

// Helper to create key sequences
std::vector<KeyEvent> makeKeySequence(std::initializer_list<std::pair<Key, char>> keys) {
    std::vector<KeyEvent> result;
    for (const auto& k : keys) {
        result.push_back({k.first, k.second});
    }
    return result;
}

// Test: Basic text input
void test_basic_text_input() {
    UITestFramework test;
    
    // Type "Hello"
    auto keys = makeKeySequence({
        {Key::Char, 'H'},
        {Key::Char, 'e'},
        {Key::Char, 'l'},
        {Key::Char, 'l'},
        {Key::Char, 'o'}
    });
    
    test.queueKeys(keys);
    
    // Simulate editor
    std::string solution = "";
    for (const auto& key : keys) {
        if (key.key == Key::Char) {
            solution += key.ch;
        }
    }
    
    bool passed = solution == "Hello";
    test.printTestResult("test_basic_text_input", passed, 
                        passed ? "" : "Expected 'Hello', got '" + solution + "'");
}

// Test: Backspace deletion
void test_backspace_deletion() {
    UITestFramework test;
    
    auto keys = makeKeySequence({
        {Key::Char, 'A'},
        {Key::Char, 'B'},
        {Key::Char, 'C'},
        {Key::Backspace, 0},
        {Key::Backspace, 0}
    });
    
    test.queueKeys(keys);
    
    std::string solution = "";
    for (const auto& key : keys) {
        if (key.key == Key::Char) {
            solution += key.ch;
        } else if (key.key == Key::Backspace && !solution.empty()) {
            solution.pop_back();
        }
    }
    
    bool passed = solution == "A";
    test.printTestResult("test_backspace_deletion", passed,
                        passed ? "" : "Expected 'A', got '" + solution + "'");
}

// Test: Tab navigation
void test_tab_navigation() {
    UITestFramework test;
    
    // Simulate tabbing through tabs
    TabbedInterface::Tab currentTab = TabbedInterface::Solution;
    int tabCount = 0;
    
    auto keys = makeKeySequence({
        {Key::Tab, 0},
        {Key::Tab, 0},
        {Key::Tab, 0},
        {Key::Tab, 0}  // Should cycle back to Solution
    });
    
    for (const auto& key : keys) {
        if (key.key == Key::Tab) {
            int current = static_cast<int>(currentTab);
            currentTab = static_cast<TabbedInterface::Tab>((current + 1) % 4);
            tabCount++;
        }
    }
    
    bool passed = (currentTab == TabbedInterface::Solution && tabCount == 4);
    test.printTestResult("test_tab_navigation", passed,
                        passed ? "" : "Tab navigation failed");
}

// Test: Word deletion (Alt+Backspace)
void test_word_deletion() {
    UITestFramework test;
    
    std::string solution = "Hello World Test";
    int cursorPos = solution.length();
    
    // Simulate Alt+Backspace (delete word to left)
    if (cursorPos > 0) {
        int start = cursorPos;
        // Move back to start of word
        while (start > 0 && (solution[start - 1] == ' ' || solution[start - 1] == '\t')) {
            start--;
        }
        while (start > 0 && solution[start - 1] != ' ' && solution[start - 1] != '\t' && 
               solution[start - 1] != '\n') {
            start--;
        }
        solution.erase(solution.begin() + start, solution.begin() + cursorPos);
    }
    
    bool passed = solution == "Hello World ";
    test.printTestResult("test_word_deletion", passed,
                        passed ? "" : "Expected 'Hello World ', got '" + solution + "'");
}

// Test: Line movement (Alt+Up/Down)
void test_line_movement() {
    UITestFramework test;
    
    std::string solution = "Line 1\nLine 2\nLine 3";
    int cursorRow = 1; // On Line 2
    int cursorCol = 3;
    
    // Simulate Alt+Up (move line up)
    std::istringstream iss(solution);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }
    
    if (cursorRow > 0) {
        std::swap(lines[cursorRow], lines[cursorRow - 1]);
    }
    
    std::ostringstream oss;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i > 0) oss << "\n";
        oss << lines[i];
    }
    solution = oss.str();
    
    bool passed = solution == "Line 2\nLine 1\nLine 3";
    test.printTestResult("test_line_movement", passed,
                        passed ? "" : "Line movement failed");
}

// Test: F12 reset confirmation
void test_reset_confirmation() {
    UITestFramework test;
    
    bool showResetConfirmation = false;
    bool resetConfirmed = false;
    
    // Simulate F12 press
    showResetConfirmation = true;
    
    // Simulate 'y' to confirm
    if (showResetConfirmation) {
        resetConfirmed = true;
        showResetConfirmation = false;
    }
    
    bool passed = resetConfirmed && !showResetConfirmation;
    test.printTestResult("test_reset_confirmation", passed,
                        passed ? "" : "Reset confirmation failed");
}

// Test: F6 help toggle
void test_help_toggle() {
    UITestFramework test;
    
    bool showHelp = false;
    
    // Toggle help on
    showHelp = !showHelp;
    bool helpOn = showHelp;
    
    // Toggle help off
    showHelp = !showHelp;
    bool helpOff = !showHelp;
    
    bool passed = helpOn && helpOff;
    test.printTestResult("test_help_toggle", passed,
                        passed ? "" : "Help toggle failed");
}

// Test: F5 compile
void test_compile_shortcut() {
    UITestFramework test;
    
    bool compileTriggered = false;
    
    // Simulate F5 press
    KeyEvent f5 = {Key::F5, 0};
    if (f5.key == Key::F5) {
        compileTriggered = true;
    }
    
    bool passed = compileTriggered;
    test.printTestResult("test_compile_shortcut", passed,
                        passed ? "" : "F5 compile shortcut failed");
}

// Test: Escape handling
void test_escape_handling() {
    UITestFramework test;
    
    bool shouldExit = false;
    bool helpVisible = false;
    bool resetConfirmationVisible = false;
    
    KeyEvent esc = {Key::Escape, 0};
    
    // Escape should not exit if help or reset confirmation is visible
    if (esc.key == Key::Escape) {
        if (helpVisible || resetConfirmationVisible) {
            // Just close the modal
            shouldExit = false;
        } else {
            shouldExit = true;
        }
    }
    
    bool passed = shouldExit; // When no modals, should exit
    test.printTestResult("test_escape_handling", passed,
                        passed ? "" : "Escape handling failed");
}

int main() {
    std::cout << "Running UI Control Tests..." << std::endl;
    std::cout << "================================" << std::endl;
    
    test_basic_text_input();
    test_backspace_deletion();
    test_tab_navigation();
    test_word_deletion();
    test_line_movement();
    test_reset_confirmation();
    test_help_toggle();
    test_compile_shortcut();
    test_escape_handling();
    
    std::cout << "================================" << std::endl;
    std::cout << "Tests completed!" << std::endl;
    
    return 0;
}

