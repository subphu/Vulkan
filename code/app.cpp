//  Copyright © 2021 Subph. All rights reserved.
//

#include <array>

#include "app.h"
#include "helper.h"
#include "system.h"

void App::run() {
    initWindow();
    initVulkan();
    m_pCamera = new Camera();
    mainLoop();
    
    m_pCompute->cleanup();
    m_pGraphic->cleanup();
    m_pRenderer->cleanUp();
    m_pWindow->close();
}


void App::initWindow() {
    m_pWindow = new Window();
    m_pWindow->create(WIDTH, HEIGHT, "Vulkan");
    m_pWindow->enableInput();
}


void App::initVulkan() {
    System &system = System::instance();
    m_pRenderer = new Renderer();
    system.m_renderer  = m_pRenderer;
    m_pRenderer->setupValidation(IS_DEBUG);
    m_pRenderer->createInstance(Window::getRequiredExtensions());
    m_pRenderer->createDebugMessenger();
    m_pRenderer->setSurface(m_pWindow->createSurface(m_pRenderer->m_instance));
    m_pRenderer->setupDeviceExtensions();
    m_pRenderer->pickPhysicalDevice();
    m_pRenderer->createLogicalDevice();
    m_pRenderer->createDeviceQueue();
    
    m_pRenderer->createCommander();
    
    m_pCommander = m_pRenderer->getCommander();

    createPipelineCompute();
    
    m_pGraphic = new GraphicMain();
    m_pGraphic->m_pInterBuffer = m_pCompute->getOutputBuffer();
    m_pGraphic->setup(m_pWindow->getSize());
    m_pGraphic->m_misc.buffSize = TEXSIZE;
    
}

void App::createPipelineCompute() {
    LOG("createPipelineCompute");
    m_pCompute = new ComputeInterference();
    m_pCompute->setup(TEXSIZE);
    m_pCompute->dispatch();
}

void App::mainLoop() {
    auto lastTime = Time::now();
    float frameDelay = 1.f/60.f;
    float lag = frameDelay;
    
    long iteration = 0;
    while (m_pWindow->isOpen()) {
        iteration++;
        update(iteration);
        lag -= frameDelay;
        
        while (lag < frameDelay) {
            draw(iteration);
        
            lag += TimeDif(Time::now() - lastTime).count();
            lastTime = Time::now();
        }
    }
    vkDeviceWaitIdle(m_pRenderer->m_device);
}

void App::update(long iteration) {
    m_pWindow->pollEvents();
    glm::vec3 movement = glm::vec3(0.f, 0.f, 0.f);
    movement.x += m_pWindow->getKeyState(key_d) - m_pWindow->getKeyState(key_a);
    movement.y += m_pWindow->getKeyState(key_q) - m_pWindow->getKeyState(key_e);
    movement.z += m_pWindow->getKeyState(key_w) - m_pWindow->getKeyState(key_s);
    
    if (m_pWindow->getMouseBtnState(mouse_btn_left)) {
        glm::vec2 cursorOffset = m_pWindow->getCursorOffset();
        movement.x += cursorOffset.x * 0.2f;
        movement.y += cursorOffset.y * 0.2f;
    }
    movement.z += m_pWindow->getScrollOffset().y;
    m_pWindow->resetInput();
    
    m_pCamera->move(movement);
    Swapchain* m_pSwapchain = m_pGraphic->m_pSwapchain;
    
    m_cameraMatrix.model = glm::mat4(1.0f);
    m_cameraMatrix.model = glm::translate(m_cameraMatrix.model, glm::vec3(0.f, -0.5f, 0.f));
    m_cameraMatrix.model = glm::scale(m_cameraMatrix.model, glm::vec3(3.0));
    m_cameraMatrix.view = m_pCamera->getViewMatrix();
    m_cameraMatrix.proj = m_pCamera->getProjection(m_pSwapchain->m_extent.width / (float) m_pSwapchain->m_extent.height);
    
    m_pGraphic->m_cameraMatrix = m_cameraMatrix;
    m_pGraphic->m_misc.camera  = m_pCamera->getPosition();
}

void App::draw(long iteration) {
    m_pGraphic->draw();
    if ( m_pWindow->checkResized()) {
        m_pGraphic->reset();
    }
}
