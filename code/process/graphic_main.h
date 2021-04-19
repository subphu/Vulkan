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
    glm::vec3 camera;
    uint buffSize;
};

class GraphicMain {
    
public:
    const std::string VERT_SHADER_PATH = "shaders/SPV/main.vert.spv";
    const std::string FRAG_SHADER_PATH = "shaders/SPV/main.frag.spv";
    
    const std::string TEX_ALBEDO_PATH   = "textures/rustediron/rustediron_albedo.png";
    const std::string TEX_AO_PATH       = "textures/rustediron/rustediron_ao.png";
    const std::string TEX_METALLIC_PATH = "textures/rustediron/rustediron_metallic.png";
    const std::string TEX_NORMAL_PATH   = "textures/rustediron/rustediron_normal.png";
    const std::string TEX_ROUGNESS_PATH = "textures/rustediron/rustediron_roughness.png";
    const std::vector<std::string> TEXURES_PATH = {
        TEX_ALBEDO_PATH, TEX_AO_PATH, TEX_METALLIC_PATH,
        TEX_NORMAL_PATH, TEX_ROUGNESS_PATH };
    
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
    Image* m_pTexAlbedo;
    std::vector<Image*> m_pTextures;
    
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
