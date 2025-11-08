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
    
    // Validate solution
    if (game_.validateSolution(level_, solutionText_)) {
        tabs_.setSuccess("✓ SUCCESS! Your solution is correct!");
        game_.markCompleted(level_.id);
        addToHistory(solutionText_);
        updateStats();
        tabs_.render();
        return;
    }
    
    // Try to get more detailed error
    try {
        AST ast = parseHDL(solutionText_);
        Net net = buildNet(ast);
        
        // Check inputs
        std::set<std::string> userInputs(ast.inputs.begin(), ast.inputs.end());
        std::set<std::string> expectedInputs(level_.inputs.begin(), level_.inputs.end());
        if (userInputs != expectedInputs) {
            tabs_.setError("Input mismatch: Expected different inputs");
            tabs_.render();
            return;
        }
        
        // Check outputs
        std::set<std::string> userOutputs(ast.outputs.begin(), ast.outputs.end());
        std::set<std::string> expectedOutputs(level_.outputs.begin(), level_.outputs.end());
        if (userOutputs != expectedOutputs) {
            tabs_.setError("Output mismatch: Expected different outputs");
            tabs_.render();
            return;
        }
        
        // Check gates
        std::set<std::string> availableGates(level_.available_gates.begin(), level_.available_gates.end());
        for (const auto& part : ast.parts) {
            std::string kindLower = part.kind;
            std::transform(kindLower.begin(), kindLower.end(), kindLower.begin(), ::tolower);
            if (availableGates.find(kindLower) == availableGates.end()) {
                tabs_.setError("Invalid gate used: " + part.kind + " (not available in this level)");
                tabs_.render();
                return;
            }
        }
        
        // Build comparison table
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
        
        tabs_.setError(tableMsg.str());
    } catch (const std::exception& e) {
        tabs_.setError("Error: " + std::string(e.what()));
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

