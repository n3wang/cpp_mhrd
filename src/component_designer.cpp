#include "component_designer.h"
#include "level_editor.h"
#include "game.h"
#include "simulator.h"
#include "syntax_checker.h"
#include <iostream>
#include <sstream>
#include <algorithm>

ComponentDesigner::ComponentDesigner(ComponentLibrary& library) 
    : library_(library) {
}

std::string ComponentDesigner::getComponentEditorTemplate() {
    std::ostringstream oss;
    oss << "// Component HDL Definition\n";
    oss << "// This component can only use NAND gates and other custom components\n";
    oss << "// You can define any number of inputs and outputs\n\n";
    oss << "Inputs: in;\n";
    oss << "Outputs: out;\n";
    oss << "Parts: g1:nand;\n";
    oss << "Wires: in->g1.in1, in->g1.in2, g1.out->out;\n";
    return oss.str();
}

bool ComponentDesigner::validateComponentHDL(const std::string& hdl) {
    // Check syntax
    SyntaxError syntaxError = checkSyntax(hdl);
    if (syntaxError.hasError) {
        return false;
    }
    
    // Parse and check that only NAND gates and custom components are used
    try {
        AST ast = parseHDL(hdl);
        
        for (const auto& part : ast.parts) {
            std::string kindLower = part.kind;
            std::transform(kindLower.begin(), kindLower.end(), kindLower.begin(), ::tolower);
            
            // Allow NAND gates
            if (kindLower == "nand") {
                continue;
            }
            
            // Allow custom components
            if (library_.hasComponent(kindLower)) {
                continue;
            }
            
            // Disallow other built-in gates
            return false;
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool ComponentDesigner::saveComponent() {
    if (componentName_.empty()) {
        return false;
    }
    
    // Validate HDL
    if (!validateComponentHDL(hdlContent_)) {
        return false;
    }
    
    Component component;
    component.name = componentName_;
    component.description = componentDescription_;
    component.hdlContent = hdlContent_;
    
    // Parse to get inputs/outputs
    try {
        component.ast = parseHDL(hdlContent_);
        component.inputs = component.ast.inputs;
        component.outputs = component.ast.outputs;
        component.net = buildNetWithComponents(component.ast, &library_);
    } catch (const std::exception&) {
        return false;
    }
    
    std::string componentsDir = ComponentLibrary::getComponentsDirectory();
    return library_.saveComponent(component, componentsDir);
}

void ComponentDesigner::createNewComponent() {
    TerminalUI::clearScreen();
    
    // Get component name
    TerminalUI::setColor(37, -1);
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║           Create New Component                          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";
    TerminalUI::resetColor();
    
    std::cout << "Component Name: ";
    std::cout.flush();
    TerminalUI::showCursor();
    std::getline(std::cin, componentName_);
    
    if (componentName_.empty()) {
        return;
    }
    
    // Check if component already exists
    if (library_.hasComponent(componentName_)) {
        TerminalUI::setColor(31, -1);
        std::cout << "Error: Component '" << componentName_ << "' already exists!\n";
        TerminalUI::resetColor();
        std::cout << "Press Enter to continue...";
        std::cin.ignore();
        return;
    }
    
    std::cout << "Description: ";
    std::cout.flush();
    std::getline(std::cin, componentDescription_);
    
    // Use level editor to edit the component HDL
    hdlContent_ = getComponentEditorTemplate();
    
    // Create a temporary "level" for the editor
    Level tempLevel;
    tempLevel.id = "component_" + componentName_;
    tempLevel.name = "Component: " + componentName_;
    tempLevel.description = componentDescription_;
    tempLevel.difficulty = 0;
    tempLevel.available_gates = {"nand"};  // Only NAND gates allowed
    tempLevel.inputs = {};  // Empty - will be determined from HDL
    tempLevel.outputs = {};  // Empty - will be determined from HDL
    tempLevel.expected = {};  // Empty expected = component design mode
    
    // Add custom components to available gates
    auto allComponents = library_.getAllComponents();
    for (const auto& comp : allComponents) {
        tempLevel.available_gates.push_back(comp.name);
    }
    
    Game tempGame;
    LevelEditor editor(tempGame, tempLevel);
    editor.setSolutionText(hdlContent_);
    
    TerminalUI::init();
    editor.run();  // Run the editor (returns false when user exits)
    hdlContent_ = editor.getSolutionText();
    
    TerminalUI::cleanup();
    
    // Try to save
    if (saveComponent()) {
        TerminalUI::setColor(32, -1);
        std::cout << "\nComponent '" << componentName_ << "' saved successfully!\n";
        TerminalUI::resetColor();
    } else {
        TerminalUI::setColor(31, -1);
        std::cout << "\nError: Failed to save component! Make sure it only uses NAND gates and custom components.\n";
        TerminalUI::resetColor();
    }
    std::cout << "Press Enter to continue...";
    std::cin.ignore();
}

void ComponentDesigner::showComponentList() {
    TerminalUI::clearScreen();
    
    TerminalUI::setColor(37, -1);
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║              Component Library                           ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";
    TerminalUI::resetColor();
    
    auto components = library_.getAllComponents();
    
    if (components.empty()) {
        std::cout << "No components found. Create your first component!\n\n";
    } else {
        for (size_t i = 0; i < components.size(); ++i) {
            const auto& comp = components[i];
            std::cout << "  " << (i + 1) << ". " << comp.name;
            if (!comp.description.empty()) {
                std::cout << " - " << comp.description;
            }
            std::cout << "\n";
            std::cout << "     Inputs: ";
            for (size_t j = 0; j < comp.inputs.size(); ++j) {
                if (j > 0) std::cout << ", ";
                std::cout << comp.inputs[j];
            }
            std::cout << " | Outputs: ";
            for (size_t j = 0; j < comp.outputs.size(); ++j) {
                if (j > 0) std::cout << ", ";
                std::cout << comp.outputs[j];
            }
            std::cout << "\n\n";
        }
    }
    
    std::cout << "Press Enter to continue...";
    std::cin.ignore();
}

bool ComponentDesigner::run() {
    TerminalUI::init();
    
    while (true) {
        TerminalUI::clearScreen();
        
        Menu menu("╔══════════════════════════════════════════════════════════╗\n"
                  "║           Component Designer                            ║\n"
                  "╚══════════════════════════════════════════════════════════╝");
        
        menu.addOption("Create New Component", "create");
        menu.addOption("View Component Library", "list");
        menu.addOption("Back to Main Menu", "back");
        
        menu.setHighlight(37, -1);
        menu.setSelectedHighlight(30, 47);
        
        int choice = menu.show();
        
        if (choice < 0) {
            TerminalUI::cleanup();
            return false;
        }
        
        std::string choiceId = menu.getOptionCount() > 0 ? menu.getOption(choice).id : "";
        
        if (choiceId == "back" || choiceId.empty()) {
            TerminalUI::cleanup();
            return false;
        } else if (choiceId == "create") {
            TerminalUI::cleanup();
            createNewComponent();
            TerminalUI::init();
        } else if (choiceId == "list") {
            TerminalUI::cleanup();
            showComponentList();
            TerminalUI::init();
        }
    }
    
    return false;
}

