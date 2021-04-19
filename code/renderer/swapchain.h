//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "renderer.h"
#include "../resources/image.h"
#include "../resources/buffer.h"
#include "../resources/shader.h"
#include "pipeline_graphic.h"
#include "frame.h"
#include "descriptor.h"

class Swapchain {
    
public:
    Swapchain();
    ~Swapchain();
    
    void cleanup();
    
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    
    VkSwapchainCreateInfoKHR m_swapchainInfo{};
    VkExtent2D m_extent;
    VkFormat m_surfaceFormat;
    void setup(Size<int> size, VkSurfaceKHR surface);
    
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    void create();
    
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    void createRenderPass();

    uint m_totalFrame;
    std::vector<Frame*> m_frames;
    void createFrames(VkDeviceSize uniformBufferSize);
    
    std::vector<VkSemaphore> m_imageSemaphores;
    void createSyncObjects();
    
    VkRenderPassBeginInfo getRenderBeginInfo();
    
private:
    
    static std::vector<VkImage> GetSwapchainImages(VkSwapchainKHR swapchain);
};
