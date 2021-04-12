//  Copyright © 2021 Subph. All rights reserved.
//

#include "compute_interference.h"

#include "../system.h"

ComputeInterference::~ComputeInterference() {}
ComputeInterference::ComputeInterference() {
    LOG("ComputeInterference::==============================");
}

void ComputeInterference::cleanup() {
    LOG("ComputeInterference::cleanup");
    m_pPipeline->cleanup();
    m_pDescriptor->cleanup();
    m_pBufferOutput->cleanup();
}

void ComputeInterference::setup(Size<uint> size) {
    LOG("ComputeInterference::setup");
    m_size = size;
    fillInput();
    createBuffers();
    createDescriptor();
    createPipeline();
}

void ComputeInterference::dispatch() {
    LOG("ComputeInterference::dispatch");
    System&    system    = System::instance();
    Commander* commander = system.m_renderer->getCommander();
    
    Size<uint> size = m_size;
    VkPipeline          pipeline       = m_pPipeline->m_pipeline;
    VkPipelineLayout    pipelineLayout = m_pPipeline->m_pipelineLayout;
    VkDescriptorSet     descSet        = m_pDescriptor->getDescriptorSets(L0)[0];
    InterferenceDetails inputConstant  = m_interferenceDetails;
    
    VkCommandBuffer commandBuffer = commander->createCommandBuffer();
    commander->beginSingleTimeCommands(commandBuffer);
    {
        vkCmdPushConstants(commandBuffer, pipelineLayout,
                           VK_SHADER_STAGE_COMPUTE_BIT,
                           0, sizeof(inputConstant), &inputConstant);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                                pipelineLayout, 0, 1, &descSet, 0, nullptr);
        
        vkCmdDispatch(commandBuffer,
                      size.width  / WORKGROUP_SIZE,
                      size.height / WORKGROUP_SIZE, 1);
    }
    commander->endSingleTimeCommands(commandBuffer);
}

Buffer* ComputeInterference::getOutputBuffer() { return m_pBufferOutput; }

// Private ==================================================


void ComputeInterference::fillInput() {
    LOG("ComputeInterference::fillInput");
    m_interferenceDetails.n      = 1.5f;
    m_interferenceDetails.width  = m_size.width;
    m_interferenceDetails.height = m_size.height;
}

void ComputeInterference::createBuffers() {
    LOG("ComputeInterference::createBuffers");
    Size<uint> size = m_size;
    uint lengthSize = size.width * size.height;
    uint outputSize = lengthSize * CHANNEL;
    std::vector<float> outputData(lengthSize, 0.0f);
    
    Buffer* pOutput = new Buffer();
    pOutput->setup(outputSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    pOutput->create();
    pOutput->fillBufferFull(outputData.data());
    
    { m_pBufferOutput = pOutput; }
}

void ComputeInterference::createDescriptor() {
    LOG("ComputeInterference::createDescriptor");
    Buffer* pOutput  = m_pBufferOutput;
    VkDescriptorBufferInfo outputBInfo  = pOutput->getBufferInfo();
    
    Descriptor* pDescriptor = new Descriptor();
    pDescriptor->setupLayout(L0);
    pDescriptor->addLayoutBindings(L0, B0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                   VK_SHADER_STAGE_COMPUTE_BIT);
    pDescriptor->createLayout(L0);
    pDescriptor->createPool();

    pDescriptor->allocate(L0);
    pDescriptor->setupPointerBuffer(L0, S0, B0, &outputBInfo);
    pDescriptor->update(L0);
    
    { m_pDescriptor = pDescriptor; }
}

void ComputeInterference::createPipeline() {
    LOG("ComputeInterference::createPipeline");
    Descriptor*      pDescriptor = m_pDescriptor;
    VkDescriptorSetLayout layout = pDescriptor->getDescriptorLayout(L0);
    Shader*        computeShader = new Shader(COMP_SHADER_PATH, VK_SHADER_STAGE_COMPUTE_BIT);
    
    PipelineCompute* pPipelineCompute = new PipelineCompute();
    pPipelineCompute->setShader(computeShader);
    pPipelineCompute->setupPushConstant(sizeof(InterferenceDetails));
    pPipelineCompute->createPipelineLayout({ layout });
    pPipelineCompute->create();
    
    { m_pPipeline = pPipelineCompute; }
}

