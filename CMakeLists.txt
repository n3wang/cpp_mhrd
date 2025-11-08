cmake_minimum_required(VERSION 3.10)
project(mhrd-cli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(mhrd-cli 
    src/mhrd_cli.cpp
    src/simulator.cpp
    src/game.cpp
    src/terminal_ui.cpp
    src/level_editor.cpp
    src/syntax_checker.cpp
)

# Test executable for UI controls
add_executable(test-ui-controls
    tests/test_ui_controls.cpp
    src/terminal_ui.cpp
    src/level_editor.cpp
    src/game.cpp
    src/simulator.cpp
    src/syntax_checker.cpp
)

# Integration test for editor
add_executable(test-editor-integration
    tests/test_editor_integration.cpp
    src/terminal_ui.cpp
    src/level_editor.cpp
    src/game.cpp
    src/simulator.cpp
    src/syntax_checker.cpp
)

install(TARGETS mhrd-cli RUNTIME DESTINATION bin)
install(FILES examples/not.hdl DESTINATION share/mhrd-cli/examples)

