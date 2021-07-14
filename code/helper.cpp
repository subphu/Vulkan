//  Copyright © 2021 Subph. All rights reserved.
//


#include <fstream>
#include <vector>
#include <set>

#include "libraries/stb_image/stb_image.h"

#include "helper.h"


std::vector<VkImage> GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain) {
    uint32_t count = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
    std::vector<VkImage> swapchainImages(count);
    vkGetSwapchainImagesKHR(device, swapchain, &count, swapchainImages.data());
    return swapchainImages;
}

uint32_t FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags flags) {
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &properties);
    
    for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) &&
           (properties.memoryTypes[i].propertyFlags & flags) == flags)
            return i;
    }
    
    throw std::runtime_error("failed to find suitable memory type!");
}

VkFormat ChooseDepthFormat(VkPhysicalDevice physicalDevice) {
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


VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Size<int> size) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    
    VkExtent2D actualExtent = {UINT32(size.width), UINT32 (size.height)};
    
    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
    return actualExtent;
}

std::vector<char> ReadBinaryFile(const std::string filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("failed to open file!");

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

unsigned char* LoadImage(const std::string filename, int* width, int* height, int* channels) {
    unsigned char *data = stbi_load(filename.c_str(), width, height, channels, STBI_rgb_alpha);
    if (data) return data;
    PRINTLN2("failed to load image ", filename);
    return nullptr;
}

float* LoadHDR(const std::string filename, int* width, int* height, int* channels) {
    float *data = stbi_loadf(filename.c_str(), width, height, channels, STBI_default);
    if (data) return data;
    PRINTLN2("failed to load hdr image ", filename);
    return nullptr;
}

uint32_t MaxMipLevel(int width, int height) {
    return UINT32(std::floor(std::log2(std::max(width, height)))) + 1;
}
