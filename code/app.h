//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "window/window.h"
#include "window/gui.h"
#include "camera/camera.h"
#include "mesh/mesh.h"

#include "renderer/renderer.h"
#include "renderer/commander.h"
#include "renderer/swapchain.h"
#include "resources/shader.h"

#include "renderer/descriptor.h"
#include "renderer/pipeline_graphic.h"
#include "renderer/pipeline_compute.h"

#include "process/compute_interference.h"
#include "process/graphic_main.h"


class App {
public:
    
    const char* SHADER_COMPILER_PATH = "shaders/compile.sh";
    
    void run();
    float duration1 = 0;
    float duration2 = 0;

private:
    Window* m_pWindow;
    GUI* m_pGui;
    Camera* m_pCamera;
    Renderer* m_pRenderer;
    GraphicMain* m_pGraphicMain;
    ComputeInterference* m_pComputeInterference;
    
    
    size_t m_currentFrame = 0;
    CameraMatrix m_cameraMatrix{};
    
    Misc m_misc{};
    
    void cleanup();
    
    void initWindow();
    void initVulkan();
    
    void recreateSwapchain();
    void recordCommandBuffer();
    
    void createGUI();
    void createTexture();
    void createModel();
    void createShaders();
    void createDescriptor();
    
    void createPipelineGraphic();
    void createPipelineCompute();
    
    void mainLoop();
    void mainLoopFps();
    void update(long iteration);
    void draw(long iteration);
    
    void moveView(Window* pWindow);
    void moveViewLock(Window* pWindow);
};
