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
    m_uniformBuffer->cleanup();
    vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
    m_depthImage->cleanup();
    m_resourceImage->cleanupImageView();
//    m_texture->cleanup();
}

void Frame::createImageResource(VkImage image, VkFormat format) {
    m_resourceImage = new ResourceImage();
    m_resourceImage->setupForSwapchain(image, format);
    m_resourceImage->createForSwapchain();
}

void Frame::createDepthResource(uint32_t mipLevels) {
    m_depthImage = new ResourceImage();
    m_depthImage->setupForDepth(m_size, mipLevels);
    m_depthImage->create();
}

void Frame::createFramebuffer(VkRenderPass renderPass) {
    VkDevice       device     = m_device;
    Size<uint32_t> size       = m_size;
    ResourceImage* frameImage = m_resourceImage;
    ResourceImage* depthImage = m_depthImage;
    
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

void Frame::updateDescriptorSet(VkDescriptorSet descriptorSet) {
    VkDevice       device        = m_device;
    Buffer*        uniformBuffer = m_uniformBuffer;
    ResourceImage* texture       = m_texture;
    
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffer->getBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range  = uniformBuffer->getBufferSize();
    
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView   = texture->getImageView();
    imageInfo.sampler     = texture->getSampler();
    
    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    descriptorWrites[0].sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding      = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].pBufferInfo     = &bufferInfo;
    
    descriptorWrites[1].sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding      = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].pImageInfo      = &imageInfo;
    
    vkUpdateDescriptorSets(device, 2, descriptorWrites.data(), 0, nullptr);
    m_descriptorSet = descriptorSet;
}

void Frame::createUniformBuffer(VkDeviceSize bufferSize) {
    m_uniformBuffer = new Buffer();
    m_uniformBuffer->setup(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    m_uniformBuffer->create();
}

void Frame::updateUniformBuffer(void* address, size_t size) {
    m_uniformBuffer->fillBuffer(address, size);
}

void Frame::setCommandBuffer(VkCommandBuffer commandBuffers) {
    m_commandBuffer = commandBuffers;
}

void Frame::setSize(Size<uint32_t> size) {
    m_size = size;
}

void Frame::setTexture(ResourceImage* texture) {
    m_texture = texture;
}
