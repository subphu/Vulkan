#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>
#include <set>
#include <optional>
#include <algorithm>
#include <fstream>
#include <array>
#include <unordered_map>

#include "window/window.h"
#include "renderer/renderer.h"
#include "camera/camera.h"
#include "libraries/stb_image/stb_image.h"
#include "libraries/tiny_obj_loader/tiny_obj_loader.h"
#include "mesh/mesh.h"

const uint32_t WIDTH = 1200;
const uint32_t HEIGHT = 800;
const int MAX_FRAME_IN_FLIGHT = 2;

const std::string MODEL_PATH = "models/viking_room/viking_room.obj";
const std::string TEXTURE_PATH = "textures/rustediron/rustediron_albedo.png";


struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


class App {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        model.cleanup();
        renderer->cleanUp();
        window.close();
    }

private:
    Window window;
    Camera camera;
    
    uint32_t mipLevels = 1;
    size_t currentFrame = 0;
    VkImage textureImage;
    VkImageView textureImageView;
    VkDeviceMemory textureImageMemory;
    VkSampler textureSampler;
    Renderer* renderer;
    Mesh model = Mesh();
    
    void initWindow() {
        window = Window();
        window.create(WIDTH, HEIGHT, "Vulkan");
        window.enableInput();
        camera = Camera();
    }
    
    
    void initVulkan() {
        renderer = new Renderer();
        renderer->setupValidation(IS_DEBUG);
        renderer->createInstance(Window::getRequiredExtensions());
        renderer->createDebugMessenger();
        renderer->setSurface(window.createSurface(renderer->m_instance));
        renderer->setDeviceExtensions();
        renderer->pickPhysicalDevice();
        renderer->createLogicalDevice();
        renderer->createDeviceQueue();
        renderer->createCommandPool();
        
        renderer->createSwapChain(window.getSize());
        renderer->createImageViews();
        renderer->createRenderPass();
        renderer->createDescriptorSetLayout();
        renderer->createPipelineLayout();
        
        VkPipelineShaderStageCreateInfo shaderStages[] = {
            renderer->createShaderStageInfo("shaders/vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
            renderer->createShaderStageInfo("shaders/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        };
        
        renderer->createGraphicsPipeline(shaderStages, model.createVertexInputInfo());
        
        renderer->createDepthResources();
        renderer->createFramebuffer();
        
        int texWidth, texHeight, texChannels;
        stbi_uc* texture = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        uint32_t maxMipLevels = MaxMapLevel(texWidth, texHeight);
        mipLevels = maxMipLevels;

        textureImage = renderer->createTextureImage(texture, texWidth, texHeight, texChannels, maxMipLevels);

        textureImageView = renderer->createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, maxMipLevels);

        textureSampler = renderer->createTextureSampler(maxMipLevels);
        
        model.setRenderer(renderer);
        model.createCube();
        model.createVertexBuffer();
        model.createIndexBuffer();
        
        renderer->createUniformBuffers(sizeof(UniformBufferObject));
        renderer->createDescriptorPool();
        renderer->createDescriptorSets(sizeof(UniformBufferObject), textureImageView, textureSampler);
        renderer->createCommandBuffers(model.vertexBuffer, model.indexBuffer, static_cast<uint32_t>(model.m_indices.size()));
        renderer->createSyncObjects();
        
    }
    
    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        
        UniformBufferObject ubo{};
        ubo.model = glm::mat4(1.0f);
        ubo.model = glm::rotate(ubo.model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        ubo.model = glm::rotate(ubo.model, glm::radians(210.0f) + time * glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = camera.getViewMatrix();
        ubo.proj = camera.getProjection(renderer->m_swapChainExtent.width / (float) renderer->m_swapChainExtent.height);

        renderer->updateUniformBuffer(&ubo, sizeof(ubo), currentImage);
    }
    
    void recreateSwapChain() {
        vkDeviceWaitIdle(renderer->m_device);

        renderer->cleanUpSwapChain();
        
        renderer->createSwapChain(window.getSize());
        renderer->createImageViews();
        renderer->createRenderPass();
        renderer->createPipelineLayout();
        
        VkPipelineShaderStageCreateInfo shaderStages[] = {
            renderer->createShaderStageInfo("shaders/vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
            renderer->createShaderStageInfo("shaders/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        };
        
        renderer->createGraphicsPipeline(shaderStages, model.createVertexInputInfo());
        
        renderer->createDepthResources();
        renderer->createFramebuffer();
        renderer->createUniformBuffers(sizeof(UniformBufferObject));
        renderer->createDescriptorPool();
        renderer->createDescriptorSets(sizeof(UniformBufferObject), textureImageView, textureSampler);
        renderer->createCommandBuffers(model.vertexBuffer, model.indexBuffer, static_cast<uint32_t>(model.m_indices.size()));
    }

    void mainLoop() {
        while (window.isOpen()) {
            window.pollEvents();
            glm::vec3 movement = glm::vec3(0.f, 0.f, 0.f);
            movement.x += window.getKeyState(key_d) - window.getKeyState(key_a);
            movement.y += window.getKeyState(key_q) - window.getKeyState(key_e);
            movement.z += window.getKeyState(key_w) - window.getKeyState(key_s);
            
            if (window.getMouseBtnState(mouse_btn_left)) {
                glm::vec2 cursorOffset = window.getCursorOffset();
                movement.x += cursorOffset.x * 0.2f;
                movement.y += cursorOffset.y * 0.2f;
            }
            movement.z += window.getScrollOffset().y;
            
            window.resetInput();
            
            camera.move(movement);
            drawFrame();
        }

        vkDeviceWaitIdle(renderer->m_device);
    }

    void drawFrame() {
        vkWaitForFences(renderer->m_device, 1, &renderer->m_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        
        uint32_t imageIndex;
        VkResult result =  vkAcquireNextImageKHR(renderer->m_device, renderer->m_swapChain, UINT64_MAX, renderer->m_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        
        if (renderer->m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(renderer->m_device, 1, &renderer->m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        
        renderer->m_imagesInFlight[imageIndex] = renderer->m_inFlightFences[currentFrame];
        
        updateUniformBuffer(imageIndex);
        
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphore[] = { renderer->m_imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &renderer->m_commandBuffers[imageIndex];

        VkSemaphore signalSemaphors[] = { renderer->m_renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphors;
        
        vkResetFences(renderer->m_device, 1, &renderer->m_inFlightFences[currentFrame]);
        
        if (vkQueueSubmit(renderer->m_graphicQueue, 1, &submitInfo, renderer->m_inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphors;

        VkSwapchainKHR swapChains[] = { renderer->m_swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(renderer->m_presentQueue, &presentInfo);
        
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.checkResized()) {
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
        
        currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
    }
    
};

int main() {
    App app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}
