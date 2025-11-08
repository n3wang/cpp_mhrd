#include "terminal_ui.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <cerrno>

static struct termios originalTermios;
static bool initialized = false;

void TerminalUI::init() {
    if (initialized) return;
    
    // Check if stdin is a terminal
    if (!isatty(STDIN_FILENO)) {
        return; // Not a terminal, skip initialization
    }
    
    tcgetattr(STDIN_FILENO, &originalTermios);
    struct termios newTermios = originalTermios;
    newTermios.c_lflag &= ~(ICANON | ECHO);
    newTermios.c_cc[VMIN] = 1;
    newTermios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
    
    std::cout << "\033[?1049h"; // Enable alternate screen buffer
    std::cout.flush();
    initialized = true;
}

void TerminalUI::cleanup() {
    if (!initialized) return;
    
    std::cout << "\033[?1049l"; // Disable alternate screen buffer
    std::cout.flush();
    if (isatty(STDIN_FILENO)) {
        tcsetattr(STDIN_FILENO, TCSANOW, &originalTermios);
    }
    showCursor();
    resetColor();
    initialized = false;
}

void TerminalUI::clearScreen() {
    std::cout << "\033[2J\033[H";
    std::cout.flush();
}

void TerminalUI::moveCursor(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H";
}

void TerminalUI::hideCursor() {
    std::cout << "\033[?25l";
}

void TerminalUI::showCursor() {
    std::cout << "\033[?25h";
}

void TerminalUI::saveCursor() {
    std::cout << "\033[s";
}

void TerminalUI::restoreCursor() {
    std::cout << "\033[u";
}

KeyEvent TerminalUI::readKey() {
    // Make sure stdin is non-blocking for error checking
    char c;
    ssize_t result = read(STDIN_FILENO, &c, 1);
    if (result != 1) {
        if (result < 0 && errno == EAGAIN) {
            return {Key::None, 0};
        }
        return {Key::None, 0};
    }
    
    // Check for escape sequences
    if (c == '\033') {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return {Key::Escape, 0};
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return {Key::Escape, 0};
        
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                char extra;
                if (read(STDIN_FILENO, &extra, 1) == 1 && extra == '~') {
                    if (seq[1] == '3') return {Key::Delete, 0};
                    if (seq[1] == '1') return {Key::Home, 0};
                    if (seq[1] == '4') return {Key::End, 0};
                }
                // Check for Shift+Enter: [13;2~ or similar
                if (seq[1] == '1' && extra == '3') {
                    char more;
                    if (read(STDIN_FILENO, &more, 1) == 1 && more == ';') {
                        if (read(STDIN_FILENO, &more, 1) == 1 && more == '2') {
                            if (read(STDIN_FILENO, &more, 1) == 1 && more == '~') {
                                return {Key::ShiftEnter, 0};
                            }
                        }
                    }
                }
                return {Key::None, 0};
            }
            switch (seq[1]) {
                case 'A': return {Key::Up, 0};
                case 'B': return {Key::Down, 0};
                case 'C': return {Key::Right, 0};
                case 'D': return {Key::Left, 0};
                case 'Z': return {Key::ShiftTab, 0}; // Shift+Tab
            }
        }
        return {Key::Escape, 0};
    }
    
    // Check for special keys
    if (c == '\t') {
        return {Key::Tab, 0};
    }
    
    if (c == '\n' || c == '\r') {
        // Check if Shift is held (simplified - terminal may send different sequence)
        // For now, we'll use a timeout-based approach or check terminal capabilities
        // Most terminals send \n for Enter, but Shift+Enter might be detected differently
        // We'll handle this in the application layer by checking modifier state
        return {Key::Enter, 0};
    }
    
    if (c == 127 || c == '\b') return {Key::Backspace, 0};
    if (c >= 32 && c <= 126) return {Key::Char, c};
    
    return {Key::None, 0};
}

int TerminalUI::getWidth() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col > 0 ? w.ws_col : 80;
}

int TerminalUI::getHeight() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row > 0 ? w.ws_row : 24;
}

void TerminalUI::setColor(int fg, int bg) {
    if (fg >= 0) std::cout << "\033[" << (30 + fg) << "m";
    if (bg >= 0) std::cout << "\033[" << (40 + bg) << "m";
}

void TerminalUI::resetColor() {
    std::cout << "\033[0m";
}

void TerminalUI::clearLine() {
    std::cout << "\033[2K";
}

void TerminalUI::clearToEndOfLine() {
    std::cout << "\033[0K";
}

// Menu implementation
Menu::Menu(const std::string& title) : title_(title), selected_(0), 
    highlightFg_(37), highlightBg_(-1), selectedFg_(30), selectedBg_(47) {
}

void Menu::addOption(const std::string& text, const std::string& id, bool enabled) {
    options_.push_back({text, id, enabled});
}

void Menu::setHighlight(int fg, int bg) {
    highlightFg_ = fg;
    highlightBg_ = bg;
}

void Menu::setSelectedHighlight(int fg, int bg) {
    selectedFg_ = fg;
    selectedBg_ = bg;
}

void Menu::setSelected(int index) {
    if (index >= 0 && index < static_cast<int>(options_.size())) {
        selected_ = index;
    }
}

void Menu::render() {
    TerminalUI::clearScreen();
    std::cout << title_ << "\n\n";
    
    for (size_t i = 0; i < options_.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == selected_);
        bool isHighlighted = isSelected;
        
        if (isHighlighted) {
            TerminalUI::setColor(selectedFg_, selectedBg_);
        } else {
            TerminalUI::setColor(highlightFg_, highlightBg_);
        }
        
        std::cout << "  [" << (i + 1) << "] ";
        if (isSelected) std::cout << "▶ ";
        else std::cout << "  ";
        
        std::cout << options_[i].text;
        
        if (!options_[i].enabled) {
            TerminalUI::resetColor();
            std::cout << " (disabled)";
        }
        
        TerminalUI::resetColor();
        std::cout << "\n";
    }
    
    std::cout << "\n  [0] Exit\n";
    std::cout << "\nUse arrow keys or numbers to select, Enter to confirm\n";
    std::cout.flush(); // Ensure output is displayed
}

int Menu::show() {
    render();
    
    while (true) {
        KeyEvent key = TerminalUI::readKey();
        
        if (key.key == Key::Enter) {
            return selected_;
        }
        
        if (key.key == Key::Escape || (key.key == Key::Char && key.ch == '0')) {
            return -1;
        }
        
        if (key.key == Key::Up) {
            selected_ = (selected_ - 1 + options_.size()) % options_.size();
            render();
        } else if (key.key == Key::Down) {
            selected_ = (selected_ + 1) % options_.size();
            render();
        } else if (key.key == Key::Char) {
            if (key.ch >= '1' && key.ch <= '9') {
                int num = key.ch - '1';
                if (num < static_cast<int>(options_.size())) {
                    selected_ = num;
                    render();
                }
            }
        }
    }
}

// TabbedInterface implementation
TabbedInterface::TabbedInterface() : activeTab_(Solution), cursorRow_(0), cursorCol_(0) {
    for (int i = 0; i < 4; ++i) scrollOffset_[i] = 0;
}

void TabbedInterface::setSolutionText(const std::string& text) {
    solutionText_ = text;
}

void TabbedInterface::setInstructionsText(const std::string& text) {
    instructionsText_ = text;
}

void TabbedInterface::setStatsText(const std::string& text) {
    statsText_ = text;
}

void TabbedInterface::setHistoryText(const std::string& text) {
    historyText_ = text;
}

void TabbedInterface::setActiveTab(Tab tab) {
    activeTab_ = tab;
}

void TabbedInterface::setCursorPosition(int row, int col) {
    cursorRow_ = row;
    cursorCol_ = col;
}

void TabbedInterface::getCursorPosition(int& row, int& col) const {
    row = cursorRow_;
    col = cursorCol_;
}

void TabbedInterface::setError(const std::string& error) {
    error_ = error;
    success_.clear();
}

void TabbedInterface::clearError() {
    error_.clear();
}

void TabbedInterface::setSuccess(const std::string& message) {
    success_ = message;
    error_.clear();
}

void TabbedInterface::clearSuccess() {
    success_.clear();
}

void TabbedInterface::render() {
    TerminalUI::clearScreen();
    renderTabs();
    renderContent();
    
    // Show status messages
    int height = TerminalUI::getHeight();
    TerminalUI::moveCursor(height - 2, 1);
    TerminalUI::clearLine();
    
    if (!error_.empty()) {
        TerminalUI::setColor(31, -1); // Red
        std::cout << "Error: " << error_;
        TerminalUI::resetColor();
    } else if (!success_.empty()) {
        TerminalUI::setColor(32, -1); // Green
        std::cout << "Success: " << success_;
        TerminalUI::resetColor();
    }
    
    TerminalUI::moveCursor(height - 1, 1);
    TerminalUI::clearLine();
    std::cout << "Tab/Shift+Tab: Switch tabs | Shift+Enter: Compile | Enter: Newline | Esc: Back to menu";
    std::cout.flush();
    
    updateCursor();
}

void TabbedInterface::renderTabs() {
    const char* tabNames[] = {"Solution", "Instructions", "Stats", "History"};
    
    for (int i = 0; i < 4; ++i) {
        if (i == static_cast<int>(activeTab_)) {
            TerminalUI::setColor(30, 47); // Black on white (inverted)
            std::cout << "▶ [" << tabNames[i] << "]";
        } else {
            TerminalUI::setColor(37, -1); // White
            std::cout << "  [" << tabNames[i] << "]";
        }
        TerminalUI::resetColor();
        if (i < 3) std::cout << " ";
    }
    std::cout << "\n";
    TerminalUI::setColor(37, -1);
    int width = TerminalUI::getWidth();
    for (int i = 0; i < width && i < 80; ++i) std::cout << "─";
    TerminalUI::resetColor();
    std::cout << "\n";
}

void TabbedInterface::renderContent() {
    int height = TerminalUI::getHeight();
    int contentHeight = height - 5; // Account for tabs, status, etc.
    
    switch (activeTab_) {
        case Solution:
            renderSolution();
            break;
        case Instructions:
            renderInstructions();
            break;
        case Stats:
            renderStats();
            break;
        case History:
            renderHistory();
            break;
    }
}

void TabbedInterface::renderSolution() {
    // Simple text editor view
    std::string display = solutionText_;
    if (display.empty()) {
        TerminalUI::setColor(37, -1);
        std::cout << "(Enter your HDL solution here)\n";
        TerminalUI::resetColor();
    } else {
        std::cout << display;
        if (display.empty() || display.back() != '\n') std::cout << "\n";
    }
    std::cout.flush();
}

void TabbedInterface::renderInstructions() {
    std::cout << instructionsText_;
    std::cout.flush();
}

void TabbedInterface::renderStats() {
    std::cout << statsText_;
    std::cout.flush();
}

void TabbedInterface::renderHistory() {
    std::cout << historyText_;
    std::cout.flush();
}

void TabbedInterface::updateCursor() {
    if (activeTab_ == Solution) {
        // Calculate cursor position in solution text
        // Count lines before cursor position
        int lines = 3; // Start after tabs (2 lines) + 1 for 1-based
        int col = 1;
        int pos = 0;
        for (int i = 0; i < cursorCol_ && i < static_cast<int>(solutionText_.length()); ++i) {
            if (solutionText_[i] == '\n') {
                lines++;
                col = 1;
                pos = i + 1;
            } else {
                col++;
            }
        }
        // Calculate column position on current line
        col = cursorCol_ - pos + 1;
        if (col < 1) col = 1;
        TerminalUI::moveCursor(lines, col);
        TerminalUI::showCursor();
        std::cout.flush();
    } else {
        TerminalUI::hideCursor();
    }
}

bool TabbedInterface::handleKey(KeyEvent key, std::string& solutionText) {
    if (key.key == Key::Tab) {
        // Cycle to next tab
        activeTab_ = static_cast<Tab>((static_cast<int>(activeTab_) + 1) % 4);
        render();
        return true;
    }
    
    if (key.key == Key::Escape) {
        return false; // Signal to go back
    }
    
    if (activeTab_ == Solution) {
        if (key.key == Key::Char) {
            // Insert character
            if (cursorCol_ < static_cast<int>(solutionText.length())) {
                solutionText.insert(solutionText.begin() + cursorCol_, key.ch);
            } else {
                solutionText += key.ch;
            }
            cursorCol_++;
            render();
            return true;
        } else if (key.key == Key::Backspace) {
            if (cursorCol_ > 0 && !solutionText.empty()) {
                solutionText.erase(solutionText.begin() + cursorCol_ - 1);
                cursorCol_--;
                render();
            }
            return true;
        } else if (key.key == Key::Delete) {
            if (cursorCol_ < static_cast<int>(solutionText.length())) {
                solutionText.erase(solutionText.begin() + cursorCol_);
                render();
            }
            return true;
        } else if (key.key == Key::Up) {
            // Move cursor up one line
            int lineStart = cursorCol_;
            while (lineStart > 0 && solutionText[lineStart - 1] != '\n') {
                lineStart--;
            }
            if (lineStart > 0) {
                // Move to previous line
                int prevLineStart = lineStart - 1;
                while (prevLineStart > 0 && solutionText[prevLineStart - 1] != '\n') {
                    prevLineStart--;
                }
                int offset = cursorCol_ - lineStart;
                cursorCol_ = prevLineStart + offset;
                if (cursorCol_ > lineStart - 1) cursorCol_ = lineStart - 1;
            }
            updateCursor();
            return true;
        } else if (key.key == Key::Down) {
            // Move cursor down one line
            int lineStart = cursorCol_;
            while (lineStart > 0 && solutionText[lineStart - 1] != '\n') {
                lineStart--;
            }
            int lineEnd = cursorCol_;
            while (lineEnd < static_cast<int>(solutionText.length()) && solutionText[lineEnd] != '\n') {
                lineEnd++;
            }
            if (lineEnd < static_cast<int>(solutionText.length())) {
                // Move to next line
                int nextLineStart = lineEnd + 1;
                int nextLineEnd = nextLineStart;
                while (nextLineEnd < static_cast<int>(solutionText.length()) && solutionText[nextLineEnd] != '\n') {
                    nextLineEnd++;
                }
                int offset = cursorCol_ - lineStart;
                cursorCol_ = nextLineStart + offset;
                if (cursorCol_ > nextLineEnd) cursorCol_ = nextLineEnd;
            }
            updateCursor();
            return true;
        } else if (key.key == Key::Left) {
            if (cursorCol_ > 0) cursorCol_--;
            updateCursor();
            return true;
        } else if (key.key == Key::Right) {
            if (cursorCol_ < static_cast<int>(solutionText.length())) cursorCol_++;
            updateCursor();
            return true;
        } else if (key.key == Key::Enter) {
            // Regular Enter - insert newline
            solutionText.insert(solutionText.begin() + cursorCol_, '\n');
            cursorCol_++;
            render();
            return true;
        } else if (key.key == Key::ShiftEnter) {
            // Shift+Enter - don't handle here, let level editor handle it
            return false; // Signal to parent to handle
        }
    }
    
    return true;
}

