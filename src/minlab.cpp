#include "simulator.h"
#include "game.h"
#include "terminal_ui.h"
#include "level_editor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <limits>

namespace fs = std::filesystem;

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) return "";
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

static void printTruthTable(const AST& ast, Net& net) {
    auto combos = allCombos(ast.inputs);
    for (auto& in : combos) {
        auto out = simulate(net, in);
        std::cout << "in {";
        bool first = true;
        for (auto& k : ast.inputs) {
            if (!first) std::cout << ",";
            first = false;
            std::cout << k << ":" << in[k];
        }
        std::cout << "} -> out {";
        first = true;
        for (auto& k : ast.outputs) {
            if (!first) std::cout << ",";
            first = false;
            std::cout << k << ":" << out[k];
        }
        std::cout << "}\n";
    }
}

static void playLevel(Game& game, const Level& level) {
    LevelEditor editor(game, level);
    editor.run();
}

static void interactiveMode() {
    Game game;
    
    // Determine paths - try multiple locations
    std::string levelsDir = "levels";
    std::string progressFile = ".minlab_progress.json";
    
    // Try to find levels directory relative to executable
    try {
        if (fs::exists("/proc/self/exe")) {
            fs::path exePath = fs::canonical("/proc/self/exe");
            fs::path projectRoot = exePath.parent_path().parent_path();
            std::string candidateLevels = (projectRoot / "levels").string();
            if (fs::exists(candidateLevels)) {
                levelsDir = candidateLevels;
                progressFile = (projectRoot / ".minlab_progress.json").string();
            }
        }
    } catch (...) {
        // Fall back to relative paths
    }
    
    // Final fallback to relative paths
    if (!fs::exists(levelsDir)) {
        levelsDir = "levels";
    }
    
    if (!game.loadLevels(levelsDir)) {
        std::cerr << "Error: Could not load levels from " << levelsDir << "\n";
        std::cerr << "Make sure the 'levels' directory exists with level JSON files.\n";
        return;
    }
    
    game.loadProgress(progressFile);
    
    TerminalUI::init();
    
    while (true) {
        // Clear screen before showing menu (in case we're returning from level editor)
        TerminalUI::clearScreen();
        
        Menu menu("╔══════════════════════════════════════════════════════════╗\n"
                  "║              minlab - Level Selector                     ║\n"
                  "╚══════════════════════════════════════════════════════════╝");
        
        const auto& levels = game.getLevels();
        for (size_t i = 0; i < levels.size(); ++i) {
            const auto& level = levels[i];
            std::string text = level.name + " (Difficulty: " + std::to_string(level.difficulty) + ")";
            if (game.isCompleted(level.id)) {
                text += " [COMPLETED]";
            }
            menu.addOption(text, level.id);
        }
        
        menu.setHighlight(37, -1); // White
        menu.setSelectedHighlight(30, 47); // Black on white
        
        int choice = menu.show();
        
        if (choice < 0) {
            // Exit (Escape or 0)
            game.saveProgress(progressFile);
            TerminalUI::cleanup();
            break;
        }
        
        if (choice >= 0 && choice < static_cast<int>(levels.size())) {
            playLevel(game, levels[choice]);
            game.saveProgress(progressFile);
            // Ensure terminal is still in raw mode after returning from level editor
            TerminalUI::init();
        }
    }
}

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    
    // If no arguments, run interactive mode
    if (argc == 1) {
        interactiveMode();
        return 0;
    }
    
    // If argument is provided, use legacy mode (backward compatibility)
    if (argc >= 2) {
        std::ifstream f(argv[1]);
        if (!f) {
            std::cerr << "Cannot open " << argv[1] << "\n";
        return 1;
    }
        std::string s((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        try {
        AST ast = parseHDL(s);
        Net net = buildNet(ast);
            printTruthTable(ast, net);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 2;
        }
        return 0;
    }
    
    return 0;
}
