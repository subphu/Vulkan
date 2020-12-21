//  Copyright © 2020 Subph. All rights reserved.
//

#pragma once
#include "../common.h"

class Renderer {
    
public:
    Renderer();
    ~Renderer();
    
    void cleanUp();
    
    std::vector<const char*> m_validationLayers = {};
    VkDebugUtilsMessengerCreateInfoEXT m_debugInfo = {};
    void setupValidation(bool isEnable);
    
    std::vector<const char*> m_deviceExtensions = {};
    void setDeviceExtensions();
    
    VkInstance m_instance = VK_NULL_HANDLE;
    void createInstance(std::vector<const char*> extensions);
    
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    void createDebugMessenger();
    
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    void setSurface(VkSurfaceKHR* pSurface);
    
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    void pickPhysicalDevice();

    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    void createLogicalDevice();
    
    VkCommandPool m_commandPool;
    
    VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
    VkDeviceMemory allocateBufferMemory(VkBuffer& buffer, VkDeviceSize size, uint32_t memoryTypeIndex);
    VkMemoryRequirements getBufferMemoryRequirements(VkBuffer& buffer);
    VkMemoryRequirements getImageMemoryRequirements(VkImage& image);
    uint32_t findMemoryTypeIdx(uint32_t typeFilter, VkMemoryPropertyFlags flags);
private:
    
    
};

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    std::cerr << " \nValidation layer: \n" << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
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

static int FindGraphicFamilyIndex(VkPhysicalDevice physicalDevice) {
    std::vector<VkQueueFamilyProperties> queueFamilies = getQueueFamilyProperties(physicalDevice);
    for (int i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) return i;
    }
    return -1;
}

static int FindPresentFamilyIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    std::vector<VkQueueFamilyProperties> queueFamilies = getQueueFamilyProperties(physicalDevice);
    for (int i = 0; i < queueFamilies.size(); i++) {
        VkBool32 supported = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supported);
        if (supported) return i;
    }
    return -1;
}
