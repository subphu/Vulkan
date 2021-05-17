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
    m_pBufferOutput->cleanup();
    m_pPipeline->cleanup();
    m_pDescriptor->cleanup();
}

void ComputeInterference::setup(uint size) {
    LOG("ComputeInterference::setup");
    m_size = { size, size };
    fillInput();
    createBuffers();
    createDescriptor();
    createPipeline();
}

void ComputeInterference::dispatch() {
    LOG("ComputeInterference::dispatch");
    Commander* commander = System::Commander();
    
    Size<uint> size = m_size;
    VkPipeline          pipeline       = m_pPipeline->m_pipeline;
    VkPipelineLayout    pipelineLayout = m_pPipeline->m_pipelineLayout;
    VkDescriptorSet     descSet        = m_pDescriptor->getDescriptorSets(L0)[0];
    InterferenceDetails inputConstant  = m_interferenceDetails;
    
    VkCommandBuffer commandBuffer = commander->createCommandBuffer();
    commander->beginSingleTimeCommands(commandBuffer);
    {
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT,
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

void ComputeInterference::setShaderPath(std::string path) { m_shaderPath = path; }

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
    uint outputSize = lengthSize * CHANNEL * sizeof(float);
    std::vector<float> outputData(lengthSize, 0.0f);
    
    Buffer* pOutput = new Buffer();
    pOutput->setup(outputSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
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
    Descriptor* pDescriptor = m_pDescriptor;
    Shader*     computeShader = new Shader(m_shaderPath, VK_SHADER_STAGE_COMPUTE_BIT);
    
    PipelineCompute* pPipeline = new PipelineCompute();
    pPipeline->setShader(computeShader);
    pPipeline->setupPushConstant(sizeof(InterferenceDetails));
    pPipeline->createPipelineLayout({ pDescriptor->getDescriptorLayout(L0) });
    pPipeline->create();
    
    { m_pPipeline = pPipeline; }
}

