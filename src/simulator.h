#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

struct GateDef {
    std::vector<std::string> inPins, outPins;
    std::function<std::unordered_map<std::string, int>(const std::unordered_map<std::string, int>&)> eval;
};

struct AST {
    std::vector<std::string> inputs, outputs;
    struct Part { std::string name, kind; };
    std::vector<Part> parts;
    struct Wire { std::string src, dst; };
    std::vector<Wire> wires;
};

struct Net {
    std::unordered_map<std::string, int> val;
    std::unordered_map<std::string, std::vector<std::string>> fan;
    std::unordered_map<std::string, GateDef> partDef;
    AST ast;
};

AST parseHDL(const std::string& src);
Net buildNet(const AST& ast);
Net buildNetWithComponents(const AST& ast, class ComponentLibrary* componentLib);
std::unordered_map<std::string, int> simulate(Net& net, const std::unordered_map<std::string, int>& inVec);
std::vector<std::unordered_map<std::string, int>> allCombos(const std::vector<std::string>& names);

#endif

