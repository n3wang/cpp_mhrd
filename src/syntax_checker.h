#ifndef SYNTAX_CHECKER_H
#define SYNTAX_CHECKER_H

#include <string>

struct SyntaxError {
    std::string message;
    int line;
    int column;
    bool hasError;
};

SyntaxError checkSyntax(const std::string& hdlContent);

#endif

