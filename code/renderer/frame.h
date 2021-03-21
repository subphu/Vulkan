//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "resource_image.h"
#include "buffer.h"

class Frame {
    
public:
    Frame();
    ~Frame();
    
    void cleanup();
    
    VkDevice m_device = VK_NULL_HANDLE;
    
    VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
    VkFramebuffer   m_framebuffer   = VK_NULL_HANDLE;
    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
    Buffer*         m_uniformBuffer = nullptr;
    ResourceImage*  m_resourceImage = nullptr;
    ResourceImage*  m_depthImage    = nullptr;
    ResourceImage*  m_texture       = nullptr;
    Size<uint32_t>  m_size{};
    
    void createImageResource(VkImage image, VkFormat format);
    void createDepthResource(uint32_t mipLevels);
    void createFramebuffer(VkRenderPass renderPass);
    void updateDescriptorSet(VkDescriptorSet descriptorSet);
    
    void createUniformBuffer(VkDeviceSize bufferSize);
    void updateUniformBuffer(void* address, size_t size);
    
    void setCommandBuffer(VkCommandBuffer commandBuffers);
    
    void setSize(Size<uint32_t> size);
    void setTexture(ResourceImage* texture);
};
