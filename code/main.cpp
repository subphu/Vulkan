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
        cleanup();
    }

private:
    Window window;
    Camera camera;
    
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    size_t currentFrame = 0;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    
    bool framebufferResized = false;
    
    uint32_t mipLevels = 1;
    VkImage textureImage;
    VkImageView textureImageView;
    VkDeviceMemory textureImageMemory;
    VkSampler textureSampler;
    
    VkImage depthImage;
    VkImageView depthImageView;
    VkDeviceMemory depthImageMemory;
    
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
        
        surface        = renderer->m_surface;
        device         = renderer->m_device;
        physicalDevice = renderer->m_physicalDevice;
        graphicsQueue  = renderer->m_graphicQueue;
        presentQueue   = renderer->m_presentQueue;
        commandPool    = renderer->m_commandPool;
        
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
        
        swapChain = renderer->m_swapChain;
        swapChainExtent = renderer->m_swapChainExtent;
        swapChainImages = renderer->m_swapChainImages;
        swapChainImageFormat = renderer->m_swapChainImageFormat;
        swapChainImageViews = renderer->m_swapChainImageViews;
        renderPass = renderer->m_renderPass;
        descriptorSetLayout = renderer->m_descriptorSetLayout;
        pipelineLayout = renderer->m_pipelineLayout;
        graphicsPipeline = renderer->m_graphicsPipeline;
        
        renderer->createDepthResources();
        renderer->createFramebuffer();
        
        depthImage = renderer->m_depthImage;
        depthImageMemory = renderer->m_depthImageMemory;
        depthImageView = renderer->m_depthImageView;
        swapChainFramebuffers = renderer->m_swapChainFramebuffers;
        
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
        
        uniformBuffers = renderer->m_uniformBuffers;
        uniformBuffersMemory = renderer->m_uniformBuffersMemory;
        descriptorPool = renderer->m_descriptorPool;
        descriptorSets = renderer->m_descriptorSets;
        commandBuffers = renderer->m_commandBuffers;
        imageAvailableSemaphores = renderer->m_imageAvailableSemaphores;
        renderFinishedSemaphores = renderer->m_renderFinishedSemaphores;
        inFlightFences = renderer->m_inFlightFences;
        imagesInFlight = renderer->m_imagesInFlight;
        
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
        ubo.proj = camera.getProjection(swapChainExtent.width / (float) swapChainExtent.height);

        void* data;
        vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
    }
    
    void recreateSwapChain() {
        vkDeviceWaitIdle(device);

        cleanupSwapChain();
        
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

        vkDeviceWaitIdle(device);
    }

    void drawFrame() {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        
        uint32_t imageIndex;
        VkResult result =  vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
        
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];
        
        updateUniformBuffer(imageIndex);
        
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphore[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphore;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

        VkSemaphore signalSemaphors[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphors;
        
        vkResetFences(device, 1, &inFlightFences[currentFrame]);
        
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphors;

        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);
        
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.checkResized()) {
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
        
        currentFrame = (currentFrame + 1) % MAX_FRAME_IN_FLIGHT;
    }
    
    void cleanupSwapChain() {
        vkDestroyImageView(device, depthImageView, nullptr);
        vkDestroyImage(device, depthImage, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);
        
        for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
            vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
        }
        
        vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
        
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            vkDestroyImageView(device, swapChainImageViews[i], nullptr);
        }
        
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }
        
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    }

    void cleanup() {
        cleanupSwapChain();
        
//        vkDestroySampler(device, textureSampler, nullptr);
//        vkDestroyImageView(device, textureImageView, nullptr);
//        vkDestroyImage(device, textureImage, nullptr);
//        vkFreeMemory(device, textureImageMemory, nullptr);
        
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        
        model.cleanup();
        
        for (size_t i = 0; i < MAX_FRAME_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
        vkDestroyCommandPool(device, commandPool, nullptr);
        
        vkDestroyDevice(device, nullptr);
        
//        if (enableValidationLayers) {
//            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
//        }
//        vkDestroySurfaceKHR(instance, surface, nullptr);
//        vkDestroyInstance(instance, nullptr);

        window.close();
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
