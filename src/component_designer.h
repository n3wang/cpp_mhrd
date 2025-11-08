#ifndef COMPONENT_DESIGNER_H
#define COMPONENT_DESIGNER_H

#include "terminal_ui.h"
#include "component_library.h"
#include <string>

class ComponentDesigner {
public:
    ComponentDesigner(ComponentLibrary& library);
    bool run();
    
private:
    ComponentLibrary& library_;
    std::string componentName_;
    std::string componentDescription_;
    std::string hdlContent_;
    
    void showComponentList();
    void createNewComponent();
    void editComponent(const std::string& componentName);
    void deleteComponent(const std::string& componentName);
    void viewComponent(const std::string& componentName);
    std::string getComponentEditorTemplate();
    bool saveComponent();
    bool validateComponentHDL(const std::string& hdl);
};

#endif

