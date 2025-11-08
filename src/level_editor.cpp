#include "level_editor.h"
#include "simulator.h"
#include "terminal_ui.h"
#include <sstream>
#include <iomanip>
#include <set>
#include <algorithm>
#include <filesystem>

LevelEditor::LevelEditor(Game& game, const Level& level) 
    : game_(game), level_(level), historyIndex_(-1) {
    // Load saved solution if available, otherwise use template
    solutionText_ = game_.loadSolution(level_.id);
    if (solutionText_.empty()) {
        solutionText_ = generateTemplate();
    }
    updateInstructions();
    updateStats();
    tabs_.setSolutionText(solutionText_);
    tabs_.setActiveTab(TabbedInterface::Solution);
}

void LevelEditor::updateInstructions() {
    std::ostringstream oss;
    oss << "╔══════════════════════════════════════════════════════════╗\n";
    oss << "║  " << level_.name;
    for (size_t i = level_.name.length(); i < 52; ++i) oss << " ";
    oss << "║\n";
    oss << "╚══════════════════════════════════════════════════════════╝\n\n";
    
    oss << "Description: " << level_.description << "\n\n";
    oss << "Difficulty: " << level_.difficulty << "\n\n";
    
    oss << "Inputs: ";
    for (size_t i = 0; i < level_.inputs.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << level_.inputs[i];
    }
    oss << "\n\n";
    
    oss << "Outputs: ";
    for (size_t i = 0; i < level_.outputs.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << level_.outputs[i];
    }
    oss << "\n\n";
    
    oss << "Available Gates: ";
    for (size_t i = 0; i < level_.available_gates.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << level_.available_gates[i];
    }
    oss << "\n\n";
    
    oss << "Expected Truth Table:\n";
    for (const auto& testCase : level_.expected) {
        const auto& inVec = testCase.at("in");
        const auto& outVec = testCase.at("out");
        
        oss << "  in {";
        bool first = true;
        for (const auto& [k, v] : inVec) {
            if (!first) oss << ", ";
            first = false;
            oss << k << ":" << v;
        }
        oss << "} -> out {";
        first = true;
        for (const auto& [k, v] : outVec) {
            if (!first) oss << ", ";
            first = false;
            oss << k << ":" << v;
        }
        oss << "}\n";
    }
    
    oss << "\n";
    oss << "💡 Template: A starter template with the correct structure is provided\n";
    oss << "   in the Solution tab. Modify it to complete the circuit.\n";
    
    tabs_.setInstructionsText(oss.str());
}

void LevelEditor::updateStats() {
    std::ostringstream oss;
    oss << "Level Statistics\n";
    oss << "══════════════════════════════════════════════════════════\n\n";
    oss << "Level ID: " << level_.id << "\n";
    oss << "Difficulty: " << level_.difficulty << "\n";
    oss << "Status: " << (game_.isCompleted(level_.id) ? "✓ Completed" : "Not completed") << "\n\n";
    
    oss << "Circuit Requirements:\n";
    oss << "  Inputs: " << level_.inputs.size() << "\n";
    oss << "  Outputs: " << level_.outputs.size() << "\n";
    oss << "  Available Gates: " << level_.available_gates.size() << "\n";
    oss << "  Test Cases: " << level_.expected.size() << "\n";
    
    tabs_.setStatsText(oss.str());
}

void LevelEditor::updateHistory() {
    std::ostringstream oss;
    oss << "Code History\n";
    oss << "══════════════════════════════════════════════════════════\n\n";
    
    if (history_.empty()) {
        oss << "No previous code saved.\n";
    } else {
        oss << "Last " << history_.size() << " saved versions:\n\n";
        for (size_t i = 0; i < history_.size(); ++i) {
            oss << "[" << (i + 1) << "] ";
            if (i == history_.size() - 1) oss << "(Current) ";
            oss << "\n";
            oss << history_[i];
            oss << "\n──────────────────────────────────────────────────────────\n\n";
        }
    }
    
    if (!getLastWorkedCode().empty()) {
        oss << "\nLast Worked Code:\n";
        oss << "──────────────────────────────────────────────────────────\n";
        oss << getLastWorkedCode();
    }
    
    tabs_.setHistoryText(oss.str());
}

void LevelEditor::addToHistory(const std::string& code) {
    if (!code.empty() && (history_.empty() || history_.back() != code)) {
        history_.push_back(code);
        if (history_.size() > 10) { // Keep last 10 versions
            history_.erase(history_.begin());
        }
        historyIndex_ = history_.size() - 1;
        updateHistory();
    }
}

std::string LevelEditor::getLastWorkedCode() const {
    if (history_.empty()) return "";
    return history_.back();
}

std::string LevelEditor::generateTemplate() const {
    std::ostringstream oss;
    
    // Inputs section
    oss << "Inputs: ";
    for (size_t i = 0; i < level_.inputs.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << level_.inputs[i];
    }
    oss << ";\n";
    
    // Outputs section
    oss << "Outputs: ";
    for (size_t i = 0; i < level_.outputs.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << level_.outputs[i];
    }
    oss << ";\n";
    
    // Parts section - provide example parts based on available gates
    oss << "Parts: ";
    if (!level_.available_gates.empty()) {
        // Create example parts for each available gate type
        std::vector<std::string> partNames;
        for (size_t i = 0; i < level_.available_gates.size() && i < 3; ++i) {
            std::string gateName = level_.available_gates[i];
            std::string partName = "g" + std::to_string(i + 1);
            partNames.push_back(partName + ":" + gateName);
        }
        for (size_t i = 0; i < partNames.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << partNames[i];
        }
    }
    oss << ";\n";
    
    // Wires section - provide example wiring
    oss << "Wires: ";
    if (!level_.available_gates.empty() && !level_.inputs.empty() && !level_.outputs.empty()) {
        // Wire first input to first gate
        std::string firstGate = "g1";
        std::string firstInput = level_.inputs[0];
        
        // Determine gate input pin name based on gate type
        std::string gateType = level_.available_gates[0];
        std::string inPin = "in1";
        if (gateType == "not") {
            inPin = "in";
        }
        
        oss << firstInput << "->" << firstGate << "." << inPin;
        
        // If there are multiple inputs and gates, wire second input to second gate or first gate's second input
        if (level_.inputs.size() > 1 && level_.available_gates.size() > 0) {
            if (gateType != "not" && level_.inputs.size() > 1) {
                oss << ", " << level_.inputs[1] << "->" << firstGate << ".in2";
            } else if (level_.available_gates.size() > 1) {
                std::string secondGate = "g2";
                std::string secondInPin = "in1";
                if (level_.available_gates[1] == "not") {
                    secondInPin = "in";
                }
                oss << ", " << level_.inputs[1] << "->" << secondGate << "." << secondInPin;
            }
        }
        
        // Wire gate output to first output
        oss << ", " << firstGate << ".out->" << level_.outputs[0];
        
        // If there are multiple outputs, wire additional gates
        if (level_.outputs.size() > 1 && level_.available_gates.size() > 1) {
            oss << ", g2.out->" << level_.outputs[1];
        }
    }
    oss << ";\n";
    
    // Add helpful comments
    oss << "\n// TODO: Complete the circuit to match the expected truth table\n";
    oss << "// Available gates: ";
    for (size_t i = 0; i < level_.available_gates.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << level_.available_gates[i];
    }
    oss << "\n";
    
    return oss.str();
}

void LevelEditor::resetToTemplate() {
    solutionText_ = generateTemplate();
    tabs_.setSolutionText(solutionText_);
    tabs_.clearError();
    tabs_.clearSuccess();
    tabs_.render();
}

void LevelEditor::compileAndTest() {
    tabs_.clearError();
    tabs_.clearSuccess();
    
    // Check syntax first
    SyntaxError syntaxError = checkSyntax(solutionText_);
    if (syntaxError.hasError) {
        std::string errorMsg = "Syntax Error at Line " + std::to_string(syntaxError.line) + ": " + syntaxError.message;
        if (!syntaxError.lineContent.empty()) {
            // Trim the line content for display
            std::string trimmedLine = syntaxError.lineContent;
            while (!trimmedLine.empty() && (trimmedLine[0] == ' ' || trimmedLine[0] == '\t')) {
                trimmedLine = trimmedLine.substr(1);
            }
            if (trimmedLine.length() > 60) {
                trimmedLine = trimmedLine.substr(0, 57) + "...";
            }
            errorMsg += "\n  Line " + std::to_string(syntaxError.line) + ": " + trimmedLine;
        }
        tabs_.setError(errorMsg);
        tabs_.render();
        return;
    }
    
    // Helper function to find line number for a given string
    auto findLineNumber = [&](const std::string& searchText) -> int {
        std::istringstream iss(solutionText_);
        std::string line;
        int lineNum = 1;
        while (std::getline(iss, line)) {
            // Remove comments for matching
            std::string lineWithoutComments = line;
            size_t commentPos = lineWithoutComments.find("//");
            if (commentPos != std::string::npos) {
                lineWithoutComments = lineWithoutComments.substr(0, commentPos);
            }
            if (lineWithoutComments.find(searchText) != std::string::npos) {
                return lineNum;
            }
            lineNum++;
        }
        return 0;
    };
    
    // Try to compile and build test table
    try {
        AST ast = parseHDL(solutionText_);
        Net net = buildNet(ast);
        
        // Check inputs
        std::set<std::string> userInputs(ast.inputs.begin(), ast.inputs.end());
        std::set<std::string> expectedInputs(level_.inputs.begin(), level_.inputs.end());
        if (userInputs != expectedInputs) {
            int lineNum = findLineNumber("Inputs:");
            std::string errorMsg = "Input mismatch: Expected different inputs";
            if (lineNum > 0) {
                errorMsg += " (Line " + std::to_string(lineNum) + ")";
            }
            tabs_.setError(errorMsg);
            tabs_.render();
            return;
        }
        
        // Check outputs
        std::set<std::string> userOutputs(ast.outputs.begin(), ast.outputs.end());
        std::set<std::string> expectedOutputs(level_.outputs.begin(), level_.outputs.end());
        if (userOutputs != expectedOutputs) {
            int lineNum = findLineNumber("Outputs:");
            std::string errorMsg = "Output mismatch: Expected different outputs";
            if (lineNum > 0) {
                errorMsg += " (Line " + std::to_string(lineNum) + ")";
            }
            tabs_.setError(errorMsg);
            tabs_.render();
            return;
        }
        
        // Check gates
        std::set<std::string> availableGates(level_.available_gates.begin(), level_.available_gates.end());
        for (const auto& part : ast.parts) {
            std::string kindLower = part.kind;
            std::transform(kindLower.begin(), kindLower.end(), kindLower.begin(), ::tolower);
            if (availableGates.find(kindLower) == availableGates.end()) {
                // Find the line with this gate
                int lineNum = findLineNumber(part.name + ":" + part.kind);
                if (lineNum == 0) {
                    lineNum = findLineNumber(part.kind);
                }
                if (lineNum == 0) {
                    lineNum = findLineNumber("Parts:");
                }
                std::string errorMsg = "Invalid gate used: " + part.kind + " (not available in this level)";
                if (lineNum > 0) {
                    errorMsg += " (Line " + std::to_string(lineNum) + ")";
                }
                tabs_.setError(errorMsg);
                tabs_.render();
                return;
            }
        }
        
        // Build comparison table - ALWAYS show test results
        std::ostringstream tableMsg;
        tableMsg << "Test Results Comparison:\n\n";
        
        Table table;
        table.setMaxWidth(TerminalUI::getWidth() - 4);
        
        // Build header
        std::vector<std::string> headers = {"#", "Status"};
        for (const auto& in : level_.inputs) headers.push_back("in." + in);
        for (const auto& out : level_.outputs) {
            headers.push_back("out." + out + " (exp)");
            headers.push_back("out." + out + " (got)");
        }
        table.addHeader(headers);
        
        // Set column alignments: # and numeric columns right-aligned, Status center
        table.setColumnAlignment(0, 1); // # right-aligned
        table.setColumnAlignment(1, 0); // Status center-aligned
        // Input and output columns right-aligned (numeric)
        for (size_t i = 2; i < headers.size(); ++i) {
            table.setColumnAlignment(static_cast<int>(i), 1); // Right-align numeric columns
        }
        
        // Add test case rows
        int testNum = 1;
        int passed = 0;
        int failed = 0;
        
        for (const auto& testCase : level_.expected) {
            const auto& inVec = testCase.at("in");
            const auto& expectedOut = testCase.at("out");
            
            auto actualOut = simulate(net, inVec);
            
            // Check if this test case passes
            bool testPasses = true;
            for (const auto& [key, expectedVal] : expectedOut) {
                if (actualOut.find(key) == actualOut.end() || actualOut.at(key) != expectedVal) {
                    testPasses = false;
                    break;
                }
            }
            
            if (testPasses) passed++;
            else failed++;
            
            // Build row
            std::vector<std::string> row;
            row.push_back(std::to_string(testNum));
            row.push_back(testPasses ? "✓ PASS" : "✗ FAIL");
            
            // Input values
            for (const auto& in : level_.inputs) {
                row.push_back(std::to_string(inVec.at(in)));
            }
            
            // Output values (expected and actual)
            for (const auto& out : level_.outputs) {
                int expVal = expectedOut.at(out);
                int actVal = (actualOut.find(out) != actualOut.end()) ? actualOut.at(out) : -1;
                row.push_back(std::to_string(expVal));
                if (expVal == actVal) {
                    row.push_back(std::to_string(actVal));
                } else {
                    row.push_back(std::to_string(actVal) + " ←");
                }
            }
            
            table.addRow(row);
            testNum++;
        }
        
        tableMsg << table.render();
        tableMsg << "\nSummary: " << passed << " passed, " << failed << " failed out of " << level_.expected.size() << " tests";
        
        // If all tests passed, add success message to the table output
        if (failed == 0 && passed == static_cast<int>(level_.expected.size())) {
            tableMsg << "\n\n✓ SUCCESS! Your solution is correct!";
            game_.markCompleted(level_.id);
            addToHistory(solutionText_);
            updateStats();
        }
        
        // Always show the test table (with success message appended if all passed)
        tabs_.setError(tableMsg.str());
    } catch (const std::runtime_error& e) {
        // Try to extract line number from error message or find it
        std::string errorMsg = e.what();
        std::string fullError = "Error: " + errorMsg;
        
        // Try to find line number by searching for problematic content
        std::istringstream iss(solutionText_);
        std::string line;
        int lineNum = 1;
        bool foundLine = false;
        
        // Look for common error patterns and try to find the line
        std::string searchPattern = "";
        if (errorMsg.find("Bad part:") != std::string::npos) {
            size_t start = errorMsg.find("Bad part:") + 9;
            searchPattern = errorMsg.substr(start);
            // Trim whitespace
            while (!searchPattern.empty() && (searchPattern[0] == ' ' || searchPattern[0] == '\t')) {
                searchPattern = searchPattern.substr(1);
            }
            // Extract just the part name (before any space or colon)
            size_t end = searchPattern.find_first_of(" \t:");
            if (end != std::string::npos) {
                searchPattern = searchPattern.substr(0, end);
            }
        } else if (errorMsg.find("Bad wire:") != std::string::npos) {
            size_t start = errorMsg.find("Bad wire:") + 9;
            searchPattern = errorMsg.substr(start);
            while (!searchPattern.empty() && (searchPattern[0] == ' ' || searchPattern[0] == '\t')) {
                searchPattern = searchPattern.substr(1);
            }
        } else if (errorMsg.find("Unknown gate") != std::string::npos) {
            size_t start = errorMsg.find("Unknown gate");
            if (errorMsg.find("kind:") != std::string::npos) {
                start = errorMsg.find("kind:") + 5;
                searchPattern = errorMsg.substr(start);
                while (!searchPattern.empty() && (searchPattern[0] == ' ' || searchPattern[0] == '\t')) {
                    searchPattern = searchPattern.substr(1);
                }
            }
        }
        
        // Search for the pattern in the source
        if (!searchPattern.empty()) {
            iss.clear();
            iss.seekg(0);
            lineNum = 1;
            while (std::getline(iss, line)) {
                std::string lineWithoutComments = line;
                size_t commentPos = lineWithoutComments.find("//");
                if (commentPos != std::string::npos) {
                    lineWithoutComments = lineWithoutComments.substr(0, commentPos);
                }
                if (lineWithoutComments.find(searchPattern) != std::string::npos) {
                    foundLine = true;
                    break;
                }
                lineNum++;
            }
        }
        
        if (foundLine) {
            fullError = "Error at Line " + std::to_string(lineNum) + ": " + errorMsg;
        } else {
            // Fallback: try to find section keywords
            if (errorMsg.find("part") != std::string::npos || errorMsg.find("gate") != std::string::npos) {
                int partsLine = findLineNumber("Parts:");
                if (partsLine > 0) {
                    fullError = "Error at Line " + std::to_string(partsLine) + ": " + errorMsg;
                }
            } else if (errorMsg.find("wire") != std::string::npos) {
                int wiresLine = findLineNumber("Wires:");
                if (wiresLine > 0) {
                    fullError = "Error at Line " + std::to_string(wiresLine) + ": " + errorMsg;
                }
            }
        }
        
        tabs_.setError(fullError);
    } catch (const std::exception& e) {
        std::string errorMsg = "Error: " + std::string(e.what());
        tabs_.setError(errorMsg);
    }
    
    tabs_.render();
}

void LevelEditor::handleTabNavigation(KeyEvent key) {
    if (key.key == Key::Tab) {
        int current = static_cast<int>(tabs_.getActiveTab());
        tabs_.setActiveTab(static_cast<TabbedInterface::Tab>((current + 1) % 4));
        tabs_.render();
    } else if (key.key == Key::ShiftTab) {
        int current = static_cast<int>(tabs_.getActiveTab());
        tabs_.setActiveTab(static_cast<TabbedInterface::Tab>((current + 3) % 4));
        tabs_.render();
    }
}


bool LevelEditor::run() {
    // Terminal is already initialized in interactiveMode, don't re-init
    // TerminalUI::init();
    
    tabs_.render();
    
    while (true) {
        KeyEvent key = TerminalUI::readKey();
        
        // Handle reset confirmation first (before Escape check)
        if (tabs_.isResetConfirmationVisible()) {
            if (key.key == Key::Char) {
                if (key.ch == 'y' || key.ch == 'Y') {
                    // Reset to template
                    solutionText_ = generateTemplate();
                    tabs_.setSolutionText(solutionText_);
                    tabs_.setResetConfirmation(false);
                    tabs_.clearError();
                    tabs_.setSuccess("Solution reset to template");
                    game_.saveSolution(level_.id, solutionText_);
                    tabs_.render();
                } else if (key.ch == 'n' || key.ch == 'N') {
                    // Cancel reset
                    tabs_.setResetConfirmation(false);
                    tabs_.render();
                }
            } else if (key.key == Key::Escape) {
                // Cancel reset on Escape (don't exit IDE)
                tabs_.setResetConfirmation(false);
                tabs_.render();
            }
            continue;
        }
        
        if (key.key == Key::Escape) {
            // Auto-save solution before exiting
            game_.saveSolution(level_.id, solutionText_);
            // Clear screen and prepare for menu
            TerminalUI::clearScreen();
            TerminalUI::showCursor();
            return false;
        }
        
        if (key.key == Key::Tab || key.key == Key::ShiftTab) {
            // Auto-save solution when switching tabs
            game_.saveSolution(level_.id, solutionText_);
            handleTabNavigation(key);
            continue;
        }
        
        TabbedInterface::Tab activeTab = tabs_.getActiveTab();
        
        if (key.key == Key::F6) {
            tabs_.toggleHelp();
            tabs_.render();
            continue;
        }
        
        if (key.key == Key::F12) {
            tabs_.setResetConfirmation(true);
            tabs_.render();
            continue;
        }
        
        if (key.key == Key::ShiftEnter || key.key == Key::F5) {
            if (activeTab == TabbedInterface::Solution) {
                compileAndTest();
            }
            continue;
        }
        
        if (activeTab == TabbedInterface::Solution) {
            bool handled = tabs_.handleKey(key, solutionText_);
            if (handled) {
                tabs_.setSolutionText(solutionText_);
                tabs_.render();
            } else if (key.key == Key::Escape) {
                // Auto-save solution before going back to menu
                game_.saveSolution(level_.id, solutionText_);
                // Clear screen and prepare for menu
                TerminalUI::clearScreen();
                TerminalUI::showCursor();
                break;
            }
        } else {
            // In other tabs, only allow tab navigation and escape
            if (key.key == Key::Escape) {
                // Auto-save solution before going back to menu
                game_.saveSolution(level_.id, solutionText_);
                // Clear screen and prepare for menu
                TerminalUI::clearScreen();
                TerminalUI::showCursor();
                break;
            }
        }
    }
    
    // Clear screen when exiting
    TerminalUI::clearScreen();
    TerminalUI::showCursor();
    return false;
}

