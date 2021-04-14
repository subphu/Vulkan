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
#include "process/graphic_main.h"


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
    Commander* m_pCommander;
    
    PipelineCompute*  m_pPipelineCompute;
    
    ComputeInterference* m_pCompute;
    GraphicMain* m_pGraphic;
    
    
    size_t m_currentFrame = 0;
    CameraMatrix m_cameraMatrix{};
    
    Misc m_misc{};
    Buffer* m_pMiscBuffer;
    
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
