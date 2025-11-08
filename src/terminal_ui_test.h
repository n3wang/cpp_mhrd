#ifndef TERMINAL_UI_TEST_H
#define TERMINAL_UI_TEST_H

#include "terminal_ui.h"
#include <string>
#include <vector>
#include <functional>
#include <sstream>

// Test harness for terminal UI
class TerminalUITest {
public:
    // Mock key sequence for testing
    struct MockKeySequence {
        std::vector<KeyEvent> keys;
        std::string description;
    };
    
    // Output capture
    class OutputCapture {
    public:
        std::ostringstream buffer;
        std::streambuf* original;
        
        OutputCapture();
        ~OutputCapture();
        std::string getOutput() const;
        void clear();
    };
    
    // Test result
    struct TestResult {
        std::string name;
        bool passed;
        std::string error;
        std::string output;
    };
    
    // Run a test that simulates key presses and checks output/state
    static TestResult runEditorTest(
        const std::string& testName,
        const std::vector<KeyEvent>& keySequence,
        std::function<bool(const std::string& output, const std::string& solutionText)> validator
    );
    
    // Helper to create key sequences
    static std::vector<KeyEvent> keySequence(std::initializer_list<std::pair<Key, char>> keys);
    
    // Common validators
    static bool containsText(const std::string& output, const std::string& text);
    static bool solutionContains(const std::string& solutionText, const std::string& text);
    static bool solutionEquals(const std::string& solutionText, const std::string& expected);
};

#endif // TERMINAL_UI_TEST_H

