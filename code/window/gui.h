//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "window.h"

class GUI {
    
public:
    GUI();
    ~GUI();
    
    void cleanup();

    void init(VkRenderPass renderPass);
    
    void setWindow(Window* window);
    
    void draw();
    void render(VkCommandBuffer commandBuffer);
    
private:
    
    Cleaner m_cleaner;
    Window* m_pWindow;
    
    VkDescriptorPool m_imguiPool;
};
