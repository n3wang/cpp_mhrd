#ifndef TERMINAL_UI_H
#define TERMINAL_UI_H

#include <string>
#include <vector>
#include <functional>

enum class Key {
    None,
    Tab,
    ShiftTab,
    ShiftEnter,
    Enter,
    Escape,
    Up,
    Down,
    Left,
    Right,
    Backspace,
    Delete,
    Home,
    End,
    F4,
    F5,
    F6,
    F12,
    AltUp,
    AltDown,
    AltBackspace,
    ShiftDelete,
    CtrlLeft,
    CtrlRight,
    CtrlBackspace,
    CtrlDelete,
    Char
};

struct KeyEvent {
    Key key;
    char ch;
};

class TerminalUI {
public:
    static void init();
    static void cleanup();
    static void clearScreen();
    static void moveCursor(int row, int col);
    static void hideCursor();
    static void showCursor();
    static void saveCursor();
    static void restoreCursor();
    static KeyEvent readKey();
    static int getWidth();
    static int getHeight();
    static void setColor(int fg, int bg = -1);
    static void resetColor();
    static void clearLine();
    static void clearToEndOfLine();
};

class Menu {
public:
    struct Option {
        std::string text;
        std::string id;
        bool enabled;
    };
    
    Menu(const std::string& title);
    void addOption(const std::string& text, const std::string& id, bool enabled = true);
    void setHighlight(int fg, int bg = -1);
    void setSelectedHighlight(int fg, int bg = -1);
    int show(); // Returns selected index, -1 for cancel
    void setSelected(int index);
    int getSelected() const { return selected_; }
    size_t getOptionCount() const { return options_.size(); }
    const Option& getOption(int index) const { return options_[index]; }
    
private:
    std::string title_;
    std::vector<Option> options_;
    int selected_;
    int highlightFg_, highlightBg_;
    int selectedFg_, selectedBg_;
    void render();
};

class Table {
public:
    Table();
    void addHeader(const std::vector<std::string>& headers);
    void addRow(const std::vector<std::string>& cells);
    void setColumnAlignment(int col, int align); // -1 left, 0 center, 1 right
    void setMaxWidth(int width);
    void setMaxHeight(int height);
    std::string render() const;
    
private:
    std::vector<std::string> headers_;
    std::vector<std::vector<std::string>> rows_;
    std::vector<int> columnAlignments_; // -1 left, 0 center, 1 right
    int maxWidth_;
    int maxHeight_;
    void calculateColumnWidths(std::vector<int>& widths) const;
    std::string formatCell(const std::string& content, int width, int align) const;
};

class TabbedInterface {
public:
    enum Tab { Solution = 0, Instructions = 1, Stats = 2, History = 3 };
    
    TabbedInterface();
    void setSolutionText(const std::string& text);
    void setInstructionsText(const std::string& text);
    void setStatsText(const std::string& text);
    void setHistoryText(const std::string& text);
    void setActiveTab(Tab tab);
    Tab getActiveTab() const { return activeTab_; }
    void setCursorPosition(int row, int col);
    void getCursorPosition(int& row, int& col) const;
    void render();
    bool handleKey(KeyEvent key, std::string& solutionText);
    void setError(const std::string& error);
    void clearError();
    void setSuccess(const std::string& message);
    void clearSuccess();
    bool isHelpVisible() const { return showHelp_; }
    void toggleHelp() { showHelp_ = !showHelp_; }
    bool isResetConfirmationVisible() const { return showResetConfirmation_; }
    void setResetConfirmation(bool show) { showResetConfirmation_ = show; }
    
private:
    Tab activeTab_;
    std::string solutionText_;
    std::string instructionsText_;
    std::string statsText_;
    std::string historyText_;
    std::string error_;
    std::string success_;
    int cursorRow_, cursorCol_;
    int scrollOffset_[4]; // One per tab
    bool showHelp_;
    bool showResetConfirmation_;
    void renderTabs();
    void renderContent();
    void renderSolution();
    void renderInstructions();
    void renderStats();
    void renderHistory();
    void renderHelp();
    void renderResetConfirmation();
    void updateCursor();
};

#endif

