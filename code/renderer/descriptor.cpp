//  Copyright © 2021 Subph. All rights reserved.
//

#include "descriptor.h"

#include "../system.h"

Descriptor::~Descriptor() {}
Descriptor::Descriptor() {
    Renderer* renderer = System::Renderer();
    m_device           = renderer->getDevice();
    m_physicalDevice   = renderer->getPhysicalDevice();
}

void Descriptor::cleanup() {
    LOG("Descriptor::cleanup");
    VkDescriptorSetDataMap::iterator it;
    for (it = m_dataMap.begin(); it != m_dataMap.end(); ++it ) {
        vkDestroyDescriptorSetLayout(m_device, it->second.layout, nullptr);
    }
    vkDestroyDescriptorPool(m_device, m_pool, nullptr);
}

void Descriptor::setupLayout(uint id, uint count) {
    m_dataMap[id] = DescriptorSetData{ .id = id, .count = count };
}

void Descriptor::addLayoutBindings(uint id, uint binding, VkDescriptorType type, VkShaderStageFlags flags) {
    if (!m_dataMap.count(id)) setupLayout(id);
    if (!m_poolSizesMap.count(type)) m_poolSizesMap[type] = VkDescriptorPoolSize{ type, 0 };
    
    uint count = 1;
    
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding         = binding;
    layoutBinding.descriptorCount = count;
    layoutBinding.descriptorType  = type;
    layoutBinding.stageFlags      = flags;
    layoutBinding.pImmutableSamplers = nullptr;
    
    VkWriteDescriptorSet writeSet{};
    writeSet.sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeSet.dstBinding      = binding;
    writeSet.descriptorCount = count;
    writeSet.descriptorType  = type;
    writeSet.dstArrayElement = 0;
    
    {
        m_dataMap[id].layoutBindings.push_back(layoutBinding);
        m_dataMap[id].writeSets.push_back(writeSet);
        m_poolSizesMap[type].descriptorCount += m_dataMap[id].count;
    }
}

void Descriptor::createLayout(uint id) {
    LOG("Descriptor::createLayout");
    VkDevice          device = m_device;
    DescriptorSetData data   = m_dataMap[id];
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = UINT32(data.layoutBindings.size());
    layoutInfo.pBindings    = data.layoutBindings.data();
    
    VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &data.layout);
    CHECK_VKRESULT(result, "failed to create descriptor set layout!");
    
    { m_dataMap[id] = data; }
}

void Descriptor::createPool() {
    LOG("Descriptor::createPool");
    VkDevice device = m_device;
    std::vector<VkDescriptorPoolSize> poolSizes = getPoolSizes();
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets       = getTotalDecriptorSets();
    poolInfo.poolSizeCount = UINT32(poolSizes.size());
    poolInfo.pPoolSizes    = poolSizes.data();

    VkDescriptorPool pool;
    VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);
    CHECK_VKRESULT(result, "failed to create descriptor pool!");
    
    { m_pool = pool; }
}

void Descriptor::allocate(uint id) {
    allocateData(&m_dataMap[id]);
}

void Descriptor::allocateAll() {
    VkDescriptorSetDataMap map = m_dataMap;
    VkDescriptorSetDataMap::iterator it;
    for (it = map.begin(); it != map.end(); ++it )
        allocateData(&it->second);
}

void Descriptor::setupPointerBuffer(uint id, uint setIdx, uint binding, VkDescriptorBufferInfo* pBufferInfo) {
    int idx = findWriteSetIdx(id, binding);
    m_dataMap[id].writeSets[idx].dstSet = m_dataMap[id].descriptorSets[setIdx];
    m_dataMap[id].writeSets[idx].pBufferInfo = pBufferInfo;
}

void Descriptor::setupPointerImage(uint id, uint setIdx, uint binding, VkDescriptorImageInfo* pImageInfo) {
    int idx = findWriteSetIdx(id, binding);
    m_dataMap[id].writeSets[idx].dstSet = m_dataMap[id].descriptorSets[setIdx];
    m_dataMap[id].writeSets[idx].pImageInfo = pImageInfo;
}

void Descriptor::update(uint id) {
    LOG("Descriptor::update");
    std::vector<VkWriteDescriptorSet> writeSets = m_dataMap[id].writeSets;
    vkUpdateDescriptorSets(m_device, UINT32(writeSets.size()), writeSets.data(), 0, nullptr);
}

VkDescriptorSetLayout Descriptor::getDescriptorLayout(uint id) {
    return m_dataMap[id].layout;
}

std::vector<VkDescriptorSet> Descriptor::getDescriptorSets(uint id) {
    return m_dataMap[id].descriptorSets;
}


// Private ==================================================


void Descriptor::allocateData(DescriptorSetData* data) {
    LOG("Descriptor::allocateData");
    VkDevice          device = m_device;
    VkDescriptorPool  pool   = m_pool;
    
    std::vector<VkDescriptorSetLayout> layouts(data->count, data->layout);
    
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = pool;
    allocInfo.descriptorSetCount = data->count;
    allocInfo.pSetLayouts        = layouts.data();
    
    data->descriptorSets.resize(data->count);
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, data->descriptorSets.data());
    CHECK_VKRESULT(result, "failed to allocate descriptor set!");
}

uint Descriptor::getTotalDecriptorSets() {
    uint total = 0;
    VkDescriptorSetDataMap map = m_dataMap;
    VkDescriptorSetDataMap::iterator it;
    for (it = map.begin(); it != map.end(); ++it )
        total += (it->second).count;
    return total;
}

std::vector<VkDescriptorPoolSize> Descriptor::getPoolSizes() {
    std::vector<VkDescriptorPoolSize> vec;
    VkDescriptorPoolSizeMap map = m_poolSizesMap;
    VkDescriptorPoolSizeMap::iterator it;
    for (it = map.begin(); it != map.end(); ++it )
        vec.push_back(it->second);
    return vec;
}

int Descriptor::findWriteSetIdx(uint id, uint binding) {
    std::vector<VkWriteDescriptorSet> writeSets = m_dataMap[id].writeSets;
    for (uint i = 0; i < writeSets.size(); i++)
        if (writeSets[i].dstBinding == binding) return i;
    return -1;
}
