//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"

class Descriptor {
    
public:
    Descriptor();
    ~Descriptor();
    
    void cleanup();
    
    void setupLayout(uint layoutId, uint count = 1);
    void createLayout(uint layoutId);
    void addLayoutBindings(uint layoutId, uint binding, VkDescriptorType type, VkShaderStageFlags flags);
    
    void create();
    
    void allocate(uint setId);
    void allocateAll();
    
    void setupPointerBuffer(uint layoutId, uint setIdx, uint binding, VkDescriptorBufferInfo bufferInfo);
    void setupPointerImage(uint layoutId, uint setIdx, uint binding, VkDescriptorImageInfo imageInfo);
    void update(uint id);
    
    VkDescriptorSetLayout getDescriptorLayout(uint id);
    std::vector<VkDescriptorSet> getDescriptorSets(uint id);
    
private:
#define VkDescriptorPoolSizeMap std::map<VkDescriptorType, VkDescriptorPoolSize>
#define VkDescriptorSetDataMap std::map<uint, DescriptorSetData>
    
    struct DescriptorSetData {
        uint id;
        uint count = 1;
        VkDescriptorSetLayout layout  = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> descriptorSets;
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        std::vector<VkWriteDescriptorSet> writeSets;
    };
    
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    
    VkDescriptorPool m_pool           = VK_NULL_HANDLE;
    
    VkDescriptorPoolSizeMap m_poolSizesMap;
    VkDescriptorSetDataMap  m_dataMap;
    
    void allocateData(DescriptorSetData* data);
    
    uint getTotalDecriptorSets();
    std::vector<VkDescriptorPoolSize> getPoolSizes();
    
    int findWriteSetIdx(uint id, uint binding);
};
