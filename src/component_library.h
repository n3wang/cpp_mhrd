#ifndef COMPONENT_LIBRARY_H
#define COMPONENT_LIBRARY_H

#include <string>
#include <vector>
#include <unordered_map>
#include "simulator.h"

struct Component {
    std::string name;
    std::string description;
    std::string hdlContent;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    AST ast;  // Parsed AST for the component
    Net net;  // Built net for simulation
    
    // Metadata
    std::string author;
    std::string createdDate;
};

class ComponentLibrary {
public:
    ComponentLibrary();
    ~ComponentLibrary();
    
    // Load all components from the components directory
    bool loadComponents(const std::string& componentsDir);
    
    // Save a component to disk
    bool saveComponent(const Component& component, const std::string& componentsDir);
    
    // Get all loaded components
    std::vector<Component> getAllComponents() const;
    
    // Get a component by name
    Component* getComponent(const std::string& name);
    const Component* getComponent(const std::string& name) const;
    
    // Check if a component exists
    bool hasComponent(const std::string& name) const;
    
    // Delete a component
    bool deleteComponent(const std::string& name, const std::string& componentsDir);
    
    // Get component directory path
    static std::string getComponentsDirectory();
    
private:
    std::unordered_map<std::string, Component> components_;
    bool parseComponentFile(const std::string& filePath, Component& component);
    bool validateComponent(const Component& component) const;
};

#endif

