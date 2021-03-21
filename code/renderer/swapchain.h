//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "renderer.h"
#include "resource_image.h"
#include "buffer.h"
#include "shader.h"
#include "pipeline_graphics.h"
#include "frame.h"

class Swapchain {
    
public:
    Swapchain();
    ~Swapchain();
    
    void cleanup();
    
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    
    uint32_t m_mipLevels = 1;
    
    VkSwapchainCreateInfoKHR m_swapchainInfo{};
    VkExtent2D m_extent;
    VkFormat m_surfaceFormat;
    void setup(Size<int> size);
    
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    void create();
    
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    void createRenderPass();
    
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    void createDescriptorSetLayout();
    
    PipelineGraphics* m_pipelineGraphics = nullptr;
    void createPipeline(std::vector<Shader*> shaders, VkPipelineVertexInputStateCreateInfo* vertexInputInfo);
    
    std::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;
    VkDescriptorPool m_descriptorPool;
    std::vector<Frame*> m_frames;
    void createFrames(VkDeviceSize uniformBufferSize, ResourceImage* texture);
    
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    void createSyncObjects();
    
private:
    
    
    static VkDescriptorPool CreateDescriptorPool(uint32_t count,
                                                 std::vector<VkDescriptorSetLayoutBinding> layoutBindings);
    
    static std::vector<VkDescriptorSet> AllocateDescriptorSets(uint32_t count,
                                                               VkDescriptorSetLayout descriptorSetLayout,
                                                               VkDescriptorPool descriptorPool);
    
    static std::vector<VkImage> GetSwapchainImages(VkSwapchainKHR swapchain);
};
