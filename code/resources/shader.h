//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"

class Shader {
    
public:
    ~Shader();
    Shader();
    Shader(const std::string filepath, VkShaderStageFlagBits stage, const char* entryPoint = "main");
    
    void cleanup();
    
    VkDevice       m_device       = VK_NULL_HANDLE;
    VkShaderModule m_shaderModule = VK_NULL_HANDLE;
    
    VkShaderModuleCreateInfo        m_shaderInfo{};
    VkPipelineShaderStageCreateInfo m_shaderStageInfo{};
    
    std::string m_filepath;
    
    void createModule(const std::string filepath);
    void createStageInfo(VkShaderStageFlagBits stage, const char* entryPoint = "main");
    
    VkPipelineShaderStageCreateInfo getShaderStageInfo();
    
private:

};
