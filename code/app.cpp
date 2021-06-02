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
    mainLoop();
    cleanup();
}

void App::cleanup() {
    m_pSettings->cleanup();
    m_pGraphicMain->cleanup();
    m_pComputeInterference->cleanup();
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

    createPipelineCompute();
    createPipelineGraphic();
    createGUI();
}

void App::createGUI() {
    m_pSettings = new Settings();
    m_pSettings->setWindow(m_pWindow);
    m_pSettings->initGUI(m_pGraphicMain->m_pSwapchain->m_renderPass);
    System::Instance().m_pSettings = m_pSettings;
}

void App::initWindow() {
    LOG("App::initWindow");
    m_pWindow = new Window();
    m_pWindow->create(WIDTH, HEIGHT, "Vulkan");
    m_pWindow->setWindowPosition(WINDOW_X, WINDOW_Y);
    m_pWindow->enableInput();
}

void App::createPipelineCompute() {
    LOG("App::createPipelineCompute");
    ComputeInterference* compute1D = new ComputeInterference();
    compute1D->setShaderPath("shaders/interference1d.comp.spv");
    compute1D->setup(TEXSIZE);
    compute1D->dispatch();
    
//    ComputeInterference* compute2D = new ComputeInterference();
//    compute2D->setShaderPath("shaders/interference2d.comp.spv");
//    compute2D->setup(TEXSIZE);
//    compute2D->dispatch();
    
    m_pComputeInterference = compute1D;
}

void App::createPipelineGraphic() {
    LOG("App::createPipelineGraphic");
    
    GraphicMain* graphic1 = new GraphicMain();
    graphic1->setShaders({
        new Shader("shaders/main1d.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        new Shader("shaders/main1d.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    });
    graphic1->setShaderCubemap({
        new Shader("shaders/skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        new Shader("shaders/skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    });
    graphic1->setInterBuffer(m_pComputeInterference->getOutputBuffer());
    graphic1->setup(m_pWindow);
    graphic1->m_misc.buffSize = TEXSIZE;
    
//    GraphicMain* graphic2 = new GraphicMain();
//    graphic2->setShaders({
//        new Shader("shaders/manual.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
//        new Shader("shaders/manual.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
//    });
//    graphic2->setInterBuffer(m_pComputes[1]->getOutputBuffer());
//    graphic2->setup(m_pRenderWindows[1]);
//    graphic2->m_misc.buffSize = TEXSIZE;
    
    m_pGraphicMain = graphic1;
}

void App::update(long iteration) {
    Settings* settings = System::Settings();
    if (settings->LockFocus) moveViewLock(m_pWindow);
    else                     moveView(m_pWindow);
    
    m_cameraMatrix.model = glm::mat4(1.0f);
    m_cameraMatrix.model = glm::translate(m_cameraMatrix.model, glm::vec3(0.f, -0.5f, 0.f));
    m_cameraMatrix.model = glm::scale(m_cameraMatrix.model, glm::vec3(3.0));
    m_cameraMatrix.view = m_pCamera->getViewMatrix();
    m_cameraMatrix.proj = m_pCamera->getProjection((float) WIDTH / HEIGHT);
    
    m_pGraphicMain->m_cameraMatrix      = m_cameraMatrix;
    m_pGraphicMain->m_misc.viewPosition = m_pCamera->getPosition();
}

void App::draw(long iteration) {
    auto start = Time::now();
    m_pGraphicMain->draw();
    duration1 += TimeDif(Time::now() - start).count();
    
    if (m_pWindow->checkResized())
        m_pGraphicMain->reset();
}

void App::moveView(Window* pWindow) {
    m_pCamera->setLockFocus(System::Settings()->LockFocus);
    pWindow->pollEvents();
    glm::vec3 movement = glm::vec3(0.f, 0.f, 0.f);
    movement.x += pWindow->getKeyState(key_d) - pWindow->getKeyState(key_a);
    movement.y += pWindow->getKeyState(key_q) - pWindow->getKeyState(key_e);
    movement.z += pWindow->getKeyState(key_w) - pWindow->getKeyState(key_s);
    m_pCamera->move(movement);
    
    glm::vec2 delta = pWindow->getCursorMovement();
    m_pCamera->turn(delta * glm::vec2(4.0, 4.0));
}

void App::moveViewLock(Window* pWindow) {
    m_pCamera->setLockFocus(System::Settings()->LockFocus);
    pWindow->pollEvents();
    glm::vec3 movement = glm::vec3(0.f, 0.f, 0.f);
    movement.x += pWindow->getKeyState(key_d) - pWindow->getKeyState(key_a);
    movement.y += pWindow->getKeyState(key_q) - pWindow->getKeyState(key_e);
    movement.z += pWindow->getKeyState(key_w) - pWindow->getKeyState(key_s);
    
    if (pWindow->getMouseBtnState(mouse_btn_left)) {
        glm::vec2 cursorOffset = pWindow->getCursorOffset();
        movement.x += cursorOffset.x * 0.2f;
        movement.y += cursorOffset.y * 0.2f;
    }
    movement.z += pWindow->getScrollOffset().y;
    pWindow->resetInput();
    
    m_pCamera->move(movement);    
}

void App::mainLoop() {
    LOG("App::mainLoop");
    auto lastTime = Time::now();
    float frameDelay = 1.f/60.f;
    float lag = frameDelay;
    
    long iteration = 0;
    while (m_pWindow->isOpen()) {
        bool lockFps = System::Settings()->LockFPS;
        
        iteration++;
        m_pSettings->drawGUI();
        update(iteration);
        
        if (!lockFps) {
            draw(iteration);
            continue;
        }
        
        lag -= frameDelay;
        while (lag < frameDelay) {
            draw(iteration);
            lag += TimeDif(Time::now() - lastTime).count();
            lastTime = Time::now();
        }
    }
    vkDeviceWaitIdle(m_pRenderer->getDevice());
}
