//  Copyright © 2021 Subph. All rights reserved.
//

#include <array>

#include "swapchain.h"
#include "../helper.h"
#include "../system.h"

Swapchain::~Swapchain() { }
Swapchain::Swapchain() {
    System &system   = System::instance();
    m_device         = system.m_renderer->m_device;
    m_physicalDevice = system.m_renderer->m_physicalDevice;
}

void Swapchain::cleanup() {
    if (m_totalFrame == 0) return;
    
    for (size_t i = 0; i < m_inFlightFences.size(); i++) {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
    }
    
    vkFreeCommandBuffers(m_device, System::instance().m_renderer->getCommander()->m_commandPool, UINT32(m_commandBuffers.size()), m_commandBuffers.data());
    
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    
    for (size_t i = 0; i < m_totalFrame; i++)
        m_uniformBuffers[i]->cleanup();
    
    for (size_t i = 0; i < m_framebuffers.size(); i++)
        vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
    
    m_depthResource->cleanup();
    m_pipelineGraphics->cleanup();
    
    for (size_t i = 0; i < m_totalFrame; i++)
        m_swapchainImageResources[i]->cleanupImageView();
    
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    m_totalFrame = 0;
}

void Swapchain::setup(Size<int> size) {
    VkPhysicalDevice physicalDevice = m_physicalDevice;
    
    System&   system   = System::instance();
    Renderer* renderer = system.getRenderer();
    
    VkSurfaceKHR       surface       = renderer->getSurface();
    VkSurfaceFormatKHR surfaceFormat = renderer->getSwapchainSurfaceFormat();
    VkPresentModeKHR   presentMode   = renderer->getSwapchainPresentMode();
    uint32_t graphicFamilyIndex = renderer->getGraphicQueueIndex();
    uint32_t presentFamilyIndex = renderer->getPresentQueueIndex();
    
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;
    
    VkExtent2D extent = ChooseSwapExtent(capabilities, size);
    
    VkSwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface          = surface;
    swapchainInfo.minImageCount    = imageCount;
    swapchainInfo.imageFormat      = surfaceFormat.format;
    swapchainInfo.imageColorSpace  = surfaceFormat.colorSpace;
    swapchainInfo.imageExtent      = extent;
    swapchainInfo.imageArrayLayers = 1;
    swapchainInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainInfo.preTransform     = capabilities.currentTransform;
    swapchainInfo.presentMode      = presentMode;
    swapchainInfo.clipped          = VK_TRUE;
    swapchainInfo.oldSwapchain     = VK_NULL_HANDLE;
    
    uint32_t queueFamilyIndices[] = { graphicFamilyIndex, presentFamilyIndex };
    if (graphicFamilyIndex != presentFamilyIndex) {
        swapchainInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        swapchainInfo.queueFamilyIndexCount = 2;
        swapchainInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
        swapchainInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        swapchainInfo.queueFamilyIndexCount = 0; // Optional
        swapchainInfo.pQueueFamilyIndices   = nullptr; // Optional
    }
    {
        m_swapchainInfo          = swapchainInfo;
        m_swapchainExtent        = extent;
        m_swapchainSurfaceFormat = surfaceFormat.format;
    }
}

void Swapchain::create() {
    LOG("createSwapchain");
    VkResult result = vkCreateSwapchainKHR(m_device, &m_swapchainInfo, nullptr, &m_swapchain);
    CHECK_VKRESULT(result, "failed to create swap chain!");
}

void Swapchain::createRenderPass() {
    LOG("createRenderPass");
    VkDevice device = m_device;
    
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format          = m_swapchainSurfaceFormat;
    colorAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format          = ChooseDepthFormat(m_physicalDevice);
    depthAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstSubpass    = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount  = UINT32(attachments.size());
    renderPassInfo.pAttachments     = attachments.data();
    renderPassInfo.subpassCount     = 1;
    renderPassInfo.pSubpasses       = &subpass;
    renderPassInfo.dependencyCount  = 1;
    renderPassInfo.pDependencies    = &dependency;
    
    VkRenderPass renderPass;
    VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
    CHECK_VKRESULT(result, "failed to create render pass!");
    
    { m_renderPass = renderPass; }
}

void Swapchain::createPipeline(std::vector<Shader*> shaders, VkPipelineVertexInputStateCreateInfo* vertexInputInfo) {
    VkExtent2D   swapchainExtent = m_swapchainExtent;
    VkRenderPass renderPass      = m_renderPass;
    VkDescriptorSetLayout descriptorSetLayout = m_descriptorSetLayout;
    
    PipelineGraphics* pipelineGraphics = new PipelineGraphics();
    pipelineGraphics->setShaders(shaders);
    pipelineGraphics->setVertexInputInfo(vertexInputInfo);

    pipelineGraphics->createPipelineLayout(descriptorSetLayout);
    pipelineGraphics->setupViewportInfo(swapchainExtent);
    
    pipelineGraphics->setupInputAssemblyInfo();
    pipelineGraphics->setupRasterizationInfo();
    pipelineGraphics->setupMultisampleInfo();
    pipelineGraphics->setupColorBlendInfo();
    pipelineGraphics->setupDepthStencilInfo();
    pipelineGraphics->setupDynamicInfo();
    pipelineGraphics->create(renderPass);
    
    { m_pipelineGraphics = pipelineGraphics; }
}

void Swapchain::createSwapchainImages() {
    LOG("createSwapchainImages");
    
    VkDevice       device                 = m_device;
    VkSwapchainKHR swapchain              = m_swapchain;
    VkFormat       swapchainSurfaceFormat = m_swapchainSurfaceFormat;
    
    uint32_t totalFrame = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &totalFrame, nullptr);
    std::vector<VkImage> swapchainImages(totalFrame);
    vkGetSwapchainImagesKHR(device, swapchain, &totalFrame, swapchainImages.data());
    
    std::vector<ResourceImage*> swapchainImageResources;
    for (size_t i = 0; i < totalFrame; i++) {
        ResourceImage *resourceImage = new ResourceImage();
        resourceImage->setupForSwapchain(swapchainImages[i], swapchainSurfaceFormat);
        resourceImage->createForSwapchain();
        swapchainImageResources.push_back(resourceImage);
    }
    {
        m_totalFrame = totalFrame;
        m_swapchainImageResources = swapchainImageResources;
    }
}

void Swapchain::createDepthResources() {
    LOG("createDepthResources");
    m_depthResource = new ResourceImage();
    m_depthResource->setupForDepth({m_swapchainExtent.width, m_swapchainExtent.height}, m_mipLevels);
    m_depthResource->create();
}

void Swapchain::createFramebuffer() {
    LOG("createFramebuffer");
    VkDevice       device          = m_device;
    VkRenderPass   renderPass      = m_renderPass;
    VkExtent2D     swapchainExtent = m_swapchainExtent;
    uint32_t       totalFrame      = m_totalFrame;
    ResourceImage* depthResource   = m_depthResource;
    std::vector<ResourceImage*> swapchainImageResources = m_swapchainImageResources;
    
    std::vector<VkFramebuffer> framebuffers(totalFrame);
    for (size_t i = 0; i < totalFrame; i++) {
        std::array<VkImageView, 2> attachments = {swapchainImageResources[i]->getImageView(), depthResource->getImageView()};
        
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = renderPass;
        framebufferInfo.attachmentCount = UINT32(attachments.size());
        framebufferInfo.pAttachments    = attachments.data();
        framebufferInfo.width           = swapchainExtent.width;
        framebufferInfo.height          = swapchainExtent.height;
        framebufferInfo.layers          = 1;
        
        VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]);
        CHECK_VKRESULT(result, "failed to create framebuffer!");
    }
    { m_framebuffers = framebuffers; }
}

void Swapchain::createUniformBuffers(VkDeviceSize bufferSize) {
    LOG("createUniformBuffers");
    uint32_t totalFrame = m_totalFrame;
    
    std::vector<Buffer*> uniformBuffers;
    for (size_t i = 0; i < totalFrame; i++) {
        Buffer* uniformBuffer = new Buffer();
        uniformBuffer->setup(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        uniformBuffer->create();
        uniformBuffers.push_back(uniformBuffer);
    }
    { m_uniformBuffers = uniformBuffers; }
}


void Swapchain::updateUniformBuffer(void* address, size_t size, uint32_t index) {
    m_uniformBuffers[index]->fillBuffer(address, size);
}

void Swapchain::createDescriptorPool() {
    LOG("createDescriptorPool");
    VkDevice device     = m_device;
    uint32_t totalFrame = m_totalFrame;
    
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = totalFrame;
    poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = totalFrame;
    
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = UINT32(poolSizes.size());
    poolInfo.pPoolSizes    = poolSizes.data();
    poolInfo.maxSets       = totalFrame;
    
    VkDescriptorPool descriptorPool;
    VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
    CHECK_VKRESULT(result, "failed to create descriptor pool!");
  
    { m_descriptorPool = descriptorPool; }
}

void Swapchain::createDescriptorSetLayout() {
    LOG("createDescriptorSetLayout");
    VkDevice device = m_device;
    
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding         = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;
            
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding         = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = UINT32(bindings.size());
    layoutInfo.pBindings    = bindings.data();
    
    VkDescriptorSetLayout descriptorSetLayout;
    VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);
    CHECK_VKRESULT(result, "failed to create descriptor set layout!");
    
    { m_descriptorSetLayout = descriptorSetLayout; }
}

void Swapchain::createDescriptorSets(VkDeviceSize uniformBufferSize, VkImageView textureImageView, VkSampler textureSampler) {
    LOG("createDescriptorSets");
    VkDevice device     = m_device;
    uint32_t totalFrame = m_totalFrame;
    VkDescriptorPool      descriptorPool      = m_descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout = m_descriptorSetLayout;
    std::vector<Buffer*>  uniformBuffers      = m_uniformBuffers;
    
    std::vector<VkDescriptorSetLayout> layouts(totalFrame, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = descriptorPool;
    allocInfo.descriptorSetCount = totalFrame;
    allocInfo.pSetLayouts        = layouts.data();
    
    std::vector<VkDescriptorSet> descriptorSets(totalFrame);
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data());
    CHECK_VKRESULT(result, "failed to allocate descriptor sets!");
    
    for (size_t i = 0; i < totalFrame; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i]->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range  = uniformBufferSize;
        
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView   = textureImageView;
        imageInfo.sampler     = textureSampler;
        
        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding      = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].pBufferInfo     = &bufferInfo;
        
        descriptorWrites[1].sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding      = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].pImageInfo      = &imageInfo;
        
        vkUpdateDescriptorSets(device, UINT32(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
    { m_descriptorSets = descriptorSets; }

}

void Swapchain::createSyncObjects() {
    LOG("createSyncObjects");
    VkDevice device     = m_device;
    uint32_t totalFrame = m_totalFrame;
    
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence>     inFlightFences;
    std::vector<VkFence>     imagesInFlight;
    
    imageAvailableSemaphores.resize(totalFrame);
    renderFinishedSemaphores.resize(totalFrame);
    inFlightFences.resize(totalFrame);
    imagesInFlight.resize(totalFrame, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < totalFrame; i++) {
        VkResult result1 = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
        VkResult result2 = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
        VkResult result3 = vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]);
        CHECK_VKRESULT(result1, "failed to create image available semaphores!");
        CHECK_VKRESULT(result2, "failed to create render finished semaphores!");
        CHECK_VKRESULT(result3, "failed to create in flight fences!");
    }
    {
        m_imageAvailableSemaphores = imageAvailableSemaphores;
        m_renderFinishedSemaphores = renderFinishedSemaphores;
        m_inFlightFences = inFlightFences;
        m_imagesInFlight = imagesInFlight;
    }
    
}

