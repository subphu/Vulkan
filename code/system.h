//  Copyright © 2020 Subph. All rights reserved.
//

#pragma once
#include "common.h"


#include "window/window.h"

class System {
    
public:
    
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    
    uint32_t graphicsFamilyIndex;
    uint32_t presentFamilyIndex;
    
    VkDevice device;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkCommandPool commandPool;
    
    void setValidation(bool isEnable);
    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();
    
    VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
    VkDeviceMemory createBufferMemory(VkBuffer& buffer, VkMemoryPropertyFlags properties);
    
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    
    void cleanup();
    static System& singleton() {
        static System instance; // Guaranteed to be destroyed. Instantiated on first use.
        return instance;
    }
    
private:
    System();
    ~System();

    std::vector<const char*> validationLayers = {};
    std::vector<const char*> deviceExtensions = {};
    VkDebugUtilsMessengerCreateInfoEXT debugInfo = {};
    
    
    // C++ 03
    // ========
    // Don't forget to declare these two. You want to make sure they
    // are unacceptable otherwise you may accidentally get copies of
    // your singleton appearing.
    System(System const&);         // Don't Implement
    void operator=(System const&); // Don't implement
    

};

