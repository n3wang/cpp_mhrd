#include "game.h"
#include "simulator.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <regex>
#include <algorithm>
#include <cctype>
#include <set>

namespace fs = std::filesystem;

std::string Game::readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) return "";
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

static inline std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static std::string extractJsonString(const std::string& json, const std::string& key) {
    std::regex re("\"" + key + "\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch m;
    if (std::regex_search(json, m, re)) return m[1];
    return "";
}

static int extractJsonInt(const std::string& json, const std::string& key) {
    std::regex re("\"" + key + "\"\\s*:\\s*(\\d+)");
    std::smatch m;
    if (std::regex_search(json, m, re)) {
        return std::stoi(m[1]);
    }
    return 0;
}

static std::vector<std::string> extractJsonArray(const std::string& json, const std::string& key) {
    std::vector<std::string> result;
    std::regex re("\"" + key + "\"\\s*:\\s*\\[([^\\]]+)\\]");
    std::smatch m;
    if (std::regex_search(json, m, re)) {
        std::string arr = m[1];
        std::regex itemRe("\"([^\"]+)\"");
        std::sregex_iterator iter(arr.begin(), arr.end(), itemRe);
        std::sregex_iterator end;
        for (; iter != end; ++iter) {
            result.push_back((*iter)[1]);
        }
    }
    return result;
}

static std::vector<std::unordered_map<std::string, std::unordered_map<std::string, int>>> 
extractExpected(const std::string& json) {
    std::vector<std::unordered_map<std::string, std::unordered_map<std::string, int>>> result;
    
    // Find the "expected" array - match from "expected": [ to matching ]
    size_t expectedPos = json.find("\"expected\"");
    if (expectedPos == std::string::npos) return result;
    
    size_t arrayStart = json.find('[', expectedPos);
    if (arrayStart == std::string::npos) return result;
    
    // Find matching closing bracket
    int depth = 1;
    size_t arrayEnd = arrayStart + 1;
    for (; arrayEnd < json.length() && depth > 0; ++arrayEnd) {
        if (json[arrayEnd] == '[') depth++;
        else if (json[arrayEnd] == ']') depth--;
    }
    if (depth != 0) return result;
    
    std::string expectedStr = json.substr(arrayStart + 1, arrayEnd - arrayStart - 2);
    
    // Extract each test case: {"in": {...}, "out": {...}}
    // Use a simpler approach: find each { "in": { ... }, "out": { ... } }
    size_t pos = 0;
    while (pos < expectedStr.length()) {
        size_t caseStart = expectedStr.find('{', pos);
        if (caseStart == std::string::npos) break;
        
        // Find matching closing brace
        depth = 1;
        size_t caseEnd = caseStart + 1;
        for (; caseEnd < expectedStr.length() && depth > 0; ++caseEnd) {
            if (expectedStr[caseEnd] == '{') depth++;
            else if (expectedStr[caseEnd] == '}') depth--;
        }
        if (depth != 0) break;
        
        std::string caseStr = expectedStr.substr(caseStart, caseEnd - caseStart);
        
        // Extract "in" and "out" objects
        std::regex inRe("\"in\"\\s*:\\s*\\{([^}]+)\\}");
        std::regex outRe("\"out\"\\s*:\\s*\\{([^}]+)\\}");
        std::smatch inMatch, outMatch;
        
        if (std::regex_search(caseStr, inMatch, inRe) && std::regex_search(caseStr, outMatch, outRe)) {
            std::unordered_map<std::string, std::unordered_map<std::string, int>> testCase;
            std::unordered_map<std::string, int> inMap, outMap;
            
            // Parse "in" map
            std::string inStr = inMatch[1];
            std::regex pairRe("\"([^\"]+)\"\\s*:\\s*(\\d+)");
            std::sregex_iterator inIter(inStr.begin(), inStr.end(), pairRe);
            std::sregex_iterator inEnd;
            for (; inIter != inEnd; ++inIter) {
                inMap[(*inIter)[1]] = std::stoi((*inIter)[2]);
            }
            
            // Parse "out" map
            std::string outStr = outMatch[1];
            std::sregex_iterator outIter(outStr.begin(), outStr.end(), pairRe);
            std::sregex_iterator outEnd;
            for (; outIter != outEnd; ++outIter) {
                outMap[(*outIter)[1]] = std::stoi((*outIter)[2]);
            }
            
            testCase["in"] = inMap;
            testCase["out"] = outMap;
            result.push_back(testCase);
        }
        
        pos = caseEnd;
    }
    
    return result;
}

bool Game::parseLevelJson(const std::string& jsonContent, Level& level) {
    level.id = extractJsonString(jsonContent, "id");
    level.name = extractJsonString(jsonContent, "name");
    level.description = extractJsonString(jsonContent, "description");
    level.difficulty = extractJsonInt(jsonContent, "difficulty");
    level.available_gates = extractJsonArray(jsonContent, "available_gates");
    level.inputs = extractJsonArray(jsonContent, "inputs");
    level.outputs = extractJsonArray(jsonContent, "outputs");
    level.expected = extractExpected(jsonContent);
    
    return !level.id.empty() && !level.name.empty();
}

bool Game::loadLevels(const std::string& levelsDir) {
    levels_.clear();
    
    if (!fs::exists(levelsDir) || !fs::is_directory(levelsDir)) {
        return false;
    }
    
    for (const auto& entry : fs::directory_iterator(levelsDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            std::string content = readFile(entry.path().string());
            Level level;
            if (parseLevelJson(content, level)) {
                levels_.push_back(level);
            }
        }
    }
    
    // Sort by difficulty and id
    std::sort(levels_.begin(), levels_.end(), [](const Level& a, const Level& b) {
        if (a.difficulty != b.difficulty) return a.difficulty < b.difficulty;
        return a.id < b.id;
    });
    
    return !levels_.empty();
}

Level* Game::getLevel(const std::string& id) {
    for (auto& level : levels_) {
        if (level.id == id) return &level;
    }
    return nullptr;
}

bool Game::validateSolution(const Level& level, const std::string& hdlContent) {
    try {
        AST ast = parseHDL(hdlContent);
        Net net = buildNet(ast);
        
        // Check inputs match
        std::set<std::string> userInputs(ast.inputs.begin(), ast.inputs.end());
        std::set<std::string> expectedInputs(level.inputs.begin(), level.inputs.end());
        if (userInputs != expectedInputs) return false;
        
        // Check outputs match
        std::set<std::string> userOutputs(ast.outputs.begin(), ast.outputs.end());
        std::set<std::string> expectedOutputs(level.outputs.begin(), level.outputs.end());
        if (userOutputs != expectedOutputs) return false;
        
        // Check available gates
        std::set<std::string> availableGates(level.available_gates.begin(), level.available_gates.end());
        for (const auto& part : ast.parts) {
            std::string kindLower = part.kind;
            std::transform(kindLower.begin(), kindLower.end(), kindLower.begin(), ::tolower);
            if (availableGates.find(kindLower) == availableGates.end()) {
                return false; // Used a gate that's not available
            }
        }
        
        // Validate against expected truth table
        for (const auto& testCase : level.expected) {
            const auto& inVec = testCase.at("in");
            const auto& expectedOut = testCase.at("out");
            
            auto actualOut = simulate(net, inVec);
            
            for (const auto& [key, expectedVal] : expectedOut) {
                if (actualOut.find(key) == actualOut.end() || actualOut.at(key) != expectedVal) {
                    return false;
                }
            }
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

void Game::markCompleted(const std::string& levelId) {
    completed_.insert(levelId);
}

bool Game::isCompleted(const std::string& levelId) const {
    return completed_.count(levelId) > 0;
}

void Game::loadProgress(const std::string& progressFile) {
    completed_.clear();
    if (!fs::exists(progressFile)) return;
    
    std::string content = readFile(progressFile);
    std::regex idRe("\"([^\"]+)\"");
    std::sregex_iterator iter(content.begin(), content.end(), idRe);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        completed_.insert((*iter)[1]);
    }
}

void Game::saveProgress(const std::string& progressFile) const {
    std::ofstream f(progressFile);
    if (!f) return;
    
    f << "{\n  \"completed\": [\n";
    bool first = true;
    for (const auto& id : completed_) {
        if (!first) f << ",\n";
        first = false;
        f << "    \"" << id << "\"";
    }
    f << "\n  ]\n}\n";
}

