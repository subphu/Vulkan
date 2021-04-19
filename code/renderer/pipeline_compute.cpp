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
    LOG("PipelineCompute::cleanup");
    m_shader->cleanup();
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
}

void PipelineCompute::setupPushConstant(uint size) {
    VkPushConstantRange constantRange{};
    constantRange.size = size;
    constantRange.offset = 0;
    constantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    { m_constantRange = constantRange; }
}

void PipelineCompute::createPipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts) {
    LOG("PipelineCompute::createPipelineLayout");
    VkDevice device = m_device;
    VkPushConstantRange constantRange = m_constantRange;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = UINT32(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts    = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges    = &constantRange;

    VkPipelineLayout pipelineLayout;
    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    CHECK_VKRESULT(result, "failed to create pipeline layout!");

    { m_pipelineLayout = pipelineLayout; }
}

void PipelineCompute::create() {
    LOG("PipelineCompute::create");
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
