//  Copyright © 2020 Subph. All rights reserved.
//

#include "window.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define WINDOW_FAILED_MESSAGE "Failed to create GLFW window"
#define EXIT_MESSAGE          "Window closed"

Window::Window() { }
Window::~Window() { }

void Window::create(GLuint width, GLuint height, const char* name) {
    m_name = name;
    setSize({ (int)width, (int)height });
    
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    m_window = glfwCreateWindow(m_size.width, m_size.height, m_name, nullptr, nullptr);
    if (m_window == NULL) exitFailure(WINDOW_FAILED_MESSAGE);
    
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, resizeCallback);
}

void Window::resizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->notifyResize();
}

void Window::createSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void Window::close() {
    std::cout << EXIT_MESSAGE << std::endl;
    glfwSetWindowShouldClose(m_window, true);
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Window::exitFailure(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    exit(EXIT_FAILURE);
}

std::vector<const char*> Window::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // validation layers
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

void Window::pollEvents() {
    glfwPollEvents();
}

bool Window::isOpen() { return !glfwWindowShouldClose(m_window); }

float Window::getRatio() { return m_ratio; }

Size<int> Window::getSize () {
    int width, height;
    glfwGetWindowSize(m_window, &width, &height);
    setSize({ width, height });
    return m_size;
}

void Window::setSize(Size<int> size) {
    m_size = size;
    calcRatio(size);
}

void Window::notifyResize() {
    m_resized = true;
}

bool Window::checkResized() {
    bool result = m_resized;
    m_resized = false;
    return result;
}

void Window::calcRatio(Size<int> size) {
    m_ratio = (float)size.width / (float)size.height;
}

void Window::settingUI() {
//    IMGUI_CHECKVERSION();
//    ImGui::CreateContext();
//    ImGuiIO& io = ImGui::GetIO(); (void)io;
////    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
////    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
////
////    Setup Dear ImGui style
////    ImGui::StyleColorsDark();
//    ImGui::StyleColorsClassic();
//
////    Setup Platform/Renderer bindings
//    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
////    ImGui_ImplOpenGL3_Init(GLSL_VERSION);
}
