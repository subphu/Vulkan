//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "window.h"

#include "../libraries/imgui/imgui.h"
#include "../libraries/imgui/backends/imgui_impl_glfw.h"
#include "../libraries/imgui/backends/imgui_impl_vulkan.h"

class Settings {
public:
    
    bool ShowDemo  = false;
    bool LockFPS   = false;
    bool LockFocus = false;
    
    float ClearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};
    float ClearDepth    = 1.0f;
    uint  ClearStencil  = 0;
    
public:
    Settings();
    ~Settings();
    
    void cleanup();

    void initGUI(VkRenderPass renderPass);
    
    void setWindow(Window* window);
    
    void drawGUI();
    void renderGUI(VkCommandBuffer commandBuffer);
    
private:
    
    Cleaner m_cleaner;
    Window* m_pWindow;
    

    VkDescriptorPool m_imguiPool;
    
    void drawStatusWindow();
};
