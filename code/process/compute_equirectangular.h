//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "../renderer/descriptor.h"
#include "../renderer/pipeline_compute.h"
#include "../resources/shader.h"
#include "../resources/buffer.h"
#include "../resources/image.h"

#define WORKGROUP_SIZE 16
#define CHANNEL 4

class ComputeEquirectangular {
    
    struct ComputeDetails {
        uint width;
        uint height;
        uint mapSize;
    };
    
    struct OutputData {
        std::vector<float> outputData[6];
    };

public:
    const std::string TEXTURE_PATH = "textures/cubemap/Arches_E_PineTree/Arches_E_PineTree.hdr";
    const std::string SHADER_PATH  = "shaders/cubemap.comp.spv";
    
    ComputeEquirectangular();
    ~ComputeEquirectangular();
    
    void cleanup();
    void setup(Size<uint> size);
    void dispatch();
    
private:
    
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    
    VkPipeline       m_pipeline       = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    
    VkPushConstantRange m_constantRange{};
    
    Shader    * m_pShader;
    Image     * m_pCubemap;
    Image     * m_pEquirectangular;
    Buffer    * m_pOutputBuffer;
    Descriptor* m_pDescriptor;
    
    ComputeDetails m_details;
    
    void createEquirectangular();
    void createOutputBuffer();
    void createDescriptor();
    void createPipelineLayout();
    void createPipeline();
    void createCubemap();
};
