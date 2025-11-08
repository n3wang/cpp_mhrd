#include "component_library.h"
#include "simulator.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <regex>
#include <cstdlib>
#include <algorithm>

namespace fs = std::filesystem;

ComponentLibrary::ComponentLibrary() {
}

ComponentLibrary::~ComponentLibrary() {
}

std::string ComponentLibrary::getComponentsDirectory() {
    // Try to find components directory relative to executable or in home directory
    std::string componentsDir;
    
    // First, try relative to executable (for development)
    try {
        if (fs::exists("/proc/self/exe")) {
            fs::path exePath = fs::canonical("/proc/self/exe");
            fs::path projectRoot = exePath.parent_path().parent_path();
            std::string candidateDir = (projectRoot / "components").string();
            if (fs::exists(candidateDir)) {
                return candidateDir;
            }
        }
    } catch (...) {
        // Fall through
    }
    
    // Try relative path
    if (fs::exists("components")) {
        return "components";
    }
    
    // Use home directory
    const char* home = std::getenv("HOME");
    if (home) {
        std::string homeComponents = std::string(home) + "/.minlab/components";
        if (!fs::exists(homeComponents)) {
            fs::create_directories(homeComponents);
        }
        return homeComponents;
    }
    
    // Final fallback
    return "components";
}

bool ComponentLibrary::parseComponentFile(const std::string& filePath, Component& component) {
    std::ifstream file(filePath);
    if (!file) {
        return false;
    }
    
    std::string line;
    std::ostringstream hdlContent;
    std::string metadataSection;
    bool inMetadata = false;
    
    // Parse metadata header (lines starting with #)
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        if (line[0] == '#') {
            inMetadata = true;
            metadataSection += line + "\n";
            
            // Parse metadata fields
            if (line.find("# Name:") == 0) {
                component.name = line.substr(7);
                // Trim whitespace
                component.name.erase(0, component.name.find_first_not_of(" \t"));
                component.name.erase(component.name.find_last_not_of(" \t") + 1);
            } else if (line.find("# Description:") == 0) {
                component.description = line.substr(14);
                component.description.erase(0, component.description.find_first_not_of(" \t"));
                component.description.erase(component.description.find_last_not_of(" \t") + 1);
            } else if (line.find("# Author:") == 0) {
                component.author = line.substr(9);
                component.author.erase(0, component.author.find_first_not_of(" \t"));
                component.author.erase(component.author.find_last_not_of(" \t") + 1);
            } else if (line.find("# Created:") == 0) {
                component.createdDate = line.substr(10);
                component.createdDate.erase(0, component.createdDate.find_first_not_of(" \t"));
                component.createdDate.erase(component.createdDate.find_last_not_of(" \t") + 1);
            }
        } else {
            // Start of HDL content
            if (inMetadata) {
                inMetadata = false;
            }
            hdlContent << line << "\n";
        }
    }
    
    component.hdlContent = hdlContent.str();
    
    // Parse the HDL to get inputs and outputs
    try {
        component.ast = parseHDL(component.hdlContent);
        component.inputs = component.ast.inputs;
        component.outputs = component.ast.outputs;
        
        // Build net for simulation
        component.net = buildNet(component.ast);
        
        return true;
    } catch (const std::exception& e) {
        // Component has invalid HDL
        return false;
    }
}

bool ComponentLibrary::validateComponent(const Component& component) const {
    // Check that component only uses NAND gates or other custom components
    for (const auto& part : component.ast.parts) {
        std::string kindLower = part.kind;
        std::transform(kindLower.begin(), kindLower.end(), kindLower.begin(), ::tolower);
        
        // Allow NAND gates
        if (kindLower == "nand") {
            continue;
        }
        
        // Allow other custom components
        if (hasComponent(kindLower)) {
            continue;
        }
        
        // Disallow other built-in gates
        if (kindLower == "not" || kindLower == "and" || kindLower == "or" || 
            kindLower == "xor" || kindLower == "nor") {
            return false;
        }
        
        // Unknown component type
        if (!hasComponent(kindLower)) {
            return false;
        }
    }
    
    return true;
}

bool ComponentLibrary::loadComponents(const std::string& componentsDir) {
    components_.clear();
    
    if (!fs::exists(componentsDir)) {
        fs::create_directories(componentsDir);
        return true;  // Directory created, no components to load
    }
    
    try {
        for (const auto& entry : fs::directory_iterator(componentsDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".hdl") {
                Component component;
                if (parseComponentFile(entry.path().string(), component)) {
                    if (validateComponent(component)) {
                        components_[component.name] = component;
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        return false;
    }
    
    return true;
}

bool ComponentLibrary::saveComponent(const Component& component, const std::string& componentsDir) {
    if (!fs::exists(componentsDir)) {
        fs::create_directories(componentsDir);
    }
    
    std::string filePath = (fs::path(componentsDir) / (component.name + ".hdl")).string();
    
    std::ofstream file(filePath);
    if (!file) {
        return false;
    }
    
    // Write metadata header
    file << "# Component Definition\n";
    file << "# Name: " << component.name << "\n";
    file << "# Description: " << component.description << "\n";
    if (!component.author.empty()) {
        file << "# Author: " << component.author << "\n";
    }
    
    // Get current date/time
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream dateStream;
    dateStream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    file << "# Created: " << dateStream.str() << "\n";
    file << "\n";
    
    // Write HDL content
    file << component.hdlContent;
    
    file.close();
    
    // Reload components to include the new one
    loadComponents(componentsDir);
    
    return true;
}

bool ComponentLibrary::deleteComponent(const std::string& name, const std::string& componentsDir) {
    std::string filePath = (fs::path(componentsDir) / (name + ".hdl")).string();
    
    if (fs::exists(filePath)) {
        fs::remove(filePath);
        components_.erase(name);
        return true;
    }
    
    return false;
}

std::vector<Component> ComponentLibrary::getAllComponents() const {
    std::vector<Component> result;
    for (const auto& [name, component] : components_) {
        result.push_back(component);
    }
    return result;
}

Component* ComponentLibrary::getComponent(const std::string& name) {
    auto it = components_.find(name);
    if (it != components_.end()) {
        return &it->second;
    }
    return nullptr;
}

const Component* ComponentLibrary::getComponent(const std::string& name) const {
    auto it = components_.find(name);
    if (it != components_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool ComponentLibrary::hasComponent(const std::string& name) const {
    return components_.find(name) != components_.end();
}

