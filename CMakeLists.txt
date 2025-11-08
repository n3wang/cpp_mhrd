cmake_minimum_required(VERSION 3.10)
project(minlab 
    VERSION 1.0.0
    DESCRIPTION "Interactive HDL puzzle game and circuit simulator"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(minlab 
    src/minlab.cpp
    src/simulator.cpp
    src/game.cpp
    src/terminal_ui.cpp
    src/level_editor.cpp
    src/syntax_checker.cpp
    src/component_library.cpp
    src/component_designer.cpp
)

# Test executable for UI controls
add_executable(test-ui-controls
    tests/test_ui_controls.cpp
    src/terminal_ui.cpp
    src/level_editor.cpp
    src/game.cpp
    src/simulator.cpp
    src/syntax_checker.cpp
    src/component_library.cpp
)

# Integration test for editor
add_executable(test-editor-integration
    tests/test_editor_integration.cpp
    src/terminal_ui.cpp
    src/level_editor.cpp
    src/game.cpp
    src/simulator.cpp
    src/syntax_checker.cpp
    src/component_library.cpp
)

install(TARGETS minlab RUNTIME DESTINATION bin)

# Install examples
install(FILES examples/not.hdl DESTINATION share/minlab/examples)

# Install levels directory
install(DIRECTORY levels/ DESTINATION share/minlab/levels
    FILES_MATCHING PATTERN "*.json"
)

