//  Copyright © 2021 Subph. All rights reserved.
//

#include "shader.h"

#include "../helper.h"
#include "../system.h"

Shader::~Shader() {}
Shader::Shader() {
    System &system = System::instance();
    m_device       = system.getRenderer()->m_device;
}

Shader::Shader(const std::string filepath, VkShaderStageFlagBits stage, const char* entryPoint) {
    System &system  = System::instance();
    m_device = system.getRenderer()->m_device;
    
    createModule(filepath);
    createStageInfo(stage, entryPoint);
}

void Shader::cleanup() {
    LOG("Shader::cleanup");
    vkDestroyShaderModule(m_device, m_shaderModule, nullptr);
}

void Shader::createModule(const std::string filepath) {
    LOG("Shader::createModule");
    VkDevice device = m_device;
    auto code = ReadBinaryFile(filepath);
    
    VkShaderModuleCreateInfo shaderInfo{};
    shaderInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderInfo.codeSize = code.size();
    shaderInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());
    
    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(device, &shaderInfo, nullptr, &shaderModule);
    CHECK_VKRESULT(result, "failed to create shader modul!");
    
    {
        m_filepath        = filepath;
        m_shaderModule    = shaderModule;
    }
}

void Shader::createStageInfo(VkShaderStageFlagBits stage, const char* entryPoint) {
    VkShaderModule shaderModule = m_shaderModule;
    
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage  = stage;
    shaderStageInfo.pName  = entryPoint;
    shaderStageInfo.module = shaderModule;
    
    { m_shaderStageInfo = shaderStageInfo; }
}

VkPipelineShaderStageCreateInfo Shader::getShaderStageInfo() {
    return m_shaderStageInfo;
}
