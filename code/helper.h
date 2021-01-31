//  Copyright © 2020 Subph. All rights reserved.
//

#pragma once

#include <fstream>
#include <vector>
#include <set>

#include "common.h"

static std::vector<VkPhysicalDevice> GetPhysicalDevices(VkInstance instance) {
    uint32_t count;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());
    return devices;
}

static std::vector<VkSurfaceFormatKHR> GetSurfaceFormatKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, formats.data());
    return formats;
}

static std::vector<VkPresentModeKHR> GetSurfaceModeKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr);
    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, modes.data());
    return modes;
}

static std::vector<VkLayerProperties> GetLayerProperties() {
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> layers(count);
    vkEnumerateInstanceLayerProperties(&count, layers.data());
    return layers;
}

static std::vector<VkExtensionProperties> GetExtensionProperties(VkPhysicalDevice physicalDevice) {
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> extensions(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensions.data());
    return extensions;
}

static std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties(VkPhysicalDevice physicalDevice) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueFamilies.data());
    return queueFamilies;
}


static std::vector<VkImage> GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapChain) {
    uint32_t count = 0;
    vkGetSwapchainImagesKHR(device, swapChain, &count, nullptr);
    std::vector<VkImage> swapChainImages(count);
    vkGetSwapchainImagesKHR(device, swapChain, &count, swapChainImages.data());
    return swapChainImages;
}

static bool CheckLayerSupport(std::vector<const char*> layers) {
    std::vector<VkLayerProperties> availableLayers = GetLayerProperties();
    std::set<std::string> requiredLayers(layers.begin(), layers.end());
    for (const auto& layerProperties : availableLayers) {
        requiredLayers.erase(layerProperties.layerName);
    }
    return requiredLayers.empty();
}

static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> extensions) {
    std::vector<VkExtensionProperties> availableExtensions = GetExtensionProperties(device);
    std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

static int FindGraphicFamilyIndex(VkPhysicalDevice physicalDevice) {
    std::vector<VkQueueFamilyProperties> queueFamilies = GetQueueFamilyProperties(physicalDevice);
    for (int i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) return i;
    }
    return -1;
}

static int FindPresentFamilyIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    std::vector<VkQueueFamilyProperties> queueFamilies = GetQueueFamilyProperties(physicalDevice);
    for (int i = 0; i < queueFamilies.size(); i++) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport) return i;
    }
    return -1;
}

static uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    throw std::runtime_error("failed to find suitable memory type!");
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    std::cerr << " \nValidation layer: \n" << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}


static bool IsDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<const char*> deviceExtensions) {
    std::vector<VkSurfaceFormatKHR> formats = GetSurfaceFormatKHR(physicalDevice, surface);
    std::vector<VkPresentModeKHR> modes = GetSurfaceModeKHR(physicalDevice, surface);
    
    bool swapChainAdequate = !formats.empty() && !modes.empty();
    bool hasGraphicFamilyIndex = FindGraphicFamilyIndex(physicalDevice) > -1;
    bool hasPresentFamilyIndex = FindPresentFamilyIndex(physicalDevice, surface) > -1;
    bool extensionSupported = CheckDeviceExtensionSupport(physicalDevice, deviceExtensions);
    
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
    
    return hasGraphicFamilyIndex && hasPresentFamilyIndex &&
    extensionSupported && swapChainAdequate &&
    supportedFeatures.samplerAnisotropy;
}

static VkFormat ChooseDepthFormat(VkPhysicalDevice physicalDevice) {
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

static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Size<int> size) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    
    VkExtent2D actualExtent = {static_cast<uint32_t>(size.width), static_cast<uint32_t> (size.height)};
    
    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
    return actualExtent;
}

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("failed to open file!");

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

static uint32_t MaxMapLevel(int width, int height) {
    return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
}
