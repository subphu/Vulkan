//  Copyright © 2021 Subph. All rights reserved.
//

#include <array>

#include "app.h"
#include "helper.h"
#include "system.h"

#define WIDTH   600
#define HEIGHT  400
#define TEXSIZE 1024
#define WINDOW_X 50
#define WINDOW_Y 100

void App::run() {
    initWindow();
    initVulkan();
    m_pCamera = new Camera();
    mainLoop();
    
    VkInstance instance = System::instance().m_renderer->m_instance;
    vkDestroySurfaceKHR(instance, m_pGUIWindow->getSurface(), nullptr);
    
    m_pCompute->cleanup();
    m_pGraphic1->cleanup();
    m_pGraphic2->cleanup();
    m_pRenderer->cleanUp();
    m_pRenderWindow2->close();
    m_pRenderWindow1->close();
    m_pGUIWindow->close();
}


void App::initWindow() {
    LOG("App::initWindow");
    m_pGUIWindow = new Window();
    m_pGUIWindow->create(WIDTH/2, HEIGHT, "Interference 2D");
    m_pGUIWindow->setWindowPosition(WINDOW_X, WINDOW_Y);
    m_pGUIWindow->enableInput();
    m_pGUIWindow->settingUI();

    m_pRenderWindow1 = new Window();
    m_pRenderWindow1->create(WIDTH, HEIGHT, "Interference 2D");
    m_pRenderWindow1->setWindowPosition(WINDOW_X + WIDTH/2, WINDOW_Y);
    m_pRenderWindow1->enableInput();
    
    m_pRenderWindow2 = new Window();
    m_pRenderWindow2->create(WIDTH, HEIGHT, "Interference 1D");
    m_pRenderWindow2->setWindowPosition(WINDOW_X + WIDTH + WIDTH/2, WINDOW_Y);
    m_pRenderWindow2->enableInput();
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
    m_pRenderWindow1->createSurface(m_pRenderer->m_instance);
    m_pRenderWindow2->createSurface(m_pRenderer->m_instance);
    
    m_pRenderer->pickPhysicalDevice(m_pGUIWindow->getSurface());
    m_pRenderer->createLogicalDevice();
    m_pRenderer->createDeviceQueue();
    
    m_pRenderer->createCommander();

    createPipelineCompute();
    createPipelineGraphic();
}

void App::createPipelineCompute() {
    LOG("App::createPipelineCompute");
    m_pCompute = new ComputeInterference();
    m_pCompute->setup(TEXSIZE);
    m_pCompute->dispatch();
}

void App::createPipelineGraphic() {
    LOG("App::createPipelineGraphic");
    m_pGraphic1 = new GraphicMain();
    m_pGraphic1->m_pInterBuffer = m_pCompute->getOutputBuffer();
    m_pGraphic1->setup(m_pRenderWindow1);
    m_pGraphic1->m_misc.buffSize = TEXSIZE;
    
    m_pGraphic2 = new GraphicMain();
    m_pGraphic2->m_pInterBuffer = m_pCompute->getOutputBuffer();
    m_pGraphic2->setup(m_pRenderWindow2);
    m_pGraphic2->m_misc.buffSize = TEXSIZE;
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
    }
    vkDeviceWaitIdle(m_pRenderer->m_device);
}

void App::update(long iteration) {
    m_pRenderWindow1->pollEvents();
    glm::vec3 movement = getMovement(m_pRenderWindow1) + getMovement(m_pRenderWindow2);
    m_pCamera->move(movement);
    
    m_cameraMatrix.model = glm::mat4(1.0f);
    m_cameraMatrix.model = glm::translate(m_cameraMatrix.model, glm::vec3(0.f, -0.5f, 0.f));
    m_cameraMatrix.model = glm::scale(m_cameraMatrix.model, glm::vec3(3.0));
    m_cameraMatrix.view = m_pCamera->getViewMatrix();
    m_cameraMatrix.proj = m_pCamera->getProjection((float) WIDTH / HEIGHT);
    
    m_pGraphic1->m_cameraMatrix = m_cameraMatrix;
    m_pGraphic1->m_misc.camera  = m_pCamera->getPosition();
    m_pGraphic2->m_cameraMatrix = m_cameraMatrix;
    m_pGraphic2->m_misc.camera  = m_pCamera->getPosition();
}

void App::draw(long iteration) {
    m_pGraphic1->draw();
    m_pGraphic2->draw();
    
    if ( m_pRenderWindow1->checkResized()) m_pGraphic1->reset();
    if ( m_pRenderWindow2->checkResized()) m_pGraphic2->reset();
}

glm::vec3 App::getMovement(Window* pWindow) {
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
    return movement;
    
}
