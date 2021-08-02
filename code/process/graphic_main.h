//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"
#include "../window/window.h"
#include "../renderer/descriptor.h"
#include "../renderer/pipeline_graphic.h"
#include "../renderer/frame.h"
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

class GraphicMain {
    
public:
    const VkClearValue CLEARCOLOR = {0.1f, 0.1f, 0.1f, 1.0f};
    const VkClearValue CLEARDS    = {1.0f, 0.0};
   
    GraphicMain();
    ~GraphicMain();
    
    void cleanup();
    void cleanupSwapchain();
    void setup(Window* pWindow);
    
    void draw();
    
    void drawCommand(Frame* pFrame);
    
    void setShaders(std::vector<Shader*> shaders);
    
    CameraMatrix m_cameraMatrix{};
    
    
//private:
    
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    
    Window*          m_pWindow     = nullptr;
    Descriptor*      m_pDescriptor = nullptr;
    PipelineGraphic* m_pPipeline   = nullptr;
    
    VkSwapchainCreateInfoKHR m_swapchainInfo{};
    VkExtent2D m_extent;
    VkFormat m_surfaceFormat;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkRenderPass  m_renderPass = VK_NULL_HANDLE;
    
    
    uint m_totalFrame;
    std::vector<Frame*> m_frames;
    std::vector<VkSemaphore> m_imageSemaphores;
    
    Mesh*  m_pCube;
    Mesh*  m_pFloor;
    
    Size<int> m_size;
    size_t m_currentFrame = 0;
    
    std::vector<Shader*> m_pShaders;
    
    void createModel();
    
    void fillInput();
    void createSwapchain();
    void createRenderPass();
    void createFrames();
    void createSyncObjects();
    void createDescriptor();
    void createPipeline();
};
