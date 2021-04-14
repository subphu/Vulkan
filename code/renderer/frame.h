//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "../resources/image.h"
#include "../resources/buffer.h"

class Frame {
    
public:
    Frame();
    ~Frame();
    
    void cleanup();
    
    VkDevice m_device = VK_NULL_HANDLE;
    
    VkFramebuffer   m_framebuffer   = VK_NULL_HANDLE;
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
    Buffer* m_uniformBuffer = nullptr;
    Image*  m_image         = nullptr;
    Image*  m_depthImage    = nullptr;
    Size<uint32_t>  m_size{};
    
    VkSemaphore m_renderSemaphore;
    VkFence     m_commandFence;
    
    
    void createDepthResource();
    void createImageResource(VkImage image, VkFormat format);
    void createFramebuffer(VkRenderPass renderPass);
    void createFinishSignal();
    
    void createUniformBuffer(VkDeviceSize bufferSize);
    void updateUniformBuffer(void* address, size_t size);
    
    void setDescriptorSet(VkDescriptorSet descriptorSet);
    void setCommandBuffer(VkCommandBuffer commandBuffers);
    
    void setSize(Size<uint32_t> size);
    
    VkDescriptorBufferInfo getBufferInfo();
};
