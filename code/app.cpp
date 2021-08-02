//  Copyright © 2021 Subph. All rights reserved.
//

#include <array>

#include "app.h"
#include "helper.h"
#include "system.h"

#define WIDTH   1200
#define HEIGHT  800
#define TEXSIZE 256
#define WINDOW_X 50
#define WINDOW_Y 100

void App::run() {
    initWindow();
    initVulkan();
    m_pCamera = new Camera();
    process();
    cleanup();
}

void App::cleanup() {
    m_pGraphicMain->cleanup();
    m_pWindow->cleanup();
    m_pRenderer->cleanUp();
}

void App::initVulkan() {
    LOG("App::initVulkan");
    m_pRenderer = new Renderer();
    System::Instance().m_pRenderer = m_pRenderer;
    
    m_pRenderer->setupValidation(IS_DEBUG);
    m_pRenderer->createInstance(Window::getRequiredExtensions());
    m_pRenderer->createDebugMessenger();
    m_pRenderer->setupDeviceExtensions();
    
    m_pWindow->createSurface(m_pRenderer->getInstance());
    
    m_pRenderer->pickPhysicalDevice(m_pWindow->getSurface());
    m_pRenderer->createLogicalDevice();
    m_pRenderer->createDeviceQueue();
    m_pRenderer->createCommander();

    createPipelineGraphic();
}

void App::initWindow() {
    LOG("App::initWindow");
    m_pWindow = new Window();
    m_pWindow->create(WIDTH, HEIGHT, "Vulkan");
    m_pWindow->setWindowPosition(WINDOW_X, WINDOW_Y);
    m_pWindow->enableInput();
}

void App::createPipelineGraphic() {
    LOG("App::createPipelineGraphic");
    
    GraphicMain* graphic1 = new GraphicMain();
    graphic1->setShaders({
        new Shader("shaders/vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        new Shader("shaders/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    });
    graphic1->setup(m_pWindow);
    
    m_pGraphicMain = graphic1;
}

void App::moveView(Window* pWindow) {
    pWindow->pollEvents();
    glm::vec3 movement = glm::vec3(0.f, 0.f, 0.f);
    movement.x += pWindow->getKeyState(key_d) - pWindow->getKeyState(key_a);
    movement.y += pWindow->getKeyState(key_q) - pWindow->getKeyState(key_e);
    movement.z += pWindow->getKeyState(key_w) - pWindow->getKeyState(key_s);
    m_pCamera->move(movement);
    
    glm::vec2 delta = pWindow->getCursorMovement();
    m_pCamera->turn(delta * glm::vec2(4.0, 4.0));
}

void App::process() {
    while (m_pWindow->isOpen()) {
        moveView(m_pWindow);
        
        m_cameraMatrix.view = m_pCamera->getViewMatrix();
        m_cameraMatrix.proj = m_pCamera->getProjection((float) WIDTH / HEIGHT);
        m_pGraphicMain->m_cameraMatrix = m_cameraMatrix;
        m_pGraphicMain->draw();
    }
    vkDeviceWaitIdle(m_pRenderer->getDevice());
}
