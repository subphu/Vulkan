//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"

class Commander {
    
public:
    ~Commander();
    Commander();
    
    void cleanup();
    
    void setupPool(VkQueue queue, uint32_t queueIndex);
    void create();
    
    VkCommandBuffer              createCommandBuffer();
    std::vector<VkCommandBuffer> createCommandBuffers(uint32_t count);
    
    void beginSingleTimeCommands(VkCommandBuffer commandBuffer);
    void endSingleTimeCommands  (VkCommandBuffer commandBuffer);
    
private:
    
    VkDevice      m_device      = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkQueue       m_queue       = VK_NULL_HANDLE;
    
    VkCommandPoolCreateInfo m_poolInfo{};
    
};
