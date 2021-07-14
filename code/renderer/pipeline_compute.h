//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "shader.h"

class PipelineCompute {
    
public:
    PipelineCompute();
    ~PipelineCompute();
    
    void cleanup();
    
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    
    VkPipeline       m_pipeline       = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    
    VkPushConstantRange m_constantRange{};
    
    Shader* m_shader;
    
    void setShader(Shader* shader);
    void setupPushConstant(uint size);
    void createPipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
    void create();
    
private:
    
};
