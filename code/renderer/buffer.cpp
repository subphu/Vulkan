//  Copyright © 2021 Subph. All rights reserved.
//

#include "buffer.h"

#include "../helper.h"
#include "../system.h"

Buffer::~Buffer() {}
Buffer::Buffer() : m_bufferInfo(GetDefaultBufferCreateInfo()) {
    System &system   = System::instance();
    m_device         = system.m_renderer->m_device;
    m_physicalDevice = system.m_renderer->m_physicalDevice;
}

void Buffer::cleanup() {
    LOG("buffer cleanup");
    vkDestroyBuffer   (m_device, m_buffer      , nullptr);
    vkFreeMemory      (m_device, m_bufferMemory, nullptr);
}

VkBuffer       Buffer::getBuffer      () { return m_buffer;       }
VkDeviceMemory Buffer::getBufferMemory() { return m_bufferMemory; }
VkDeviceSize   Buffer::getBufferSize  () { return m_bufferInfo.size; }

void Buffer::setup(VkDeviceSize size, VkBufferUsageFlags usage) {
    VkBufferCreateInfo bufferInfo = m_bufferInfo;
    
    bufferInfo.size  = size;
    bufferInfo.usage = usage;
    
    m_bufferInfo = bufferInfo;
}

void Buffer::create() {
    createBuffer();
    allocateBufferMemory();
}

void Buffer::createBuffer() {
    LOG("createBuffer");
    VkResult result = vkCreateBuffer(m_device, &m_bufferInfo, nullptr, &m_buffer);
    CHECK_VKRESULT(result, "failed to buffer!");
}

void Buffer::allocateBufferMemory() {
    VkDevice         device         = m_device;
    VkPhysicalDevice physicalDevice = m_physicalDevice;
    VkBuffer         buffer         = m_buffer;
    
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);
    
    uint32_t memoryTypeIndex;
    memoryTypeIndex = FindMemoryTypeIndex(physicalDevice,
                                          memoryRequirements.memoryTypeBits,
                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memoryRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    VkDeviceMemory bufferMemory;
    VkResult       result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory);
    CHECK_VKRESULT(result, "failed to allocate buffer memory!");
    
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
    
    { m_bufferMemory = bufferMemory; }
}

void Buffer::cmdCopyFromBuffer(VkBuffer sourceBuffer, VkDeviceSize size) {
    VkBuffer     buffer     = m_buffer;
    System&      system     = System::instance();
    Commander*   commander  = system.getCommander();
    
    VkCommandBuffer commandBuffer = commander->createCommandBuffer();
    commander->beginSingleTimeCommands(commandBuffer);
    VkBufferCopy    copyRegion = { 0, 0, size };
    vkCmdCopyBuffer(commandBuffer, sourceBuffer, buffer, 1, &copyRegion);
    commander->endSingleTimeCommands(commandBuffer);
}

void Buffer::fillBuffer(const void* address, size_t size, uint32_t shift) {
    void* ptr = mapMemory(size);
    ptr = static_cast<char*>(ptr) + shift;
    memcpy(ptr, address, size);
    unmapMemory();
}

void Buffer::fillBufferFull(const void* address) {
    fillBuffer(address, static_cast<size_t>(m_bufferInfo.size));
}

void* Buffer::mapMemory(size_t size) {
    void* ptr;
    vkMapMemory(m_device, m_bufferMemory, 0, size, 0, &ptr);
    return ptr;
}

void Buffer::unmapMemory() {
    vkUnmapMemory(m_device, m_bufferMemory);
}

// ======================================================================================

VkBufferCreateInfo Buffer::GetDefaultBufferCreateInfo() {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    return bufferInfo;
}
