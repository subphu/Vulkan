//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "../renderer/descriptor.h"
#include "../renderer/swapchain.h"
#include "../renderer/pipeline_graphic.h"
#include "../resources/shader.h"
#include "../resources/buffer.h"
#include "../mesh/mesh.h"

#define WORKGROUP_SIZE 16
#define CHANNEL 4

struct CameraMatrix {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Misc {
    uint buffSize;
};

class GraphicMain {
    
public:
    const std::string VERT_SHADER_PATH = "shaders/vert.spv";
    const std::string FRAG_SHADER_PATH = "shaders/frag.spv";
    const std::string TEXTURE_PATH = "textures/rustediron/rustediron_albedo.png";
    
    const VkClearValue CLEARCOLOR = {0.8f, 0.8f, 0.8f, 1.0f};
    const VkClearValue CLEARDS    = {1.0f, 0.0};
   
    GraphicMain();
    ~GraphicMain();
    
    void cleanup();
    void reset();
    void setup(Size<int> size);
    
    void draw();
    
    void createDrawCommand();
    void drawCommand(Frame* pFrame);
    
    Swapchain*   m_pSwapchain  = nullptr;
    CameraMatrix m_cameraMatrix{};
    
    Misc m_misc{};
    
//private:
    
    Descriptor*      m_pDescriptor = nullptr;
    PipelineGraphic* m_pPipeline   = nullptr;
    
    Mesh* m_pMesh;
    Image* m_pTexture;
    
    Size<int> m_size;
    size_t m_currentFrame = 0;
    Buffer* m_pMiscBuffer;
    Buffer* m_pInterBuffer;
    
    void createTexture();
    void createModel();
    
    void fillInput();
    void createBuffers();
    void createSwapchain();
    void createDescriptor();
    void createPipeline();
};
