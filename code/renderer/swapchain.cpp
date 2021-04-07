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
    LOG("Swapchain::cleanup");
    if (m_frames.size() == 0) return;
    
    for (size_t i = 0; i < m_inFlightFences.size(); i++) {
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
    }
    
    for (size_t i = 0; i < m_frames.size(); i++)
        m_frames[i]->cleanup();
    m_frames = {};
    
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
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
        m_extent        = extent;
        m_swapchainInfo = swapchainInfo;
        m_surfaceFormat = surfaceFormat.format;
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
    colorAttachment.format          = m_surfaceFormat;
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

void Swapchain::createFrames(VkDeviceSize uniformBufferSize) {
    LOG("createFrames");
    VkSwapchainKHR swapchain     = m_swapchain;
    VkRenderPass   renderPass    = m_renderPass;
    VkExtent2D     extent        = m_extent;
    VkFormat       surfaceFormat = m_surfaceFormat;
    
    std::vector<VkImage> swapchainImages = GetSwapchainImages(swapchain);
    uint32_t totalFrame = UINT32(swapchainImages.size());
    
    std::vector<Frame*> frames;
    for (size_t i = 0; i < totalFrame; i++) {
        Frame* frame = new Frame();
        frame->setSize({extent.width, extent.height});
        frame->createDepthResource();
        frame->createImageResource(swapchainImages[i], surfaceFormat);
        frame->createFramebuffer(renderPass);
        frame->createUniformBuffer(uniformBufferSize);
        frames.push_back(frame);
    }
    {
        m_frames = frames;
        m_totalFrame = totalFrame;
    }
}

void Swapchain::createSyncObjects() {
    LOG("createSyncObjects");
    VkDevice device     = m_device;
    uint32_t totalFrame = UINT32(m_frames.size());
    
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



// Private ==================================================


std::vector<VkImage> Swapchain::GetSwapchainImages(VkSwapchainKHR swapchain) {
    System &system  = System::instance();
    VkDevice device = system.m_renderer->m_device;
    
    uint32_t count;
    vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);
    std::vector<VkImage> swapchainImages(count);
    vkGetSwapchainImagesKHR(device, swapchain, &count, swapchainImages.data());
    return swapchainImages;
}