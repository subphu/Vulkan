//  Copyright © 2021 Subph. All rights reserved.
//

#include "image.h"

#include "../helper.h"
#include "../system.h"
#include "buffer.h"

Image::~Image() {}
Image::Image() : m_imageInfo(GetDefaultImageCreateInfo()),
                                 m_imageViewInfo(GetDefaultImageViewCreateInfo()) {
    System &system   = System::instance();
    m_device         = system.getRenderer()->m_device;
    m_physicalDevice = system.getRenderer()->m_physicalDevice;
}

void Image::cleanup() {
    LOG("Image::cleanup");
    cleanupImageView();
    if (m_image == VK_NULL_HANDLE) return;
    vkDestroyImage(m_device, m_image, nullptr);
    if (m_sampler == VK_NULL_HANDLE) return;
    vkDestroySampler(m_device, m_sampler, nullptr);
}

void Image::cleanupImageView() {
    LOG("Image::cleanupImageView");
    vkDestroyImageView(m_device, m_imageView  , nullptr);
    vkFreeMemory      (m_device, m_imageMemory, nullptr);
}

void Image::setupForDepth(Size<uint32_t> size, uint32_t mipLevels) {
    LOG("Image::setupForDepth");
    VkPhysicalDevice      physicalDevice = m_physicalDevice;
    VkImageCreateInfo     imageInfo      = m_imageInfo;
    VkImageViewCreateInfo imageViewInfo  = m_imageViewInfo;
    
    VkFormat depthFormat = ChooseDepthFormat(physicalDevice);
    
    imageInfo.extent.width  = size.width;
    imageInfo.extent.height = size.height;
    imageInfo.mipLevels     = mipLevels;
    imageInfo.format        = depthFormat;
    imageInfo.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    
    imageViewInfo.format = depthFormat;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    {
        m_imageInfo     = imageInfo;
        m_imageViewInfo = imageViewInfo;
    }
}

void Image::setupForTexture(const std::string filepath) {
    LOG("Image::setupForTexture");
    int width, height, channels;
    unsigned char*  data      = LoadImage(filepath, &width, &height, &channels);
    uint32_t        mipLevels = MaxMipLevel(width, height);
    
    VkImageCreateInfo     imageInfo      = m_imageInfo;
    VkImageViewCreateInfo imageViewInfo  = m_imageViewInfo;
    
    imageInfo.extent.width  = width;
    imageInfo.extent.height = height;
    imageInfo.mipLevels     = mipLevels;
    imageInfo.format        = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.usage         = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT;
    
    imageViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageViewInfo.subresourceRange.levelCount = mipLevels;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    
    {
        m_rawData       = data;
        m_imageInfo     = imageInfo;
        m_imageViewInfo = imageViewInfo;
    }
}

void Image::setupForCubemap(const std::string *filepaths) {
    LOG("Image::setupForCubemap");
    int width = 0, height = 0, channels = 0;
    std::vector<unsigned char*> data;
    
    for (int i = 0; i < 6; ++i) {
        data.push_back(LoadImage(filepaths[i], &width, &height, &channels));
    }
    uint32_t mipLevels = MaxMipLevel(width, height);
    
    VkImageCreateInfo     imageInfo      = m_imageInfo;
    VkImageViewCreateInfo imageViewInfo  = m_imageViewInfo;
    
    imageInfo.arrayLayers   = 6;
    imageInfo.extent.width  = width;
    imageInfo.extent.height = height;
    imageInfo.mipLevels     = mipLevels;
    imageInfo.format        = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.flags         = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageInfo.usage         = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT;
    
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    imageViewInfo.format   = VK_FORMAT_R8G8B8A8_SRGB;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.levelCount = mipLevels;
    imageViewInfo.subresourceRange.layerCount = 6;
    
    {
        m_rawCubemap    = data;
        m_imageInfo     = imageInfo;
        m_imageViewInfo = imageViewInfo;
    }
    
}

void Image::setupForSwapchain(VkImage image, VkFormat imageFormat) {
    LOG("Image::setupForSwapchain");
    m_image = image;
    VkImageCreateInfo     imageInfo      = m_imageInfo;
    VkImageViewCreateInfo imageViewInfo  = m_imageViewInfo;
    
    imageInfo    .format = imageFormat;
    imageViewInfo.format = imageFormat;
    imageViewInfo.image  = image;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    
    {
        m_imageInfo     = imageInfo;
        m_imageViewInfo = imageViewInfo;
    }
}

void Image::create() {
    createImage();
    allocateImageMemory();
    createImageView();
}

void Image::createForTexture() {
    createImage();
    allocateImageMemory();
    createImageView();
    createSampler();
}

void Image::createForCubemap() {
    createImage();
    allocateImageMemory();
    createImageView();
    createSampler();
}

void Image::createForSwapchain() {
    createImageView();
}

void Image::createImage() {
    LOG("createImage");
    VkResult result = vkCreateImage(m_device, &m_imageInfo, nullptr, &m_image);
    CHECK_VKRESULT(result, "failed to create image!");
}

void Image::createImageView() {
    LOG("createImageView");
    m_imageViewInfo.image = m_image;
    VkResult result = vkCreateImageView(m_device, &m_imageViewInfo, nullptr, &m_imageView);
    CHECK_VKRESULT(result, "failed to create image views!");
}

void Image::allocateImageMemory() {
    LOG("allocateImageMemory");
    VkDevice         device         = m_device;
    VkPhysicalDevice physicalDevice = m_physicalDevice;
    VkImage          image          = m_image;
    
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, image, &memoryRequirements);
    
    uint32_t memoryTypeIndex;
    memoryTypeIndex = FindMemoryTypeIndex(physicalDevice,
                                          memoryRequirements.memoryTypeBits,
                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memoryRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    VkDeviceMemory imageMemory;
    VkResult       result = vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory);
    CHECK_VKRESULT(result, "failed to allocate image memory!");
    
    vkBindImageMemory(device, image, imageMemory, 0);
    
    { m_imageMemory = imageMemory; }
}

void Image::createSampler() {
    LOG("createSampler");
    VkDevice device    = m_device;
    float    mipLevels = m_imageInfo.mipLevels;
    
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter    = VK_FILTER_LINEAR;
    samplerInfo.minFilter    = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy    = 16.0f;
    samplerInfo.borderColor      = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.compareEnable    = VK_FALSE;
    samplerInfo.compareOp        = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias       = 0.0f;
    samplerInfo.minLod           = 0.0f;
    samplerInfo.maxLod           = mipLevels;
    
    VkSampler sampler;
    VkResult  result = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
    CHECK_VKRESULT(result, "failed to create texture sampler!");
    
    { m_sampler = sampler; }
}

void Image::copyCubemapToImage() {
    LOG("Image::copyCubemapToImage");
    std::vector<unsigned char*> rawData = m_rawCubemap;
    
    VkDeviceSize imageSize = getImageSize();
    uint32_t     layerSize = imageSize / 6.0;
    
    Buffer *tempBuffer = new Buffer();
    tempBuffer->setup(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    tempBuffer->create();
    for (int i = 0; i < 6; ++i) {
        tempBuffer->fillBuffer(rawData[i], layerSize, layerSize * i);
    }
    
    cmdTransitionToTransferDest();
    cmdCopyBufferToImage(tempBuffer->m_buffer);
    cmdGenerateMipmaps();
    
    tempBuffer->cleanup();
}

void Image::copyRawDataToImage() {
    LOG("Image::copyRawDataToImage");
    unsigned char* rawData = m_rawData;
    
    VkDeviceSize imageSize = getImageSize();
    
    Buffer *tempBuffer = new Buffer();
    tempBuffer->setup(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    tempBuffer->create();
    tempBuffer->fillBufferFull(rawData);
    
    cmdTransitionToTransferDest();
    cmdCopyBufferToImage(tempBuffer->m_buffer);
    cmdGenerateMipmaps();
    
    tempBuffer->cleanup();
}

void Image::cmdTransitionToTransferDest() {
    LOG("Image::cmdTransitionToTransferDest");
    VkImage               image         = m_image;
    VkImageCreateInfo     imageInfo     = m_imageInfo;
    VkImageViewCreateInfo imageViewInfo = m_imageViewInfo;
    
    System&           system    = System::instance();
    Commander*        commander = system.getCommander();
    
    VkCommandBuffer commandBuffer = commander->createCommandBuffer();
    commander->beginSingleTimeCommands(commandBuffer);
    
    VkImageMemoryBarrier barrier = GetDefaultImageMemoryBarrier();
    barrier.image         = image;
    barrier.oldLayout     = imageInfo.initialLayout;
    barrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.subresourceRange.levelCount = imageInfo.mipLevels;
    barrier.subresourceRange.layerCount = imageViewInfo.subresourceRange.layerCount;
    
    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
    
    commander->endSingleTimeCommands(commandBuffer);
}

void Image::cmdCopyBufferToImage(VkBuffer buffer) {
    LOG("Image::cmdCopyBufferToImage");
    VkImage               image         = m_image;
    VkImageCreateInfo     imageInfo     = m_imageInfo;
    VkImageViewCreateInfo imageViewInfo = m_imageViewInfo;
    
    System&           system    = System::instance();
    Commander*        commander = system.getCommander();
    
    VkCommandBuffer commandBuffer = commander->createCommandBuffer();
    commander->beginSingleTimeCommands(commandBuffer);
    
    VkBufferImageCopy region{};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;
    
    region.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel        = 0;
    region.imageSubresource.baseArrayLayer  = 0;
    region.imageSubresource.layerCount      = imageViewInfo.subresourceRange.layerCount;
    
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {imageInfo.extent.width, imageInfo.extent.height, 1};
    
    vkCmdCopyBufferToImage(commandBuffer,
                           buffer,
                           image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &region);
    
    commander->endSingleTimeCommands(commandBuffer);
}

void Image::cmdGenerateMipmaps() {
    LOG("Image::cmdGenerateMipmaps");
    VkPhysicalDevice      physicalDevice = m_physicalDevice;
    VkImage               image          = m_image;
    VkImageCreateInfo     imageInfo      = m_imageInfo;
    VkImageViewCreateInfo imageViewInfo  = m_imageViewInfo;
    
    System&           system    = System::instance();
    Commander*        commander = system.getCommander();
    
    uint32_t layerCount = imageViewInfo.subresourceRange.layerCount;
    
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, imageInfo.format, &formatProperties);
    
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }
    
    VkCommandBuffer commandBuffer = commander->createCommandBuffer();
    commander->beginSingleTimeCommands(commandBuffer);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = layerCount;

    int32_t mipWidth  = imageInfo.extent.width;
    int32_t mipHeight = imageInfo.extent.height;

    for (uint32_t i = 1; i < imageInfo.mipLevels; i++) {
        int32_t halfMipWidth  = ceil(mipWidth /2);
        int32_t halfMipHeight = ceil(mipHeight/2);
        
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
        
        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel       = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount     = layerCount;
        
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { halfMipWidth, halfMipHeight, 1 };
        blit.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel       = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount     = layerCount;
        
        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);
        
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);
        
        mipWidth  = halfMipWidth;
        mipHeight = halfMipHeight;
    }
    
    barrier.subresourceRange.baseMipLevel = imageInfo.mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
    
    commander->endSingleTimeCommands(commandBuffer);

}

VkImage         Image::getImage      () { return m_image;       }
VkImageView     Image::getImageView  () { return m_imageView;   }
VkDeviceMemory  Image::getImageMemory() { return m_imageMemory; }
VkSampler       Image::getSampler    () { return m_sampler;     }
unsigned int    Image::getChannelSize() { return GetChannelSize(m_imageInfo.format); }
VkDeviceSize    Image::getImageSize  () { return m_imageInfo.extent.width * m_imageInfo.extent.height * getChannelSize() * m_imageInfo.arrayLayers; }
VkDescriptorImageInfo Image::getImageInfo() {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView   = m_imageView;
    imageInfo.sampler     = m_sampler;
    return imageInfo;
}


// Private ==================================================


unsigned int Image::GetChannelSize(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8G8B8_SRGB  : return 3; break;
        case VK_FORMAT_R8G8B8A8_SRGB: return 4; break;
        default: return 0; break;
    }
}

VkFormat Image::ChooseDepthFormat(VkPhysicalDevice physicalDevice) {
    const std::vector<VkFormat>& candidates = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        
        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        }
    }
    
    throw std::runtime_error("failed to find depth format!");
}

VkImageCreateInfo Image::GetDefaultImageCreateInfo() {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.arrayLayers   = 1;
    imageInfo.extent.depth  = 1;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    return imageInfo;
}

VkImageViewCreateInfo Image::GetDefaultImageViewCreateInfo() {
    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.subresourceRange.layerCount     = 1;
    imageViewInfo.subresourceRange.baseMipLevel   = 0;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    return imageViewInfo;
}

VkImageMemoryBarrier Image::GetDefaultImageMemoryBarrier() {
    VkImageMemoryBarrier barrier{};
    barrier.sType     = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;
    return barrier;
}
