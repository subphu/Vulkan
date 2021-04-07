//  Copyright © 2021 Subph. All rights reserved.
//

#include "pipeline_compute.h"

#include "../system.h"

PipelineCompute::~PipelineCompute() {}
PipelineCompute::PipelineCompute() {
    System &system   = System::instance();
    m_device         = system.m_renderer->m_device;
    m_physicalDevice = system.m_renderer->m_physicalDevice;
}

void PipelineCompute::cleanup() {
    m_shader->cleanup();
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
}

void PipelineCompute::setupLayoutBindings(uint32_t bindingsCount) {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
    for(uint32_t i = 0; i < bindingsCount; i++){
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding         = i;
        layoutBinding.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        layoutBinding.descriptorCount = 1;
        layoutBinding.stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;
        layoutBindings.push_back(layoutBinding);
    }
    { m_layoutBindings = layoutBindings; }
}

void PipelineCompute::setupConstant() {
    VkPushConstantRange constantRange{};
    constantRange.size = sizeof(constantRange);
    constantRange.offset = 0;
    constantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    { m_constantRange = constantRange; }
}

void PipelineCompute::createDescriptorSetLayout() {
    LOG("createDescriptorSetLayout");
    VkDevice device = m_device;
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = m_layoutBindings;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = UINT32(layoutBindings.size());
    layoutInfo.pBindings    = layoutBindings.data();
    
    VkDescriptorSetLayout descriptorSetLayout;
    VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);
    CHECK_VKRESULT(result, "failed to create descriptor set layout!");

    { m_descriptorSetLayout = descriptorSetLayout; }
}

void PipelineCompute::createPipelineLayout() {
    LOG("createPipelineLayout");
    VkDevice device = m_device;
    VkPushConstantRange constantRange = m_constantRange;
    VkDescriptorSetLayout descriptorSetLayout = m_descriptorSetLayout;
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &constantRange;
    
    VkPipelineLayout pipelineLayout;
    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    CHECK_VKRESULT(result, "failed to create pipeline layout!");
    
    { m_pipelineLayout = pipelineLayout; }
}

void PipelineCompute::create() {
    LOG("createComputePipelines");
    VkDevice device = m_device;
    Shader*  shader = m_shader;
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    
    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage  = shader->getShaderStageInfo();
    pipelineInfo.layout = pipelineLayout;
    
    VkPipeline pipeline;
    VkResult result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
    CHECK_VKRESULT(result, "failed to create compute pipeline!");
    
    { m_pipeline = pipeline; }
}

void PipelineCompute::setShader(Shader* shader) { m_shader = shader; }
