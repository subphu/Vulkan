//  Copyright © 2020 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "commander.h"
#include "swapchain.h"
#include "../resources/buffer.h"
#include "../resources/image.h"

class Swapchain;
class Buffer;
class Image;

class Renderer {
    
public:
    Renderer();
    ~Renderer();
    
    void cleanUp();
    
    std::vector<const char*> m_validationLayers = {};
    VkDebugUtilsMessengerCreateInfoEXT m_debugInfo = {};
    void setupValidation(bool isEnable);
    
    VkInstance m_instance = VK_NULL_HANDLE;
    void createInstance(std::vector<const char*> extensions);
    
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    void createDebugMessenger();
    
    std::vector<const char*> m_deviceExtensions = {};
    void setupDeviceExtensions();
    
    std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
    std::vector<VkPresentModeKHR  > m_presentModes;
    VkSurfaceFormatKHR getSwapchainSurfaceFormat();
    VkPresentModeKHR   getSwapchainPresentMode();

    uint32_t m_graphicQueueIndex = 0;
    uint32_t m_presentQueueIndex = 0;
    uint32_t getGraphicQueueIndex();
    uint32_t getPresentQueueIndex(VkSurfaceKHR surface);

    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDevice getPhysicalDevice();
    void pickPhysicalDevice(VkSurfaceKHR surface);
    
    VkDevice m_device = VK_NULL_HANDLE;
    VkDevice getDevice();
    void createLogicalDevice();
    
    VkQueue m_graphicQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    void createDeviceQueue();
    
    Commander* m_commander = nullptr;
    Commander* getCommander();
    void createCommander();

private:
    
    static std::vector<VkPhysicalDevice>        GetPhysicalDevices(VkInstance instance);
    static std::vector<VkSurfaceFormatKHR>      GetSurfaceFormatKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    static std::vector<VkPresentModeKHR>        GetSurfaceModeKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    static std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties(VkPhysicalDevice physicalDevice);
    
    
    static int FindGraphicQueueIndex(VkPhysicalDevice physicalDevice);
    static int FindPresentQueueIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    
    static bool CheckLayerSupport(std::vector<const char*> layers);
    static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> extensions);

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger);
    static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                       VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 void* pUserData);
};
