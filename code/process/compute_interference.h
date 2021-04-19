//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "../renderer/descriptor.h"
#include "../renderer/pipeline_compute.h"
#include "../resources/shader.h"
#include "../resources/buffer.h"

#define WORKGROUP_SIZE 16
#define CHANNEL 4

struct InterferenceDetails {
    uint width;
    uint height;
    float n; // refractive index
};

class ComputeInterference {
    
public:
    const std::string COMP_SHADER_PATH = "shaders/SPV/interference.comp.spv";
    
    ComputeInterference();
    ~ComputeInterference();
    
    void cleanup();
    void setup(uint size);
    void dispatch();
    
    Buffer* getOutputBuffer();
    
private:
    
    Descriptor* m_pDescriptor;
    PipelineCompute* m_pPipeline;
    
    Buffer* m_pBufferOutput;
    
    Size<uint> m_size;
    InterferenceDetails m_interferenceDetails;
    
    void fillInput();
    void createBuffers();
    void createDescriptor();
    void createPipeline();
};
