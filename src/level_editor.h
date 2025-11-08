#ifndef LEVEL_EDITOR_H
#define LEVEL_EDITOR_H

#include "terminal_ui.h"
#include "game.h"
#include "syntax_checker.h"
#include <string>
#include <vector>

class LevelEditor {
public:
    LevelEditor(Game& game, const Level& level);
    bool run(); // Returns true if level completed, false if cancelled
    
private:
    Game& game_;
    const Level& level_;
    TabbedInterface tabs_;
    std::string solutionText_;
    std::vector<std::string> history_; // Code history
    int historyIndex_;
    
    void updateInstructions();
    void updateStats();
    void updateHistory();
    void compileAndTest();
    void addToHistory(const std::string& code);
    std::string getLastWorkedCode() const;
    void handleTabNavigation(KeyEvent key);
};

#endif

