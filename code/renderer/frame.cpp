//  Copyright © 2021 Subph. All rights reserved.
//

#include <array>
#include "frame.h"
#include "../system.h"

Frame::~Frame() {}
Frame::Frame() {
    System &system = System::instance();
    m_device       = system.m_renderer->m_device;
}

void Frame::cleanup() {
    LOG("Frame::cleanup");
    
    m_uniformBuffer->cleanup();
    vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
    m_depthImage->cleanup();
    m_image->cleanupImageView();
}

void Frame::createImageResource(VkImage image, VkFormat format) {
    m_image = new Image();
    m_image->setupForSwapchain(image, format);
    m_image->createForSwapchain();
}

void Frame::createDepthResource() {
    m_depthImage = new Image();
    m_depthImage->setupForDepth(m_size, 1);
    m_depthImage->create();
}

void Frame::createFramebuffer(VkRenderPass renderPass) {
    LOG("createFramebuffer");
    VkDevice       device     = m_device;
    Size<uint32_t> size       = m_size;
    Image* frameImage = m_image;
    Image* depthImage = m_depthImage;
    
    int attachmentCount = 2;
    VkImageView attachments[] = {frameImage->getImageView(), depthImage->getImageView()};
    
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass      = renderPass;
    framebufferInfo.attachmentCount = attachmentCount;
    framebufferInfo.pAttachments    = attachments;
    framebufferInfo.width           = size.width;
    framebufferInfo.height          = size.height;
    framebufferInfo.layers          = 1;
    
    VkFramebuffer framebuffer;
    VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);
    CHECK_VKRESULT(result, "failed to create framebuffer!");
    
    { m_framebuffer = framebuffer; }
}

void Frame::createUniformBuffer(VkDeviceSize bufferSize) {
    m_uniformBuffer = new Buffer();
    m_uniformBuffer->setup(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    m_uniformBuffer->create();
}

void Frame::updateUniformBuffer(void* address, size_t size) {
    m_uniformBuffer->fillBuffer(address, size);
}

VkDescriptorBufferInfo Frame::getBufferInfo() {
    Buffer* uniformBuffer = m_uniformBuffer;
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffer->getBuffer();
    bufferInfo.range  = uniformBuffer->getBufferSize();
    bufferInfo.offset = 0;
    return bufferInfo;
}

void Frame::setDescriptorSet(VkDescriptorSet descriptorSet) {
    m_descriptorSet = descriptorSet;
}

void Frame::setCommandBuffer(VkCommandBuffer commandBuffers) {
    m_commandBuffer = commandBuffers;
}

void Frame::setSize(Size<uint32_t> size) {
    m_size = size;
}
