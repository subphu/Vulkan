//  Copyright © 2021 Subph. All rights reserved.
//

#include "compute_equirectangular.h"

#include "../system.h"

ComputeEquirectangular::~ComputeEquirectangular() {}
ComputeEquirectangular::ComputeEquirectangular() {
    LOG("ComputeEquirectangular::==============================");
    Renderer* renderer = System::Renderer();
    m_device           = renderer->getDevice();
    m_physicalDevice   = renderer->getPhysicalDevice();
}

void ComputeEquirectangular::cleanup() {
    LOG("ComputeEquirectangular::cleanup");
    m_pShader->cleanup();
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    m_pDescriptor->cleanup();
}

void ComputeEquirectangular::setup(Size<uint> size) {
    LOG("ComputeEquirectangular::setup");
    m_details.width = size.width;
    m_details.height = size.height;
    m_details.mapSize = 1024;
    m_pShader = new Shader(SHADER_PATH, VK_SHADER_STAGE_COMPUTE_BIT);

    createEquirectangular();
    createOutputBuffer();
    createDescriptor();
    createPipelineLayout();
    createPipeline();
}

void ComputeEquirectangular::dispatch() {
    LOG("ComputeEquirectangular::dispatch");
    Commander* commander = System::Commander();

    
    VkPipeline          pipeline       = m_pipeline;
    VkPipelineLayout    pipelineLayout = m_pipelineLayout;
    ComputeDetails      details        = m_details;
    VkDescriptorSet     outputDescSet  = m_pDescriptor->getDescriptorSets(L0)[0];
    VkDescriptorSet     equirecDescSet = m_pDescriptor->getDescriptorSets(L1)[0];

    VkCommandBuffer commandBuffer = commander->createCommandBuffer();
    commander->beginSingleTimeCommands(commandBuffer);
    {
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
                       0, sizeof(details), &details);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                pipelineLayout, 0, 1, &outputDescSet, 0, nullptr);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                pipelineLayout, 0, 1, &equirecDescSet, 0, nullptr);

    vkCmdDispatch(commandBuffer,
                  details.mapSize / WORKGROUP_SIZE,
                  details.mapSize / WORKGROUP_SIZE, 6);
    }
    commander->endSingleTimeCommands(commandBuffer);
}


// Private ==================================================

void ComputeEquirectangular::createCubemap() {
    LOG("ComputeEquirectangular::createCubemap");
    m_pCubemap = new Image();
    m_pCubemap->setupForCubemap({ m_details.width, m_details.height });
    m_pCubemap->m_imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    m_pCubemap->m_imageViewInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    m_pCubemap->createForCubemap();
}

void ComputeEquirectangular::createEquirectangular() {
    LOG("ComputeEquirectangular::createEquirectangular");
    m_pEquirectangular = new Image();
    m_pEquirectangular->setupForHDRTexture(TEXTURE_PATH);
    m_pEquirectangular->createForTexture();
    m_pEquirectangular->copyRawDataToImage();
}

void ComputeEquirectangular::createOutputBuffer() {
    LOG("ComputeEquirectangular::createOutputBuffer");
    Buffer* pOutput = new Buffer();
    uint lengthSize = m_details.mapSize * m_details.mapSize;
    uint outputSize = 6 * lengthSize * CHANNEL * sizeof(float);

    pOutput->setup(outputSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    pOutput->create();
    
    { m_pOutputBuffer = pOutput; }
}

void ComputeEquirectangular::createDescriptor() {
    LOG("ComputeEquirectangular::createDescriptor");
    VkDescriptorBufferInfo outputBInfo  = m_pOutputBuffer->getBufferInfo();
    VkDescriptorImageInfo  outputIInfo  = m_pEquirectangular->getImageInfo();
    
    Descriptor* pDescriptor = new Descriptor();
    pDescriptor->setupLayout(L0);
    pDescriptor->addLayoutBindings(L0, B0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                   VK_SHADER_STAGE_COMPUTE_BIT);
    pDescriptor->createLayout(L0);
    
    pDescriptor->setupLayout(L1);
    pDescriptor->addLayoutBindings(L1, B0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                   VK_SHADER_STAGE_FRAGMENT_BIT);
    pDescriptor->createLayout(L1);
    
    pDescriptor->createPool();
    pDescriptor->allocate(L0);
    pDescriptor->allocate(L1);

    pDescriptor->setupPointerBuffer(L0, S0, B0, &outputBInfo);
    pDescriptor->setupPointerImage (L1, S0, B0, &outputIInfo);
    pDescriptor->update(L0);
    
    { m_pDescriptor = pDescriptor; }
}

void ComputeEquirectangular::createPipelineLayout() {
    LOG("ComputeEquirectangular::createPipelineLayout");
    VkDevice device = m_device;
    VkDescriptorSetLayout descriptorSetLayout = m_pDescriptor->getDescriptorLayout(L0);
    
    VkPushConstantRange constantRange{};
    constantRange.size       = sizeof(ComputeDetails);
    constantRange.offset     = 0;
    constantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges    = &constantRange;

    VkPipelineLayout pipelineLayout;
    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    CHECK_VKRESULT(result, "failed to create pipeline layout!");

    { m_pipelineLayout = pipelineLayout; }
}

void ComputeEquirectangular::createPipeline() {
    LOG("ComputeEquirectangular::createPipeline");
    VkDevice device = m_device;
    Shader*  shader = m_pShader;
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
