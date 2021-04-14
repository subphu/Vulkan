//  Copyright © 2020 Subph. All rights reserved.
//

#include <array>
#include <set>

#include "renderer.h"
#include "../helper.h"

Renderer::Renderer() { }
Renderer::~Renderer() { }

void Renderer::cleanUp() {
    LOG("Renderer::cleanUp");
    m_commander->cleanup();
    
    vkDestroyDevice(m_device, nullptr);
    DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

void Renderer::setupValidation(bool isEnable) {
    if (!isEnable) return;
    USE_FUNC(DebugCallback);

    std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
    debugInfo.sType  = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugInfo.pfnUserCallback = DebugCallback;
    
    bool result = CheckLayerSupport(validationLayers);
    CHECK_BOOL(result, "validation layers requested, but not available!");
    
    {
        m_debugInfo        = debugInfo;
        m_validationLayers = validationLayers;
    }
}

void Renderer::createInstance(std::vector<const char*> extensions) {
    LOG("createInstance");
    VkDebugUtilsMessengerCreateInfoEXT debugInfo = m_debugInfo;
    std::vector<const char*> validationLayers    = m_validationLayers;
    
    VkApplicationInfo appInfo{};
    appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName    = "Application";
    appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName         = "No Engine";
    appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion          = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo        = &appInfo;
    instanceInfo.enabledExtensionCount   = UINT32(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();
    instanceInfo.enabledLayerCount       = UINT32(validationLayers.size());
    instanceInfo.ppEnabledLayerNames     = validationLayers.data();
    instanceInfo.pNext                   = &debugInfo;
 
    VkInstance     instance;
    VkResult       result = vkCreateInstance(&instanceInfo, nullptr, &instance);
    CHECK_VKRESULT(result, "failed to create vulkan instance!");
    
    { m_instance = instance; }
}

void Renderer::createDebugMessenger() {
    LOG("createDebugMessenger");
    VkResult result = CreateDebugUtilsMessengerEXT(m_instance, &m_debugInfo, nullptr, &m_debugMessenger);
    CHECK_VKRESULT(result, "failed to set up debug messenger!");
}

void Renderer::setupDeviceExtensions() {
    m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
}

void Renderer::pickPhysicalDevice() {
    LOG("pickPhysicalDevice");
    VkSurfaceKHR surface  = m_surface;
    VkInstance   instance = m_instance;
    
    std::vector<const char*>      deviceExtensions = m_deviceExtensions;
    std::vector<VkPhysicalDevice> physicalDevices  = GetPhysicalDevices(instance);
    CHECK_ZERO(physicalDevices.size(), "failed to find GPUs with Vulkan support!");
    
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR>   presentModes;
    int graphicQueueIndex = 0;
    int presentQueueIndex = 0;
    
    for (const auto& tempDevice : physicalDevices) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(tempDevice, &properties);
        LOG(properties.deviceName);

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(tempDevice, &supportedFeatures);
        
        physicalDevice    = tempDevice;
        surfaceFormats    = GetSurfaceFormatKHR  (tempDevice, surface);
        presentModes      = GetSurfaceModeKHR    (tempDevice, surface);
        presentQueueIndex = FindPresentQueueIndex(tempDevice, surface);
        graphicQueueIndex = FindGraphicQueueIndex(tempDevice);
        
        bool swapchainAdequate  = !surfaceFormats.empty() && !presentModes.empty();
        bool hasFamilyIndex     = graphicQueueIndex > -1 && presentQueueIndex > -1;
        bool extensionSupported = CheckDeviceExtensionSupport(tempDevice, deviceExtensions);
        
        if (swapchainAdequate && hasFamilyIndex && extensionSupported &&
            supportedFeatures.samplerAnisotropy) break;
    }
    {
        m_physicalDevice    = physicalDevice;
        m_surfaceFormats    = surfaceFormats;
        m_presentModes      = presentModes;
        m_graphicQueueIndex = graphicQueueIndex;
        m_presentQueueIndex = presentQueueIndex;
    }
}

void Renderer::createLogicalDevice() {
    LOG("createLogicalDevice");
    
    VkPhysicalDevice         physicalDevice     = m_physicalDevice;
    std::vector<const char*> deviceExtensions   = m_deviceExtensions;
    std::vector<const char*> validationLayers   = m_validationLayers;
    std::set<uint32_t>       queueFamilyIndices = {m_graphicQueueIndex, m_presentQueueIndex};
    
    float queuePriority = 1.f;
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    for (uint32_t familyIndex : queueFamilyIndices) {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType             = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex  = familyIndex;
        queueInfo.queueCount        = 1;
        queueInfo.pQueuePriorities  = &queuePriority;
        queueInfos.push_back(queueInfo);
    }
    
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    
    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount     = UINT32(queueInfos.size());
    deviceInfo.pQueueCreateInfos        = queueInfos.data();
    deviceInfo.pEnabledFeatures         = &deviceFeatures;
    deviceInfo.enabledExtensionCount    = UINT32(deviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames  = deviceExtensions.data();
    deviceInfo.enabledLayerCount        = UINT32(validationLayers.size());
    deviceInfo.ppEnabledLayerNames      = validationLayers.data();
    
    VkDevice device = VK_NULL_HANDLE;
    VkResult result = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device);
    CHECK_VKRESULT(result, "failed to create logical device");

    { m_device = device; }
}

void Renderer::createDeviceQueue() {
    vkGetDeviceQueue(m_device, m_graphicQueueIndex, 0, &m_graphicQueue);
    vkGetDeviceQueue(m_device, m_presentQueueIndex, 0, &m_presentQueue);
}

Commander* Renderer::getCommander() { return m_commander; }
void Renderer::createCommander() {
    m_commander = new Commander();
    m_commander->setupPool(m_graphicQueue, m_graphicQueueIndex);
    m_commander->create();
}

VkSurfaceFormatKHR Renderer::getSwapchainSurfaceFormat() {
    const std::vector<VkSurfaceFormatKHR>& availableFormats = m_surfaceFormats;
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format     == VK_FORMAT_B8G8R8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR Renderer::getSwapchainPresentMode() {
    const std::vector<VkPresentModeKHR>& availablePresentModes = m_presentModes;
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

uint32_t Renderer::getGraphicQueueIndex() { return m_graphicQueueIndex; }
uint32_t Renderer::getPresentQueueIndex() { return m_presentQueueIndex; }

void Renderer::setSurface(VkSurfaceKHR surface) { m_surface = surface; }

VkSurfaceKHR     Renderer::getSurface()        { return m_surface; }
VkPhysicalDevice Renderer::getPhysicalDevice() { return m_physicalDevice; }
VkDevice         Renderer::getDevice()         { return m_device; }


// Private ==================================================


std::vector<VkPhysicalDevice> Renderer::GetPhysicalDevices(VkInstance instance) {
    uint32_t count;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());
    return devices;
}

std::vector<VkSurfaceFormatKHR> Renderer::GetSurfaceFormatKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    uint32_t count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, formats.data());
    return formats;
}

std::vector<VkPresentModeKHR> Renderer::GetSurfaceModeKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    uint32_t count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr);
    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, modes.data());
    return modes;
}

std::vector<VkQueueFamilyProperties> Renderer::GetQueueFamilyProperties(VkPhysicalDevice physicalDevice) {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueFamilies.data());
    return queueFamilies;
}

int Renderer::FindGraphicQueueIndex(VkPhysicalDevice physicalDevice) {
    std::vector<VkQueueFamilyProperties> queueFamilies = GetQueueFamilyProperties(physicalDevice);
    for (int i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) return i;
    }
    return -1;
}

int Renderer::FindPresentQueueIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    std::vector<VkQueueFamilyProperties> queueFamilies = GetQueueFamilyProperties(physicalDevice);
    for (int i = 0; i < queueFamilies.size(); i++) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport) return i;
    }
    return -1;
}

bool Renderer::CheckLayerSupport(std::vector<const char*> layers) {
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> availableLayers(count);
    vkEnumerateInstanceLayerProperties(&count, availableLayers.data());
    
    std::set<std::string> requiredLayers(layers.begin(), layers.end());
    for (const auto& layerProperties : availableLayers) {
        requiredLayers.erase(layerProperties.layerName);
    }
    return requiredLayers.empty();
}

bool Renderer::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice, std::vector<const char*> extensions) {
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, availableExtensions.data());

    std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    return requiredExtensions.empty();
}

VkResult Renderer::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Renderer::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    std::cerr << " \nValidation layer: \n" << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
