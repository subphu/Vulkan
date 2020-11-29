//  Copyright © 2020 Subph. All rights reserved.
//

#pragma once
#include "common.h"

class System {
    
public:
    
    VkDevice device;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    
    VkQueue graphicsQueue;
    VkCommandPool commandPool;
    
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
    VkDeviceMemory createBufferMemory(VkBuffer& buffer, VkMemoryPropertyFlags properties);
    
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    
    static System& instance() {
        static System instance; // Guaranteed to be destroyed. Instantiated on first use.
        return instance;
    }
    
private:
    System();
    ~System();

    // C++ 03
    // ========
    // Don't forget to declare these two. You want to make sure they
    // are unacceptable otherwise you may accidentally get copies of
    // your singleton appearing.
    System(System const&);         // Don't Implement
    void operator=(System const&); // Don't implement
    
};


