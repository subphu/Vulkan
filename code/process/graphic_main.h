//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "../window/window.h"
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
    glm::vec3 viewPosition;
    uint buffSize;
};

class GraphicMain {
    
public:
    const std::string MODEL_PATH = "models/bunny/bunny.obj";
    const uint TEX_IDX = 6; // 3,4,
    const std::vector<std::string> TEXTURES = {"cliffrockface", "cobblestylized", "greasypan", "layered-rock1", "limestone6",  "roughrockface", "rustediron", "slimy-slippery-rock1", "slipperystonework", "worn-wet-old-cobblestone"};
    const std::string TEX_PATH = TEXTURES[TEX_IDX] + "/" + TEXTURES[TEX_IDX];
    const std::string TEX_ALBEDO_PATH   = "textures/pbr/" + TEX_PATH + "_albedo.png";
    const std::string TEX_AO_PATH       = "textures/pbr/" + TEX_PATH + "_ao.png";
    const std::string TEX_METALLIC_PATH = "textures/pbr/" + TEX_PATH + "_metallic.png";
    const std::string TEX_NORMAL_PATH   = "textures/pbr/" + TEX_PATH + "_normal.png";
    const std::string TEX_ROUGNESS_PATH = "textures/pbr/" + TEX_PATH + "_roughness.png";
    const std::vector<std::string> TEXURES_PATH = {
        TEX_ALBEDO_PATH, TEX_AO_PATH, TEX_METALLIC_PATH,
        TEX_NORMAL_PATH, TEX_ROUGNESS_PATH };
    
    const VkClearValue CLEARCOLOR = {0.1f, 0.1f, 0.1f, 1.0f};
    const VkClearValue CLEARDS    = {1.0f, 0.0};
   
    GraphicMain();
    ~GraphicMain();
    
    void cleanup();
    void reset();
    void setup(Window* pWindow);
    
    void draw();
    
    void drawCommand(Frame* pFrame);
    
    void setInterBuffer(Buffer* buffer);
    void setShaders(std::vector<Shader*> shaders);
    
    Swapchain*   m_pSwapchain  = nullptr;
    CameraMatrix m_cameraMatrix{};
    
    Misc m_misc{};
    
//private:
    
    Window*          m_pWindow      = nullptr;
    Descriptor*      m_pDescriptor = nullptr;
    PipelineGraphic* m_pPipeline   = nullptr;
    
    Mesh* m_pMesh;
    Image* m_pTexAlbedo;
    std::vector<Image*> m_pTextures;
    
    Size<int> m_size;
    size_t m_currentFrame = 0;
    Buffer* m_pMiscBuffer;
    Buffer* m_pInterBuffer;
    
    std::vector<Shader*> m_pShaders;
    
    void createTexture();
    void createModel();
    
    void fillInput();
    void createBuffers();
    void createSwapchain();
    void createDescriptor();
    void createPipelines();
};
