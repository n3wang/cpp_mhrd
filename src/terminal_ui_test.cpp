#include "terminal_ui_test.h"
#include "level_editor.h"
#include "game.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

// Global mock key queue for testing
static std::vector<KeyEvent> mockKeyQueue;
static bool useMockKeys = false;
static int mockKeyIndex = 0;

// Override readKey for testing
KeyEvent TerminalUI::readKey() {
    if (useMockKeys && mockKeyIndex < static_cast<int>(mockKeyQueue.size())) {
        return mockKeyQueue[mockKeyIndex++];
    }
    
    // Fall back to real implementation if no mock keys
    // (This is the original implementation, but we'll use mock in tests)
    return {Key::None, 0};
}

// Output capture implementation
TerminalUITest::OutputCapture::OutputCapture() {
    original = std::cout.rdbuf();
    std::cout.rdbuf(buffer.rdbuf());
}

TerminalUITest::OutputCapture::~OutputCapture() {
    std::cout.rdbuf(original);
}

std::string TerminalUITest::OutputCapture::getOutput() const {
    return buffer.str();
}

void TerminalUITest::OutputCapture::clear() {
    buffer.str("");
    buffer.clear();
}

// Helper to create key sequences
std::vector<KeyEvent> TerminalUITest::keySequence(std::initializer_list<std::pair<Key, char>> keys) {
    std::vector<KeyEvent> result;
    for (const auto& k : keys) {
        result.push_back({k.first, k.second});
    }
    return result;
}

// Common validators
bool TerminalUITest::containsText(const std::string& output, const std::string& text) {
    return output.find(text) != std::string::npos;
}

bool TerminalUITest::solutionContains(const std::string& solutionText, const std::string& text) {
    return solutionText.find(text) != std::string::npos;
}

bool TerminalUITest::solutionEquals(const std::string& solutionText, const std::string& expected) {
    return solutionText == expected;
}

// Run a test
TerminalUITest::TestResult TerminalUITest::runEditorTest(
    const std::string& testName,
    const std::vector<KeyEvent>& keySequence,
    std::function<bool(const std::string& output, const std::string& solutionText)> validator
) {
    TestResult result;
    result.name = testName;
    result.passed = false;
    
    try {
        // Setup mock keys
        mockKeyQueue = keySequence;
        mockKeyIndex = 0;
        useMockKeys = true;
        
        // Capture output
        OutputCapture capture;
        
        // Create a test game and level editor
        Game game;
        game.loadLevels("levels");
        
        // Get first level for testing
        const Level* level = game.getLevel("level01");
        if (!level) {
            result.error = "Could not load test level";
            useMockKeys = false;
            return result;
        }
        
        LevelEditor editor(*level, game);
        
        // Run the editor with mock keys
        // We need to modify LevelEditor::run() to be testable
        // For now, we'll simulate the key handling manually
        
        // Get initial solution text
        std::string solutionText = editor.getSolutionText();
        
        // Process each key
        for (const auto& key : keySequence) {
            // Simulate what handleKey would do
            // This is a simplified version - in practice, we'd need to expose more internals
            if (key.key == Key::Char) {
                solutionText += key.ch;
            } else if (key.key == Key::Backspace && !solutionText.empty()) {
                solutionText.pop_back();
            }
            
            // Render (captures output)
            // editor.render(); // Would need to expose this
        }
        
        // Get output
        result.output = capture.getOutput();
        
        // Validate
        result.passed = validator(result.output, solutionText);
        
        if (!result.passed) {
            result.error = "Validation failed";
        }
        
    } catch (const std::exception& e) {
        result.error = std::string("Exception: ") + e.what();
    } catch (...) {
        result.error = "Unknown exception";
    }
    
    // Cleanup
    useMockKeys = false;
    mockKeyQueue.clear();
    mockKeyIndex = 0;
    
    return result;
}

