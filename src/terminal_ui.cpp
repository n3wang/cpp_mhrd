#include "terminal_ui.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <cerrno>
#include <iomanip>
#include <sstream>

static struct termios originalTermios;
static bool initialized = false;

void TerminalUI::init() {
    // Check if stdin is a terminal
    if (!isatty(STDIN_FILENO)) {
        return; // Not a terminal, skip initialization
    }
    
    // Save original terminal settings if not already saved
    if (!initialized) {
        tcgetattr(STDIN_FILENO, &originalTermios);
        std::cout << "\033[?1049h"; // Enable alternate screen buffer
        std::cout.flush();
        initialized = true;
    }
    
    // Always ensure raw mode is set (idempotent)
    struct termios newTermios = originalTermios;
    newTermios.c_lflag &= ~(ICANON | ECHO);
    newTermios.c_cc[VMIN] = 1;
    newTermios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
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
        char seq[2];
        ssize_t r1 = read(STDIN_FILENO, &seq[0], 1);
        if (r1 != 1) return {Key::Escape, 0};
        
        if (seq[0] == '[') {
            ssize_t r2 = read(STDIN_FILENO, &seq[1], 1);
            if (r2 != 1) return {Key::Escape, 0};
            
            if (seq[1] >= '0' && seq[1] <= '9') {
                char extra;
                ssize_t r3 = read(STDIN_FILENO, &extra, 1);
                if (r3 != 1) return {Key::None, 0};
                
                // Handle modifier sequences: [1;mA where m is modifier
                // 2 = Shift, 3 = Alt, 4 = Shift+Alt, 5 = Ctrl, 6 = Shift+Ctrl
                if (seq[1] == '1' && extra == ';') {
                    char modifier;
                    ssize_t r4 = read(STDIN_FILENO, &modifier, 1);
                    if (r4 != 1) return {Key::None, 0};
                    
                    char key;
                    ssize_t r5 = read(STDIN_FILENO, &key, 1);
                    if (r5 != 1) return {Key::None, 0};
                    
                    if (modifier == '3') { // Alt
                        if (key == 'A') return {Key::AltUp, 0};
                        if (key == 'B') return {Key::AltDown, 0};
                    } else if (modifier == '5') { // Ctrl
                        if (key == 'C') return {Key::CtrlRight, 0};
                        if (key == 'D') return {Key::CtrlLeft, 0};
                    }
                    return {Key::None, 0};
                }
                
                // Handle F5: [15~ (multi-digit function key)
                if (seq[1] == '1' && extra == '5') {
                    char tilde;
                    if (read(STDIN_FILENO, &tilde, 1) == 1 && tilde == '~') {
                        return {Key::F5, 0};
                    }
                    return {Key::None, 0};
                }
                
                // Handle single-digit function keys and special keys
                if (extra == '~') {
                    if (seq[1] == '1') return {Key::Home, 0};
                    if (seq[1] == '3') return {Key::Delete, 0};
                    if (seq[1] == '4') return {Key::End, 0};
                }
                
                // Check for Shift+Enter: [13;2~ or similar (some terminals)
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
        return {Key::Enter, 0};
    }
    
    // Check for Ctrl+Backspace (typically \b or 127 with Ctrl)
    // Ctrl+Backspace often sends \b (8) or 127
    // We'll detect Ctrl+Backspace by checking if it's 127 or \b
    // Note: This is terminal-dependent, some send different sequences
    if (c == 127 || c == '\b') {
        // Check if there's a modifier - for now, treat as regular backspace
        // Ctrl+Backspace detection would need more complex handling
        return {Key::Backspace, 0};
    }
    
    // Check for Ctrl+Delete (typically sends DEL with Ctrl modifier)
    // This is complex and terminal-dependent
    
    // Check for Ctrl+key combinations (Ctrl+A = 1, Ctrl+B = 2, etc.)
    // Ctrl+Backspace might be sent as \b or 127
    // Ctrl+Delete might be sent as a special sequence
    
    if (c >= 32 && c <= 126) return {Key::Char, c};
    
    // Handle Ctrl+Backspace and Ctrl+Delete via special characters
    // Ctrl+H = Backspace (8), but we already handle that
    // For Ctrl+Delete, we'd need to detect it differently
    
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

// Table implementation
Table::Table() : maxWidth_(80), maxHeight_(24) {
}

void Table::addHeader(const std::vector<std::string>& headers) {
    headers_ = headers;
    columnAlignments_.resize(headers.size(), -1); // Default left align
}

void Table::addRow(const std::vector<std::string>& cells) {
    rows_.push_back(cells);
}

void Table::setColumnAlignment(int col, int align) {
    if (col >= 0 && col < static_cast<int>(columnAlignments_.size())) {
        columnAlignments_[col] = align;
    }
}

void Table::setMaxWidth(int width) {
    maxWidth_ = width;
}

void Table::setMaxHeight(int height) {
    maxHeight_ = height;
}

void Table::calculateColumnWidths(std::vector<int>& widths) const {
    if (headers_.empty()) return;
    
    widths.resize(headers_.size(), 0);
    
    // Find max width for each column (content only, no padding yet)
    for (size_t i = 0; i < headers_.size(); ++i) {
        widths[i] = static_cast<int>(headers_[i].length());
    }
    
    for (const auto& row : rows_) {
        for (size_t i = 0; i < row.size() && i < widths.size(); ++i) {
            widths[i] = std::max(widths[i], static_cast<int>(row[i].length()));
        }
    }
    
    // Ensure minimum width of 3
    for (auto& w : widths) {
        w = std::max(3, w);
    }
}

std::string Table::formatCell(const std::string& content, int width, int align) const {
    std::string result;
    int contentLen = static_cast<int>(content.length());
    int cellWidth = width; // Total cell width including padding
    
    // Truncate if too long
    std::string displayContent = content;
    if (contentLen > cellWidth - 2) {
        displayContent = content.substr(0, cellWidth - 5) + "...";
        contentLen = static_cast<int>(displayContent.length());
    }
    
    if (align == 1) { // Right align
        int padding = cellWidth - contentLen - 2; // -2 for spaces on both sides
        for (int i = 0; i < padding; ++i) result += " ";
        result += " " + displayContent + " ";
    } else if (align == 0) { // Center
        int totalPadding = cellWidth - contentLen - 2;
        int leftPad = totalPadding / 2;
        int rightPad = totalPadding - leftPad;
        result += " ";
        for (int i = 0; i < leftPad; ++i) result += " ";
        result += displayContent;
        for (int i = 0; i < rightPad; ++i) result += " ";
        result += " ";
    } else { // Left align
        result += " " + displayContent;
        int padding = cellWidth - contentLen - 2;
        for (int i = 0; i < padding; ++i) result += " ";
        result += " ";
    }
    
    return result;
}

std::string Table::render() const {
    if (headers_.empty()) return "";
    
    std::ostringstream oss;
    std::vector<int> widths;
    calculateColumnWidths(widths);
    
    // Calculate total width (borders + separators + column widths)
    int totalWidth = 1; // Left border
    for (int w : widths) {
        totalWidth += w + 1; // +1 for separator
    }
    
    // Adjust if too wide
    if (totalWidth > maxWidth_ && !widths.empty()) {
        int excess = totalWidth - maxWidth_;
        int perColumn = excess / static_cast<int>(widths.size());
        for (auto& w : widths) {
            w = std::max(3, w - perColumn);
        }
    }
    
    // Top border
    oss << "┌";
    for (size_t i = 0; i < widths.size(); ++i) {
        for (int j = 0; j < widths[i]; ++j) oss << "─";
        if (i < widths.size() - 1) oss << "┬";
    }
    oss << "┐\n";
    
    // Header row
    oss << "│";
    for (size_t i = 0; i < headers_.size() && i < widths.size(); ++i) {
        int align = (i < columnAlignments_.size()) ? columnAlignments_[i] : -1;
        oss << formatCell(headers_[i], widths[i], align);
        oss << "│";
    }
    oss << "\n";
    
    // Header separator
    oss << "├";
    for (size_t i = 0; i < widths.size(); ++i) {
        for (int j = 0; j < widths[i]; ++j) oss << "─";
        if (i < widths.size() - 1) oss << "┼";
    }
    oss << "┤\n";
    
    // Data rows
    for (const auto& row : rows_) {
        oss << "│";
        for (size_t i = 0; i < row.size() && i < widths.size(); ++i) {
            int align = (i < columnAlignments_.size()) ? columnAlignments_[i] : -1;
            oss << formatCell(row[i], widths[i], align);
            oss << "│";
        }
        // Fill missing columns if row is shorter than headers
        for (size_t i = row.size(); i < widths.size(); ++i) {
            int align = (i < columnAlignments_.size()) ? columnAlignments_[i] : -1;
            oss << formatCell("", widths[i], align);
            oss << "│";
        }
        oss << "\n";
    }
    
    // Bottom border
    oss << "└";
    for (size_t i = 0; i < widths.size(); ++i) {
        for (int j = 0; j < widths[i]; ++j) oss << "─";
        if (i < widths.size() - 1) oss << "┴";
    }
    oss << "┘\n";
    
    return oss.str();
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
    int height = TerminalUI::getHeight();
    
    // Render tabs (2 lines)
    renderTabs();
    
    // Calculate available space
    int errorLines = 0;
    if (!error_.empty()) {
        for (char c : error_) {
            if (c == '\n') errorLines++;
        }
        errorLines++; // Count last line
    }
    
    // Reserve space: tabs (2 lines) + error area (if any) + status bar (1 line)
    int maxErrorDisplayLines = 0;
    if (!error_.empty() || !success_.empty()) {
        maxErrorDisplayLines = std::min(errorLines + 1, height - 5); // Reserve space for errors
    }
    int statusBarRow = height - 1;
    int errorStartRow = statusBarRow - maxErrorDisplayLines;
    int contentEndRow = errorStartRow - 1;
    
    // Render content (will naturally flow from row 3)
    // Content rendering functions will output text, we just need to make sure
    // we don't overwrite it with error messages
    renderContent();
    
    // Clear and render error/success area at the bottom
    for (int i = 0; i < maxErrorDisplayLines; ++i) {
        TerminalUI::moveCursor(errorStartRow + i, 1);
        TerminalUI::clearLine();
    }
    
    if (!error_.empty()) {
        TerminalUI::setColor(31, -1); // Red
        std::istringstream errorStream(error_);
        std::string line;
        int currentLine = 0;
        bool isTableContent = false;
        
        while (std::getline(errorStream, line) && currentLine < maxErrorDisplayLines) {
            TerminalUI::moveCursor(errorStartRow + currentLine, 1);
            
            // Check if this is table content (contains box-drawing characters or table-related text)
            bool isTableLine = (line.find("┌") != std::string::npos || line.find("│") != std::string::npos || 
                               line.find("├") != std::string::npos || line.find("└") != std::string::npos ||
                               line.find("┴") != std::string::npos || line.find("┬") != std::string::npos ||
                               line.find("┼") != std::string::npos || line.find("─") != std::string::npos);
            
            bool isTableHeader = (line.find("Test Results Comparison:") != std::string::npos);
            bool isSummary = (line.find("Summary:") != std::string::npos);
            
            if (isTableLine || isTableHeader || isSummary) {
                isTableContent = true;
                // Table content - no prefix, no indentation
                std::cout << line;
            } else if (currentLine == 0 && !isTableContent) {
                // First line of non-table error - add "Error: " prefix
                std::cout << "Error: " << line;
            } else if (!isTableContent) {
                // Regular error continuation line - indent
                std::cout << "       " << line;
            } else {
                // After table content, regular text
                std::cout << line;
            }
            currentLine++;
        }
        TerminalUI::resetColor();
    } else if (!success_.empty()) {
        TerminalUI::moveCursor(errorStartRow, 1);
        TerminalUI::setColor(32, -1); // Green
        std::cout << "Success: " << success_;
        TerminalUI::resetColor();
    }
    
    // Status bar at bottom
    TerminalUI::moveCursor(statusBarRow, 1);
    TerminalUI::clearLine();
    std::cout << "Tab/Shift+Tab: Switch tabs | F5: Compile | Enter: Newline | Esc: Back to menu";
    if (activeTab_ == TabbedInterface::Solution) {
        std::cout << " | Template provided";
    }
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
    // Content rendering doesn't need height calculation here
    // The individual render functions handle their own display
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
    // Text editor view with line numbers
    if (solutionText_.empty()) {
        TerminalUI::setColor(37, -1);
        std::cout << "   1 | (Enter your HDL solution here)\n";
        TerminalUI::resetColor();
    } else {
        std::istringstream iss(solutionText_);
        std::string line;
        int lineNum = 1;
        while (std::getline(iss, line)) {
            // Print line number with padding
            TerminalUI::setColor(90, -1); // Dark gray for line numbers
            std::cout << std::setw(4) << std::right << lineNum << " | ";
            TerminalUI::resetColor();
            std::cout << line << "\n";
            lineNum++;
        }
        // Handle case where text doesn't end with newline
        if (!solutionText_.empty() && solutionText_.back() != '\n') {
            TerminalUI::setColor(90, -1);
            std::cout << std::setw(4) << std::right << lineNum << " | ";
            TerminalUI::resetColor();
        }
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
        // Calculate cursor position in solution text with line numbers
        // Count lines before cursor position
        int lines = 3; // Start after tabs (2 lines) + 1 for 1-based
        int col = 1;
        int pos = 0;
        int currentLineNum = 1;
        for (int i = 0; i < cursorCol_ && i < static_cast<int>(solutionText_.length()); ++i) {
            if (solutionText_[i] == '\n') {
                lines++;
                col = 1;
                pos = i + 1;
                currentLineNum++;
            } else {
                col++;
            }
        }
        // Calculate column position on current line
        // Add offset for line number display: "   1 | " = 7 characters
        // Line numbers can be 1-4 digits, so we need to account for that
        int lineNumWidth = (currentLineNum < 10) ? 4 : (currentLineNum < 100) ? 5 : (currentLineNum < 1000) ? 6 : 7;
        col = cursorCol_ - pos + 1 + lineNumWidth + 3; // +3 for " | "
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
        } else if (key.key == Key::ShiftEnter || key.key == Key::F5) {
            // Shift+Enter or F5 - don't handle here, let level editor handle it
            return false; // Signal to parent to handle
        }
    }
    
    return true;
}

