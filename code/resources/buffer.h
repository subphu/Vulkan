//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"

class Buffer {
    
public:
    ~Buffer();
    Buffer();

    void cleanup();
    
    unsigned char* m_desc;
    
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    
    VkBuffer         m_buffer         = VK_NULL_HANDLE;
    VkDeviceMemory   m_bufferMemory   = VK_NULL_HANDLE;
    
    VkBufferCreateInfo m_bufferInfo{};
    
    VkBuffer       getBuffer();
    VkDeviceSize   getBufferSize();
    VkDeviceMemory getBufferMemory();
    VkDescriptorBufferInfo getBufferInfo();
    
    void setup (VkDeviceSize size, VkBufferUsageFlags usage);
    void create();
    
    void createBuffer();
    void allocateBufferMemory();
    
    void cmdCopyFromBuffer(VkBuffer sourceBuffer, VkDeviceSize size);
    
    void fillBuffer    (const void* address, size_t size, uint32_t shift = 0);
    void fillBufferFull(const void* address);
    
    void* mapMemory(size_t size);
    void  unmapMemory();
    
    
private:
    
    VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
    VkDeviceMemory allocateBufferMemory(VkBuffer& buffer, VkDeviceSize size, uint32_t memoryTypeIndex);
    
    static VkBufferCreateInfo GetDefaultBufferCreateInfo();
};
