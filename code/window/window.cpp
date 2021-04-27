//  Copyright © 2020 Subph. All rights reserved.
//

#include "window.h"

#include "../libraries/imgui/imgui.h"
#include "../libraries/imgui/imgui_impl_glfw.h"
#include "../libraries/imgui/imgui_impl_opengl3.h"

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

VkSurfaceKHR Window::getSurface() { return m_surface; }
VkSurfaceKHR Window::createSurface(VkInstance instance) {
    VkResult result = glfwCreateWindowSurface(instance, m_window, nullptr, &m_surface);
    CHECK_VKRESULT(result, "failed to create window surface!");
    return m_surface;
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

bool Window::isOpen() { return !glfwWindowShouldClose(m_window); }

float Window::getRatio() { return m_ratio; }

Size<int> Window::getFrameSize () {
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    setSize({ width, height });
    return m_size;
}

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

void Window::enableInput() {
    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(m_window, this->mouseButtonCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    resetInput();
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->setMouseButton(button, action);
}

void Window::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->setScroll(xoffset, yoffset);
}

void Window::setWindowPosition(uint x, uint y) {
    glfwSetWindowPos(m_window, x, y);
}

void Window::setMouseButton(int button, int action) {
    m_mouseBtn[mouse_btn_left] = button == GLFW_MOUSE_BUTTON_LEFT ?
        action == GLFW_PRESS : m_mouseBtn[mouse_btn_left];
    m_mouseBtn[mouse_btn_right] = button == GLFW_MOUSE_BUTTON_LEFT ?
        action == GLFW_PRESS : m_mouseBtn[mouse_btn_right];
}

void Window::setScroll(double xoffset, double yoffset) {
    m_scrollOffset = glm::vec2(xoffset, yoffset);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::resetInput() {
    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    glm::vec2 cursorPos = glm::vec2(x, -y);
    m_cursorOffset = m_cursorPos - cursorPos;
    m_cursorPos = cursorPos;
    m_scrollOffset = glm::vec2(0, 0);
}

bool Window::getKeyState(int key) { return glfwGetKey(m_window, key); }
bool Window::getMouseBtnState(int idx) { return m_mouseBtn[idx]; }
glm::vec2 Window::getCursorPosition() { return m_cursorPos; }
glm::vec2 Window::getCursorOffset() { return m_cursorOffset; }
glm::vec2 Window::getScrollOffset() { return m_scrollOffset; }

glm::vec2 Window::getCursorMovement() {
    if (!getMouseBtnState(mouse_btn_left)) {
        resetInput();
        return glm::vec2(0, 0);
    }
    glm::vec2 cursorPos = m_cursorPos;
    if (cursorPos.x == 0 && cursorPos.y == 0) {
        resetInput();
        return glm::vec2(0, 0);
    }
    glm::vec2 lastPos = cursorPos;
    resetInput();
    cursorPos = m_cursorPos;
    glm::vec2 move = glm::vec2(cursorPos.x - lastPos.x, cursorPos.y - lastPos.y);
    return glm::clamp(move, glm::vec2(-99, -99), glm::vec2(99, 99));
}

void Window::settingUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//
//    Setup Dear ImGui style
//    ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

//    Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
//    ImGui_ImplOpenGL3_Init(GLSL_VERSION);
}
