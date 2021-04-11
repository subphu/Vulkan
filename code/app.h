//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "window/window.h"
#include "renderer/renderer.h"
#include "renderer/commander.h"
#include "renderer/swapchain.h"
#include "camera/camera.h"
#include "mesh/mesh.h"
#include "resources/shader.h"

#include "renderer/descriptor.h"
#include "renderer/pipeline_graphic.h"
#include "renderer/pipeline_compute.h"

#include "process/compute_interference.h"

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Texture {
    VkImageView imageView;
    VkSampler sampler;
};

class App {
public:
    const uint32_t WIDTH   = 1200;
    const uint32_t HEIGHT  = 800;
    const int DOUBLEBUFFER = 2;
    
    const char*   SHADER_COMPILER_PATH = "shaders/compile.sh";
    const std::string VERT_SHADER_PATH = "shaders/vert.spv";
    const std::string FRAG_SHADER_PATH = "shaders/frag.spv";
    const std::string COMP_SHADER_PATH = "shaders/comp.spv";
    const std::string MODEL_PATH   = "models/viking_room/viking_room.obj";
    const std::string TEXTURE_PATH = "textures/rustediron/rustediron_albedo.png";
    
    void run();

private:
    Window* m_pWindow;
    Camera* m_pCamera;
    
    Renderer* m_pRenderer;
    Swapchain* m_pSwapchain;
    Commander* m_pCommander;
    
    Descriptor* m_pDescriptor;
    
    PipelineGraphic* m_pPipelineGraphic;
    PipelineCompute*  m_pPipelineCompute;
    
    ComputeInterference* m_pCompute;
    
    Mesh* m_pModel;
    Image* m_pTexture;
    std::vector<Shader*> m_shaders;
    
    size_t m_currentFrame = 0;
    UniformBufferObject m_ubo{};
    
    
    void initWindow();
    void initVulkan();
    
    void recreateSwapchain();
    void recordCommandBuffer();
    
    void createTexture();
    void createModel();
    void createShaders();
    void createDescriptor();
    
    void createPipelineGraphic();
    void createPipelineCompute();
    
    void mainLoop();
    void update(long iteration);
    void draw(long iteration);
    
    
};
