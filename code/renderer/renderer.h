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
    std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
    std::vector<VkPresentModeKHR> m_surfaceModes;
    uint32_t m_graphicFamilyIndex = 0;
    uint32_t m_presentFamilyIndex = 0;
    void pickPhysicalDevice();
    
    VkDevice m_device = VK_NULL_HANDLE;
    void createLogicalDevice();
    
    VkQueue m_graphicQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    void createDeviceQueue();
    
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    void createCommandPool();
    
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    
    VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
    VkDeviceMemory allocateBufferMemory(VkBuffer& buffer, VkDeviceSize size, uint32_t memoryTypeIndex);
    VkMemoryRequirements getBufferMemoryRequirements(VkBuffer& buffer);
    VkMemoryRequirements getImageMemoryRequirements(VkImage& image);
    uint32_t findMemoryTypeIdx(uint32_t typeFilter, VkMemoryPropertyFlags flags);
private:
    
    
};
