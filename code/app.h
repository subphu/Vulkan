//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "window/window.h"
#include "renderer/renderer.h"
#include "renderer/commander.h"
#include "renderer/swapchain.h"
#include "camera/camera.h"
#include "mesh/mesh.h"
#include "renderer/shader.h"

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
    
    const std::string VERT_SHADER_PATH = "shaders/vert.spv";
    const std::string FRAG_SHADER_PATH = "shaders/frag.spv";
    const std::string MODEL_PATH   = "models/viking_room/viking_room.obj";
    const std::string TEXTURE_PATH = "textures/rustediron/rustediron_albedo.png";
    
    void run();

private:
    Window* m_window;
    Camera* m_camera;
    
    Renderer* m_renderer;
    Swapchain* m_swapchain;
    Commander* m_commander;
    
    Mesh* m_model;
    ResourceImage *m_texture;
    
    size_t m_currentFrame = 0;
    UniformBufferObject m_ubo{};
    
    
    void initWindow();
    void initVulkan();
    
    void recreateSwapchain();
    void recordCommandBuffer();
    
    void createTexture();
    void createModel();
    
    void mainLoop();
    void update(long iteration);
    void draw(long iteration);
    
};
