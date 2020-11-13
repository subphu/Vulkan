//  Copyright © 2020 Subph. All rights reserved.
//

#pragma once
#define GLFW_INCLUDE_VULKAN

#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

template<typename T> struct Size {
    T width, height;
};

class Window {
    
public:
    Window();
    ~Window();
    
    void create(GLuint width, GLuint height, const char* name);
    void createSurface(VkInstance instance, VkSurfaceKHR* surface);
    void close();
    
    bool isOpen();
    float getRatio();
    Size<int> getSize();
    std::vector<const char*> getRequiredExtensions();
    
    void setSize(Size<int> size);
    void notifyResize();
    bool checkResized();
    
    void pollEvents();
    void settingUI();
    
private:
    GLFWwindow* m_window;
    Size<int> m_size = { 900, 900 };
    bool m_resized = false;
    float m_ratio = 1.f;
    float m_frameTime = 1.f/60.f;
    const char* m_name = "Vulkan";

    void calcRatio(Size<int> size);
    void exitFailure(std::string message);
    static void resizeCallback(GLFWwindow* window, int width, int height);

};

