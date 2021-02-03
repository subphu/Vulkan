//  Copyright © 2020 Subph. All rights reserved.
//

#pragma once

#include <array>

#include "../common.h"
#include "../helper.h"

class Renderer {
    
public:
    Renderer();
    ~Renderer();
    
    void cleanUp();
    
    std::vector<const char*> m_validationLayers = {};
    VkDebugUtilsMessengerCreateInfoEXT m_debugInfo = {};
    void setupValidation(bool isEnable);
    
    std::vector<const char*> m_deviceExtensions = {};
    void setDeviceExtensions();
    
    VkInstance m_instance = VK_NULL_HANDLE;
    void createInstance(std::vector<const char*> extensions);
    
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    void createDebugMessenger();
    
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    void setSurface(VkSurfaceKHR* pSurface);
    
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
    std::vector<VkPresentModeKHR> m_presentModes;
    uint32_t m_graphicFamilyIndex = 0;
    uint32_t m_presentFamilyIndex = 0;
    void pickPhysicalDevice();
    
    VkDevice m_device = VK_NULL_HANDLE;
    void createLogicalDevice();
    
    VkQueue m_graphicQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    void createDeviceQueue();
    
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    void createCommandPool();
    
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    void createSwapChain(Size<int> windowSize);
    
    std::vector<VkImageView> m_swapChainImageViews;
    void createImageViews();
    
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    void createRenderPass();
    
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    void createDescriptorSetLayout();
    
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    void createPipelineLayout();
    
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    void createGraphicsPipeline(VkPipelineShaderStageCreateInfo* shaderStages, VkPipelineVertexInputStateCreateInfo* vertexInputInfo);
    
    uint32_t m_mipLevels = 1;
    VkImage m_depthImage = VK_NULL_HANDLE;
    VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;
    VkImageView m_depthImageView = VK_NULL_HANDLE;
    void createDepthResources();
    
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    void createFramebuffer();
    
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    void createUniformBuffers(VkDeviceSize bufferSize);
    
    VkDescriptorPool m_descriptorPool;
    void createDescriptorPool();
    
    std::vector<VkDescriptorSet> m_descriptorSets;
    void createDescriptorSets(VkDeviceSize uniformBufferSize, VkImageView textureImageView, VkSampler textureSampler);
    
    std::vector<VkCommandBuffer> m_commandBuffers;
    void createCommandBuffers(VkBuffer vertexBuffer, VkBuffer indexBuffer, uint32_t indexSize);
    
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    void createSyncObjects();
    
public:
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    
    VkShaderModule createShaderModule(const std::vector<char> & code);
    VkPipelineShaderStageCreateInfo createShaderStageInfo(const std::string& filename, VkShaderStageFlagBits stage);
    
    VkBuffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
    VkDeviceMemory allocateBufferMemory(VkBuffer& buffer, VkDeviceSize size, uint32_t memoryTypeIndex);
    VkMemoryRequirements getBufferMemoryRequirements(VkBuffer& buffer);
    VkMemoryRequirements getImageMemoryRequirements(VkImage& image);
    
    uint32_t findMemoryTypeIdx(uint32_t typeFilter, VkMemoryPropertyFlags flags);
    
    VkImage createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
    VkDeviceMemory bindImageMemory(VkImage image, VkMemoryPropertyFlags properties);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
    
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    
    VkImage createTextureImage(unsigned char* rawTexture, int width, int height, int channels, uint32_t mipLevels);
    
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t width, int32_t height, uint32_t mipLevels);
    VkSampler createTextureSampler(uint32_t mipLevels);
private:
    
    
};
