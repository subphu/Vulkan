//  Copyright © 2020 Subph. All rights reserved.
//

#include "renderer.h"

Renderer::Renderer() { }
Renderer::~Renderer() { cleanUp(); }

void Renderer::cleanUp() {
    DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

void Renderer::setupValidation(bool isEnable) {
    {
        if (!isEnable) return;
        USE_FUNC(DebugCallback);
    }
    m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
    m_debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    m_debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    m_debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    m_debugInfo.pfnUserCallback = DebugCallback;
    {
        CHECK_BOOL(checkLayerSupport(m_validationLayers), "validation layers requested, but not available!");
        CHECK_ZERO(m_validationLayers.size(), "validation layers empty!");
        CHECK_ZERO(m_debugInfo.sType, "debug info empty!");
    }
}

void Renderer::createInstance(std::vector<const char*> extensions) {
    {
        CHECK_ZERO(extensions.size(), "extension empty!");
        USE_VAR(m_debugInfo);
        USE_VAR(m_validationLayers);
    }
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
    instanceInfo.ppEnabledLayerNames = m_validationLayers.data();
    instanceInfo.pNext = &m_debugInfo;
 
    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_instance);
    {
        CHECK_VKRESULT(result, "failed to create vulkan instance!");
        CHECK_HANDLE(m_instance, "instance undefined!");
        std::cout << "vulkan instance created!\n";
    }
}

void Renderer::createDebugMessenger() {
    {
        USE_VAR(m_debugInfo);
        CHECK_HANDLE(m_instance, "instance undefined!");
    }
    VkResult result = CreateDebugUtilsMessengerEXT(m_instance, &m_debugInfo, nullptr, &m_debugMessenger);
    {
        CHECK_VKRESULT(result, "failed to set up debug messenger!");
        CHECK_HANDLE(m_debugMessenger, "debug messenger undefined!");
    }
}

void Renderer::setDeviceExtensions() {
    m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    CHECK_ZERO(m_deviceExtensions.size(), "device extensions empty!");
}

void Renderer::setSurface(VkSurfaceKHR* pSurface) { m_surface = *pSurface; }

void Renderer::pickPhysicalDevice() {
    {
        CHECK_HANDLE(m_instance, "instance undefined!");
        CHECK_HANDLE(m_surface, "surface undefined!");
        CHECK_ZERO(m_deviceExtensions.size(), "device extensions empty!");
    }
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    CHECK_BOOL(deviceCount, "failed to find GPUs with Vulkan support!");
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    
    for (const auto& device : devices) {
        if (isDeviceSuitable(device, m_surface, m_deviceExtensions)) {
            m_physicalDevice = device;
            break;
        }
    }
    {
        CHECK_HANDLE(m_physicalDevice, "failed to find a suitable GPU!");
    }
}

void Renderer::createLogicalDevice() {
    {
        CHECK_HANDLE(m_physicalDevice, "physical device undefined!");
        CHECK_HANDLE(m_surface, "surface undefined!");
        CHECK_ZERO(m_deviceExtensions.size(), "device extensions empty!");
        USE_VAR(m_validationLayers);
    }
    uint32_t graphicFamilyIndex = FindGraphicFamilyIndex(m_physicalDevice);
    uint32_t presentFamilyIndex = FindPresentFamilyIndex(m_physicalDevice, m_surface);
    std::set<uint32_t> uniqueQueueFamilies = {graphicFamilyIndex, presentFamilyIndex};
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    
    float queuePriority = 1.f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamily;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;
        queueInfos.push_back(queueInfo);
    }
    
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    
    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    deviceInfo.pQueueCreateInfos = queueInfos.data();
    deviceInfo.pEnabledFeatures = &deviceFeatures;
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames = m_deviceExtensions.data();
    deviceInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
    deviceInfo.ppEnabledLayerNames = m_validationLayers.data();
    
    VkResult result = vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_device);
    CHECK_VKRESULT(result, "failed to create logical device");
    
    vkGetDeviceQueue(m_device, graphicFamilyIndex, 0, &m_graphicQueue);
    vkGetDeviceQueue(m_device, presentFamilyIndex, 0, &m_presentQueue);
    {
        CHECK_HANDLE(m_device, "logical device undefined!");
        CHECK_HANDLE(m_graphicQueue, "graphic queue undefined!");
        CHECK_HANDLE(m_presentQueue, "present queue undefined!");
    }
}

VkBuffer Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage) {
    {
        CHECK_HANDLE(m_device, "logical device undefined!");
    }
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkBuffer buffer;
    VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer);
    {
        CHECK_VKRESULT(result, "failed to create buffer!");
        return buffer;
    }
}

VkDeviceMemory Renderer::allocateBufferMemory(VkBuffer& buffer, VkDeviceSize size, uint32_t memoryTypeIndex) {
    {
        CHECK_HANDLE(m_device, "logical device undefined!");
    }
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    VkDeviceMemory bufferMemory;
    VkResult result = vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory);
    {
        CHECK_VKRESULT(result, "failed to allocate buffer memory!");
        return bufferMemory;
    }
}

VkMemoryRequirements Renderer::getBufferMemoryRequirements(VkBuffer& buffer) {
    { CHECK_HANDLE(m_device, "logical device undefined!"); }
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memoryRequirements);
    { return memoryRequirements; }
}

VkMemoryRequirements Renderer::getImageMemoryRequirements(VkImage& image) {
    { CHECK_HANDLE(m_device, "logical device undefined!"); }
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(m_device, image, &memoryRequirements);
    { return memoryRequirements; }
}

uint32_t Renderer::findMemoryTypeIdx(uint32_t typeFilter, VkMemoryPropertyFlags flags) {
    { CHECK_HANDLE(m_physicalDevice, "physical device undefined!"); }
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &properties);
    
    for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
        VkMemoryPropertyFlags propertyFlags = properties.memoryTypes[i].propertyFlags;
        if ((typeFilter & (1 << i)) && (propertyFlags & flags) == flags)
            return i;
    }
    { throw std::runtime_error("failed to find suitable memory type!"); }
}

