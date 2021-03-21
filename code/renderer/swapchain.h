//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "renderer.h"
#include "resource_image.h"
#include "buffer.h"
#include "shader.h"
#include "pipeline_graphics.h"

class Swapchain {
    
public:
    Swapchain();
    ~Swapchain();
    
    void cleanup();
    
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    
    std::vector<VkCommandBuffer> m_commandBuffers;
    
    VkSwapchainCreateInfoKHR m_swapchainInfo{};
    VkExtent2D m_swapchainExtent;
    VkFormat m_swapchainSurfaceFormat;
    void setup(Size<int> size);
    
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    void create();
    
    uint32_t m_totalFrame = 0;
    std::vector<ResourceImage*> m_swapchainImageResources;
    void createSwapchainImages();
    
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    void createRenderPass();
    
    
    PipelineGraphics* m_pipelineGraphics = nullptr;
    void createPipeline(std::vector<Shader*> shaders, VkPipelineVertexInputStateCreateInfo* vertexInputInfo);
    
    uint32_t m_mipLevels = 1;
    
    ResourceImage *m_depthResource = nullptr;
    void createDepthResources();
    
    std::vector<VkFramebuffer> m_framebuffers;
    void createFramebuffer();
    
    std::vector<Buffer*> m_uniformBuffers;
    void createUniformBuffers(VkDeviceSize bufferSize);
    void updateUniformBuffer(void* address, size_t size, uint32_t index);
    
    
    
    VkDescriptorPool m_descriptorPool;
    void createDescriptorPool();
    
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    void createDescriptorSetLayout();
    
    std::vector<VkDescriptorSet> m_descriptorSets;
    void createDescriptorSets(VkDeviceSize uniformBufferSize, VkImageView textureImageView, VkSampler textureSampler);
    
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    void createSyncObjects();
    
private:
    
    
};
