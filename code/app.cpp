//  Copyright © 2021 Subph. All rights reserved.
//

#include <array>

#include "app.h"
#include "helper.h"
#include "system.h"

#define WIDTH   800
#define HEIGHT  600
#define TEXSIZE 256
#define WINDOW_X 50
#define WINDOW_Y 100

void App::run() {
    initWindow();
    initVulkan();
    m_pCamera = new Camera();
//    mainLoop();
    mainLoopFps();
    
    VkInstance instance = System::instance().m_renderer->m_instance;
    vkDestroySurfaceKHR(instance, m_pGUIWindow->getSurface(), nullptr);
    
    for (GraphicMain*         graphic : m_pGraphics) graphic->cleanup();
    for (ComputeInterference* compute : m_pComputes) compute->cleanup();
    for (Window* window : m_pRenderWindows) window->close();
    
    m_pRenderer->cleanUp();
    m_pGUIWindow->close();
}


void App::initVulkan() {
    LOG("App::initVulkan");
    System &system = System::instance();
    m_pRenderer = new Renderer();
    system.m_renderer  = m_pRenderer;
    
    m_pRenderer->setupValidation(IS_DEBUG);
    m_pRenderer->createInstance(Window::getRequiredExtensions());
    m_pRenderer->createDebugMessenger();
    m_pRenderer->setupDeviceExtensions();
    
    m_pGUIWindow->createSurface(m_pRenderer->m_instance);
    
    for (Window* window : m_pRenderWindows)
        window->createSurface(m_pRenderer->m_instance);
    
    m_pRenderer->pickPhysicalDevice(m_pGUIWindow->getSurface());
    m_pRenderer->createLogicalDevice();
    m_pRenderer->createDeviceQueue();
    
    m_pRenderer->createCommander();

    createPipelineCompute();
    createPipelineGraphic();
}


void App::initWindow() {
    LOG("App::initWindow");
    m_pGUIWindow = new Window();
    m_pGUIWindow->create(WIDTH/2, HEIGHT, "Parameters");
    m_pGUIWindow->setWindowPosition(WINDOW_X, WINDOW_Y);
    m_pGUIWindow->enableInput();

    Window* renderWindow1 = new Window();
    renderWindow1->create(WIDTH, HEIGHT, "Interference 1D");
    renderWindow1->setWindowPosition(WINDOW_X + WIDTH/2, WINDOW_Y);
    renderWindow1->enableInput();
    m_pRenderWindows.push_back(renderWindow1);
    
//    Window* renderWindow2 = new Window();
//    renderWindow2->create(WIDTH, HEIGHT, "Manual");
//    renderWindow2->setWindowPosition(WINDOW_X + WIDTH + WIDTH/2, WINDOW_Y);
//    renderWindow2->enableInput();
//    m_pRenderWindows.push_back(renderWindow2);
}

void App::createPipelineCompute() {
    LOG("App::createPipelineCompute");
    ComputeInterference* compute1D = new ComputeInterference();
    compute1D->setShaderPath("shaders/SPV/interference1d.comp.spv");
    compute1D->setup(TEXSIZE);
    compute1D->dispatch();
    m_pComputes.push_back(compute1D);
    
//    ComputeInterference* compute2D = new ComputeInterference();
//    compute2D->setShaderPath("shaders/SPV/interference2d.comp.spv");
//    compute2D->setup(TEXSIZE);
//    compute2D->dispatch();
//    m_pComputes.push_back(compute2D);
}

void App::createPipelineGraphic() {
    LOG("App::createPipelineGraphic");
    
    GraphicMain* graphic1 = new GraphicMain();
    graphic1->setShaders({
        new Shader("shaders/SPV/main1d.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
        new Shader("shaders/SPV/main1d.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
    });
    graphic1->setInterBuffer(m_pComputes[0]->getOutputBuffer());
    graphic1->setup(m_pRenderWindows[0]);
    graphic1->m_misc.buffSize = TEXSIZE;
    m_pGraphics.push_back(graphic1);
    
//    GraphicMain* graphic2 = new GraphicMain();
//    graphic2->setShaders({
//        new Shader("shaders/SPV/manual.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
//        new Shader("shaders/SPV/manual.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
//    });
//    graphic2->setInterBuffer(m_pComputes[1]->getOutputBuffer());
//    graphic2->setup(m_pRenderWindows[1]);
//    graphic2->m_misc.buffSize = TEXSIZE;
//    m_pGraphics.push_back(graphic2);
}

void App::update(long iteration) {
    for (Window* window : m_pRenderWindows) {
        window->pollEvents();
        moveView(window);
    }
    
    m_cameraMatrix.model = glm::mat4(1.0f);
    m_cameraMatrix.model = glm::translate(m_cameraMatrix.model, glm::vec3(0.f, -0.5f, 0.f));
    m_cameraMatrix.model = glm::scale(m_cameraMatrix.model, glm::vec3(3.0));
    m_cameraMatrix.view = m_pCamera->getViewMatrix();
    m_cameraMatrix.proj = m_pCamera->getProjection((float) WIDTH / HEIGHT);
    
    for (GraphicMain* graphic : m_pGraphics) {
        graphic->m_cameraMatrix      = m_cameraMatrix;
        graphic->m_misc.viewPosition = m_pCamera->getPosition();
    }
}

void App::draw(long iteration) {
    auto start = Time::now();
    for (GraphicMain* graphic : m_pGraphics) {
        graphic->draw();
        duration1 += TimeDif(Time::now() - start).count();
    }
    
    for (uint i = 0; i < m_pRenderWindows.size(); i++) {
        if (m_pRenderWindows[i]->checkResized())
            m_pGraphics[i]->reset();
    }
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

void App::moveViewLock(Window* pWindow) {
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

void App::mainLoopFps() {
    auto lastTime = Time::now();
    float pastTime = 0;
    long iteration = 0;
    while (m_pGUIWindow->isOpen()) {
        iteration++;
        update(iteration);
        draw(iteration);
        
        pastTime += TimeDif(Time::now() - lastTime).count();
        lastTime = Time::now();

        if (pastTime > 1.0) {
            PRINTLN2("duration1", duration1);
            PRINTLN2("duration2", duration2);
            LOG(iteration);
            duration1 = 0;
            duration2 = 0;
            iteration = 0;
            pastTime = 0;
        }
    }
    vkDeviceWaitIdle(m_pRenderer->m_device);
    
}

void App::mainLoop() {
    auto lastTime = Time::now();
    float frameDelay = 1.f/60.f;
    float lag = frameDelay;
    
    long iteration = 0;
    while (m_pGUIWindow->isOpen()) {
        iteration++;
        update(iteration);
        lag -= frameDelay;
        
        while (lag < frameDelay) {
            draw(iteration);
        
            lag += TimeDif(Time::now() - lastTime).count();
            lastTime = Time::now();
        }
        
        if (iteration % 60 == 0) {
            PRINTLN2("duration1", duration1);
            PRINTLN2("duration2", duration2);
            LOG("");
            duration1 = 0;
            duration2 = 0;
        }
    }
    vkDeviceWaitIdle(m_pRenderer->m_device);
}
