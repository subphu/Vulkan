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
    
    const char* SHADER_COMPILER_PATH = "shaders/compile.sh";
    const std::string MODEL_PATH = "models/viking_room/viking_room.obj";
    
    void run();

private:
    Window* m_pGUIWindow;
    Window* m_pComputeWindow;
    Window* m_pRenderWindow1;
    Window* m_pRenderWindow2;
    
    Camera* m_pCamera;
    
    Renderer* m_pRenderer;
    
    ComputeInterference* m_pCInterference1D;
    ComputeInterference* m_pCInterference2D;
    GraphicMain* m_pGMain1;
    GraphicMain* m_pGMain2;
    
    size_t m_currentFrame = 0;
    CameraMatrix m_cameraMatrix{};
    
    Misc m_misc{};
    
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
    
    glm::vec3 getMovement(Window* pWindow);
};
