//  Copyright © 2021 Subph. All rights reserved.
//

#include "commander.h"

#include "../system.h"

Commander::~Commander() {}
Commander::Commander() {
    System &system   = System::instance();
    m_device         = system.m_renderer->m_device;
}

void Commander::cleanup() {
    LOG("Commander::cleanup");
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
}

void Commander::setupPool(VkQueue queue, uint32_t queueIndex) {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueIndex;
    poolInfo.flags            = 0;
    m_poolInfo = poolInfo;
    m_queue    = queue;
}

void Commander::create() {
    LOG("createCommandPool");
    VkResult result = vkCreateCommandPool(m_device, &m_poolInfo, nullptr, &m_commandPool);
    CHECK_VKRESULT(result, "failed to create command pool!");
}

VkCommandBuffer Commander::createCommandBuffer() {
    LOG("createCommandBuffer");
    VkDevice      device      = m_device;
    VkCommandPool commandPool = m_commandPool;
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    return commandBuffer;
}

std::vector<VkCommandBuffer> Commander::createCommandBuffers(uint32_t size) {
    LOG("createCommandBuffers");
    VkDevice      device      = m_device;
    VkCommandPool commandPool = m_commandPool;
    
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = size;
    
    std::vector<VkCommandBuffer> commandBuffers;
    commandBuffers.resize(size);
    
    vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
    return commandBuffers;
}

void Commander::beginSingleTimeCommands(VkCommandBuffer commandBuffer) {
    LOG("beginSingleTimeCommands");
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

void Commander::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    LOG("endSingleTimeCommands");
    VkDevice      device      = m_device;
    VkQueue       queue       = m_queue;
    VkCommandPool commandPool = m_commandPool;
    
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;
    
    vkQueueSubmit  (queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
