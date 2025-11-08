#ifndef SYNTAX_CHECKER_H
#define SYNTAX_CHECKER_H

#include <string>

struct SyntaxError {
    std::string message;
    int line;
    int column;
    std::string lineContent; // The actual line content with the error
    bool hasError;
};

SyntaxError checkSyntax(const std::string& hdlContent);

#endif

