#ifndef GAME_H
#define GAME_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "component_library.h"

struct Level {
    std::string id;
    std::string name;
    std::string description;
    int difficulty;
    std::vector<std::string> available_gates;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::vector<std::unordered_map<std::string, std::unordered_map<std::string, int>>> expected;
};

class Game {
public:
    Game();
    bool loadLevels(const std::string& levelsDir);
    std::vector<Level> getLevels() const { return levels_; }
    Level* getLevel(const std::string& id);
    bool validateSolution(const Level& level, const std::string& hdlContent);
    void markCompleted(const std::string& levelId);
    bool isCompleted(const std::string& levelId) const;
    void loadProgress(const std::string& progressFile);
    void saveProgress(const std::string& progressFile) const;
    void saveSolution(const std::string& levelId, const std::string& solution);
    std::string loadSolution(const std::string& levelId) const;
    ComponentLibrary& getComponentLibrary() { return componentLibrary_; }
    const ComponentLibrary& getComponentLibrary() const { return componentLibrary_; }
    
private:
    std::vector<Level> levels_;
    std::unordered_set<std::string> completed_;
    std::unordered_map<std::string, std::string> savedSolutions_; // levelId -> solution
    ComponentLibrary componentLibrary_;
    bool parseLevelJson(const std::string& jsonContent, Level& level);
    std::string readFile(const std::string& path);
};

#endif

