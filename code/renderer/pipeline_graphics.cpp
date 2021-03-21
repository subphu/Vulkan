//  Copyright © 2021 Subph. All rights reserved.
//

#include <array>

#include "pipeline_graphics.h"

#include "../system.h"

PipelineGraphics::~PipelineGraphics() {}
PipelineGraphics::PipelineGraphics() {
    System &system   = System::instance();
    m_device         = system.m_renderer->m_device;
    m_physicalDevice = system.m_renderer->m_physicalDevice;
}

void PipelineGraphics::cleanup() {
    for (int i = 0; i < m_shaders.size(); i++) m_shaders[i]->cleanup();
    vkDestroyPipeline(m_device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
}

void PipelineGraphics::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout) {
    LOG("createPipelineLayout");
    VkDevice device = m_device;
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts    = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges    = nullptr;
    
    VkPipelineLayout pipelineLayout;
    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    CHECK_VKRESULT(result, "failed to create pipeline layout!");
    
    { m_pipelineLayout = pipelineLayout; }
}

void PipelineGraphics::setupViewportInfo(VkExtent2D swapchainExtent) {
    VkViewport* viewport = new VkViewport();
    viewport->x = 0.0f;
    viewport->y = 0.0f;
    viewport->width  = (float) swapchainExtent.width;
    viewport->height = (float) swapchainExtent.height;
    viewport->minDepth = 0.0f;
    viewport->maxDepth = 1.0f;
    
    VkRect2D* scissor = new VkRect2D();
    scissor->offset = {0, 0};
    scissor->extent = swapchainExtent;
    
    VkPipelineViewportStateCreateInfo* viewportInfo = new VkPipelineViewportStateCreateInfo();
    viewportInfo->sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo->viewportCount = 1;
    viewportInfo->pViewports    = viewport;
    viewportInfo->scissorCount  = 1;
    viewportInfo->pScissors     = scissor;
    
    { m_viewportInfo = viewportInfo; }
}

void PipelineGraphics::setupInputAssemblyInfo() {
    VkPipelineInputAssemblyStateCreateInfo* inputAssemblyInfo = new VkPipelineInputAssemblyStateCreateInfo();
    inputAssemblyInfo->sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo->primitiveRestartEnable = VK_FALSE;
    
    { m_inputAssemblyInfo = inputAssemblyInfo; }
}

void PipelineGraphics::setupRasterizationInfo() {
    VkPipelineRasterizationStateCreateInfo* rasterizationInfo = new VkPipelineRasterizationStateCreateInfo();
    rasterizationInfo->sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo->polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationInfo->cullMode    = VK_CULL_MODE_BACK_BIT;
    rasterizationInfo->frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationInfo->rasterizerDiscardEnable = VK_FALSE;
    rasterizationInfo->depthClampEnable        = VK_FALSE;
    rasterizationInfo->depthBiasEnable         = VK_FALSE;
    rasterizationInfo->depthBiasClamp          = 0.0f;
    rasterizationInfo->depthBiasSlopeFactor    = 0.0f;
    rasterizationInfo->depthBiasConstantFactor = 0.0f;
    rasterizationInfo->lineWidth               = 1.0f;
    
    { m_rasterizationInfo = rasterizationInfo; }
}

void PipelineGraphics::setupMultisampleInfo() {
    VkPipelineMultisampleStateCreateInfo* multisampleInfo = new VkPipelineMultisampleStateCreateInfo();
    multisampleInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo->rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo->sampleShadingEnable   = VK_FALSE;
    multisampleInfo->minSampleShading      = 1.0f;
    multisampleInfo->pSampleMask           = nullptr;
    multisampleInfo->alphaToOneEnable      = VK_FALSE;
    multisampleInfo->alphaToCoverageEnable = VK_FALSE;
    
    { m_multisampleInfo = multisampleInfo; }
}

void PipelineGraphics::setupColorBlendInfo() {
    VkPipelineColorBlendAttachmentState* colorBlendAttachment = new VkPipelineColorBlendAttachmentState();
    colorBlendAttachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment->blendEnable         = VK_FALSE;
    colorBlendAttachment->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment->dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment->colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment->alphaBlendOp        = VK_BLEND_OP_ADD;
    
    VkPipelineColorBlendStateCreateInfo* colorBlendInfo = new VkPipelineColorBlendStateCreateInfo();
    colorBlendInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo->logicOpEnable     = VK_FALSE;
    colorBlendInfo->logicOp           = VK_LOGIC_OP_COPY;
    colorBlendInfo->attachmentCount   = 1;
    colorBlendInfo->pAttachments      = colorBlendAttachment;
    colorBlendInfo->blendConstants[0] = 0.0f;
    colorBlendInfo->blendConstants[1] = 0.0f;
    colorBlendInfo->blendConstants[2] = 0.0f;
    colorBlendInfo->blendConstants[3] = 0.0f;
    
    {
        m_colorBlendAttachment = colorBlendAttachment;
        m_colorBlendInfo = colorBlendInfo;
    }
}

void PipelineGraphics::setupDynamicInfo() {
    std::vector<VkDynamicState>* dynamicStates = new std::vector<VkDynamicState>();
    dynamicStates->push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStates->push_back(VK_DYNAMIC_STATE_LINE_WIDTH);

    VkPipelineDynamicStateCreateInfo* dynamicInfo = new VkPipelineDynamicStateCreateInfo();
    dynamicInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicInfo->dynamicStateCount = 2;
    dynamicInfo->pDynamicStates    = dynamicStates->data();
    
    {
        m_dynamicStates = dynamicStates;
        m_dynamicInfo = dynamicInfo;
    }
}

void PipelineGraphics::setupDepthStencilInfo() {
    VkPipelineDepthStencilStateCreateInfo* depthStencilInfo = new VkPipelineDepthStencilStateCreateInfo();
    depthStencilInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo->depthTestEnable        = VK_TRUE;
    depthStencilInfo->depthWriteEnable       = VK_TRUE;
    depthStencilInfo->depthBoundsTestEnable  = VK_FALSE;
    depthStencilInfo->depthCompareOp         = VK_COMPARE_OP_LESS;
    depthStencilInfo->minDepthBounds         = 0.0f;
    depthStencilInfo->maxDepthBounds         = 1.0f;
    depthStencilInfo->stencilTestEnable      = VK_FALSE;
    
    { m_depthStencilInfo = depthStencilInfo; }
}

void PipelineGraphics::setShaders(std::vector<Shader*> shaders) {
    m_shaders = shaders;
}

void PipelineGraphics::setVertexInputInfo(VkPipelineVertexInputStateCreateInfo* vertexInputInfo) {
    m_vertexInputInfo = vertexInputInfo;
}

void PipelineGraphics::create(VkRenderPass renderPass) {
    LOG("createGraphicsPipelines");
    VkDevice         device         = m_device;
    VkPipelineLayout pipelineLayout = m_pipelineLayout;
    VkPipelineViewportStateCreateInfo*      viewportInfo      = m_viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo* inputAssemblyInfo = m_inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo* rasterizationInfo = m_rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo*   multisampleInfo   = m_multisampleInfo;
    VkPipelineColorBlendStateCreateInfo*    colorBlendInfo    = m_colorBlendInfo;
    VkPipelineDynamicStateCreateInfo*       dynamicInfo       = m_dynamicInfo;
    VkPipelineDepthStencilStateCreateInfo*  depthStencilInfo  = m_depthStencilInfo;
    
    std::vector<Shader*> shaders = m_shaders;
    VkPipelineVertexInputStateCreateInfo* vertexInputInfo = m_vertexInputInfo;
    
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for (int i = 0; i < shaders.size(); i++) {
        shaderStages.push_back(shaders[i]->getShaderStageInfo());
    }
    
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.layout     = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass    = 0;
    pipelineInfo.stageCount = UINT32(shaders.size());
    pipelineInfo.pStages             = shaderStages.data();
    pipelineInfo.pVertexInputState   = vertexInputInfo;
    pipelineInfo.pViewportState      = viewportInfo;
    pipelineInfo.pInputAssemblyState = inputAssemblyInfo;
    pipelineInfo.pRasterizationState = rasterizationInfo;
    pipelineInfo.pMultisampleState   = multisampleInfo;
    pipelineInfo.pColorBlendState    = colorBlendInfo;
    pipelineInfo.pDynamicState       = dynamicInfo;
    pipelineInfo.pDepthStencilState  = depthStencilInfo;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex   = -1;
    
    VkPipeline pipeline;
    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
    CHECK_VKRESULT(result, "failed to create graphics pipeline!");
    
    { m_pipeline = pipeline; }
}

// ==================================================

VkFormat PipelineGraphics::ChooseDepthFormat() {
    VkPhysicalDevice physicalDevice = System::instance().m_renderer->m_physicalDevice;
    
    const std::vector<VkFormat>& candidates = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        
        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return format;
        }
    }
    
    throw std::runtime_error("failed to find depth format!");
}
