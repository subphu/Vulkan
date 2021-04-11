//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"

enum Set     { S0 = 0, S1 = 1, S2 = 2, S3 = 3, S4 = 4, S5 = 5, S6 = 6, S7 = 7 };
enum Layout  { L0 = 0, L1 = 1, L2 = 2, L3 = 3, L4 = 4, L5 = 5, L6 = 6, L7 = 7 };
enum Binding { B0 = 0, B1 = 1, B2 = 2, B3 = 3, B4 = 4, B5 = 5, B6 = 6, B7 = 7 };

class Descriptor {
    
public:
    Descriptor();
    ~Descriptor();
    
    void cleanup();
    
    void setupLayout(uint layoutId, uint count = 1);
    void createLayout(uint layoutId);
    void addLayoutBindings(uint layoutId, uint binding, VkDescriptorType type, VkShaderStageFlags flags);
    
    void createPool();
    
    void allocate(uint layoutId);
    void allocateAll();
    
    void setupPointerBuffer(uint layoutId, uint setIdx, uint binding, VkDescriptorBufferInfo* pBufferInfo);
    void setupPointerImage(uint layoutId, uint setIdx, uint binding, VkDescriptorImageInfo* pImageInfo);
    void update(uint layoutId);
    
    VkDescriptorSetLayout getDescriptorLayout(uint layoutId);
    std::vector<VkDescriptorSet> getDescriptorSets(uint layoutId);
    
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
