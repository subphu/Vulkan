//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "shader.h"

class PipelineGraphic {
    
public:
    PipelineGraphic();
    ~PipelineGraphic();
    
    void cleanup();
    
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    
    VkPipeline       m_pipeline       = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    
    VkPipelineViewportStateCreateInfo*      m_viewportInfo{};
    
    VkPipelineInputAssemblyStateCreateInfo* m_inputAssemblyInfo{};
    VkPipelineRasterizationStateCreateInfo* m_rasterizationInfo{};
    VkPipelineMultisampleStateCreateInfo*   m_multisampleInfo{};

    VkPipelineColorBlendAttachmentState* m_colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo* m_colorBlendInfo{};

    std::vector<VkDynamicState>*      m_dynamicStates;
    VkPipelineDynamicStateCreateInfo* m_dynamicInfo{};

    VkPipelineDepthStencilStateCreateInfo* m_depthStencilInfo{};
    
    std::vector<Shader*> m_shaders;
    VkPushConstantRange  m_constantRange{};
    VkPipelineVertexInputStateCreateInfo* m_vertexInputInfo;
    
    void setShaders(std::vector<Shader*> shaders);
    void setVertexInputInfo(VkPipelineVertexInputStateCreateInfo* vertexInputInfo);
    
    void setupPushConstant(uint size);
    void createPipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
    
    void setupViewportInfo(VkExtent2D swapchainExtent);
    void setupInputAssemblyInfo();
    void setupRasterizationInfo();
    void setupMultisampleInfo();
    void setupColorBlendInfo();
    void setupDynamicInfo();
    void setupDepthStencilInfo();

    void create(VkRenderPass renderPass);
    
private:
    static VkFormat ChooseDepthFormat();

};
