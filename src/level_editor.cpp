#include "level_editor.h"
#include "simulator.h"
#include <sstream>
#include <iomanip>
#include <set>
#include <algorithm>
#include <filesystem>

LevelEditor::LevelEditor(Game& game, const Level& level) 
    : game_(game), level_(level), historyIndex_(-1) {
    // Load saved solution if available
    solutionText_ = game_.loadSolution(level_.id);
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

void LevelEditor::compileAndTest() {
    tabs_.clearError();
    tabs_.clearSuccess();
    
    // Check syntax first
    SyntaxError syntaxError = checkSyntax(solutionText_);
    if (syntaxError.hasError) {
        tabs_.setError("Syntax Error (Line " + std::to_string(syntaxError.line) + "): " + syntaxError.message);
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
        
        tabs_.setError("Solution does not match expected truth table");
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
    TerminalUI::init();
    
    tabs_.render();
    
    while (true) {
        KeyEvent key = TerminalUI::readKey();
        
        if (key.key == Key::Escape) {
            // Auto-save solution before exiting
            game_.saveSolution(level_.id, solutionText_);
            TerminalUI::cleanup();
            return false;
        }
        
        if (key.key == Key::Tab || key.key == Key::ShiftTab) {
            // Auto-save solution when switching tabs
            game_.saveSolution(level_.id, solutionText_);
            handleTabNavigation(key);
            continue;
        }
        
        TabbedInterface::Tab activeTab = tabs_.getActiveTab();
        
        if (key.key == Key::ShiftEnter) {
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
                break;
            }
        } else {
            // In other tabs, only allow tab navigation and escape
            if (key.key == Key::Escape) {
                // Auto-save solution before going back to menu
                game_.saveSolution(level_.id, solutionText_);
                break;
            }
        }
    }
    
    TerminalUI::cleanup();
    return false;
}

