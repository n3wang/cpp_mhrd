#include "syntax_checker.h"
#include "simulator.h"
#include <sstream>
#include <stdexcept>

SyntaxError checkSyntax(const std::string& hdlContent) {
    SyntaxError error;
    error.hasError = false;
    error.line = 0;
    error.column = 0;
    error.lineContent = "";
    
    if (hdlContent.empty()) {
        error.hasError = true;
        error.message = "Empty HDL content";
        error.line = 1;
        return error;
    }
    
    // Store lines for better error reporting
    std::vector<std::string> lines;
    std::istringstream iss(hdlContent);
    std::string line;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }
    
    try {
        parseHDL(hdlContent);
    } catch (const std::runtime_error& e) {
        error.hasError = true;
        error.message = e.what();
        
        // Try to find line number by searching for the problematic content
        std::string errorMsg = e.what();
        
        // Extract the problematic part/wire from error message
        std::string problemPart = "";
        if (errorMsg.find("Bad part:") != std::string::npos) {
            size_t start = errorMsg.find("Bad part:") + 9;
            problemPart = errorMsg.substr(start);
            // Trim whitespace
            while (!problemPart.empty() && (problemPart[0] == ' ' || problemPart[0] == '\t')) {
                problemPart = problemPart.substr(1);
            }
        } else if (errorMsg.find("Bad wire:") != std::string::npos) {
            size_t start = errorMsg.find("Bad wire:") + 9;
            problemPart = errorMsg.substr(start);
            while (!problemPart.empty() && (problemPart[0] == ' ' || problemPart[0] == '\t')) {
                problemPart = problemPart.substr(1);
            }
        } else if (errorMsg.find("Unknown gate kind:") != std::string::npos) {
            size_t start = errorMsg.find("Unknown gate kind:") + 18;
            problemPart = errorMsg.substr(start);
            while (!problemPart.empty() && (problemPart[0] == ' ' || problemPart[0] == '\t')) {
                problemPart = problemPart.substr(1);
            }
        }
        
        // Search for the problematic content in lines
        for (size_t i = 0; i < lines.size(); ++i) {
            std::string currentLine = lines[i];
            // Remove comments for matching
            size_t commentPos = currentLine.find("//");
            if (commentPos != std::string::npos) {
                currentLine = currentLine.substr(0, commentPos);
            }
            
            if (!problemPart.empty() && currentLine.find(problemPart) != std::string::npos) {
                error.line = static_cast<int>(i + 1);
                error.lineContent = lines[i];
                break;
            }
            
            // Fallback: check for section keywords
            if (errorMsg.find("Bad part") != std::string::npos && currentLine.find("Parts:") != std::string::npos) {
                error.line = static_cast<int>(i + 1);
                error.lineContent = lines[i];
            } else if (errorMsg.find("Bad wire") != std::string::npos && currentLine.find("Wires:") != std::string::npos) {
                error.line = static_cast<int>(i + 1);
                error.lineContent = lines[i];
            }
        }
        
        if (error.line == 0) {
            error.line = 1;
            if (!lines.empty()) error.lineContent = lines[0];
        }
    } catch (...) {
        error.hasError = true;
        error.message = "Unknown parsing error";
        error.line = 1;
        if (!lines.empty()) error.lineContent = lines[0];
    }
    
    return error;
}

