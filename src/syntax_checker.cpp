#include "syntax_checker.h"
#include "simulator.h"
#include <sstream>
#include <stdexcept>

SyntaxError checkSyntax(const std::string& hdlContent) {
    SyntaxError error;
    error.hasError = false;
    error.line = 0;
    error.column = 0;
    
    if (hdlContent.empty()) {
        error.hasError = true;
        error.message = "Empty HDL content";
        return error;
    }
    
    try {
        parseHDL(hdlContent);
    } catch (const std::runtime_error& e) {
        error.hasError = true;
        error.message = e.what();
        
        // Try to find line number
        std::istringstream iss(hdlContent);
        std::string line;
        int lineNum = 1;
        while (std::getline(iss, line)) {
            if (line.find("Parts:") != std::string::npos && error.message.find("Bad part") != std::string::npos) {
                error.line = lineNum;
                break;
            }
            if (line.find("Wires:") != std::string::npos && error.message.find("Bad wire") != std::string::npos) {
                error.line = lineNum;
                break;
            }
            lineNum++;
        }
        if (error.line == 0) error.line = 1;
    } catch (...) {
        error.hasError = true;
        error.message = "Unknown parsing error";
        error.line = 1;
    }
    
    return error;
}

