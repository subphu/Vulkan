//  Copyright © 2021 Subph. All rights reserved.
//

#include "graphic_main.h"

#include "../system.h"

#include "helper.h"

GraphicMain::~GraphicMain() {}
GraphicMain::GraphicMain() {
    LOG("GraphicMain::==============================");
    Renderer* renderer = System::Renderer();
    m_device           = renderer->getDevice();
    m_physicalDevice   = renderer->getPhysicalDevice();
}

void GraphicMain::cleanup() {
    LOG("GraphicMain::cleanup");
    
    for (Shader* shader : m_pShaders ) shader->cleanup();
    
    m_pCube->cleanup();
    cleanupSwapchain();
    m_pPipeline->cleanup();
    m_pDescriptor->cleanup();
}

void GraphicMain::cleanupSwapchain() {
    if (m_frames.size() == 0) return;
    for (size_t i = 0; i < m_imageSemaphores.size(); i++) {
        vkDestroySemaphore(m_device, m_imageSemaphores[i], nullptr);
    }
    
    for (size_t i = 0; i < m_frames.size(); i++)
        m_frames[i]->cleanup();
    m_frames = {};
    
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
}

void GraphicMain::setup(Window* pWindow) {
    LOG("GraphicMain::setup");
    m_pWindow = pWindow;
    fillInput();
    createModel();
    createSwapchain();
    createRenderPass();
    createFrames();
    createSyncObjects();
    createDescriptor();
    createPipeline();
}

void GraphicMain::drawCommand(Frame* pFrame) {
    PipelineGraphic* pPipeline      = m_pPipeline;
    VkPipeline       pipeline       = pPipeline->m_pipeline;
    VkPipelineLayout pipelineLayout = pPipeline->m_pipelineLayout;
    VkExtent2D       extent         = m_extent;
    CameraMatrix     cameraMatrix   = m_cameraMatrix;
    
    Mesh* pCube  = m_pCube;
    Mesh* pFloor = m_pFloor;
    VkDeviceSize offsets[]   = {0};
    
    VkDescriptorSet frameDescSet   = pFrame->m_descriptorSet;
    VkCommandBuffer commandBuffer  = pFrame->m_commandBuffer;
    
    VkViewport* viewport = new VkViewport();
    viewport->x = 0.0f;
    viewport->y = 0.0f;
    viewport->width  = (float) m_size.width;
    viewport->height = (float) m_size.height;
    viewport->minDepth = 0.0f;
    viewport->maxDepth = 1.0f;
    
    VkRect2D* scissor = new VkRect2D();
    scissor->offset = {0, 0};
    scissor->extent = extent;
    
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color =  {0.1f, 0.1f, 0.1f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    
    VkCommandBufferBeginInfo commandBeginInfo{};
    commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkResult result = vkBeginCommandBuffer(commandBuffer, &commandBeginInfo);
    CHECK_VKRESULT(result, "failed to begin recording command buffer!");
    {
        vkCmdSetViewport(commandBuffer, 0, 1, viewport);
        vkCmdSetScissor (commandBuffer, 0, 1, scissor);
        
        vkCmdSetLineWidth(commandBuffer, 1.0f);
        
        VkRenderPassBeginInfo renderBeginInfo{};
        renderBeginInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderBeginInfo.renderPass  = m_renderPass;
        renderBeginInfo.renderArea.extent = m_extent;
        renderBeginInfo.renderArea.offset = {0,0};
        renderBeginInfo.framebuffer     = pFrame->m_framebuffer;
        renderBeginInfo.clearValueCount = UINT32(clearValues.size());
        renderBeginInfo.pClearValues    = clearValues.data();
        vkCmdBeginRenderPass(commandBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            cameraMatrix.model = pCube->getMatrix();
            pFrame->updateUniformBuffer(&cameraMatrix, sizeof(CameraMatrix));
            
            VkBuffer vertexBuffers[] = {pCube->m_vertexBuffer->m_buffer};
            VkBuffer indexBuffers    =  pCube->m_indexBuffer->m_buffer;
            uint32_t indexSize       = UINT32(pCube->m_indices.size());
            
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout, L0, 1, &frameDescSet, 0, nullptr);
                
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer  (commandBuffer, indexBuffers, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(commandBuffer, indexSize, 1, 0, 0, 0);
        }
        {
            cameraMatrix.model = pFloor->getMatrix();
            pFrame->updateUniformBuffer(&cameraMatrix, sizeof(CameraMatrix));
            
            VkBuffer vertexBuffers[] = {pFloor->m_vertexBuffer->m_buffer};
            VkBuffer indexBuffers    =  pFloor->m_indexBuffer->m_buffer;
            uint32_t indexSize       = UINT32(pFloor->m_indices.size());
            
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout, L0, 1, &frameDescSet, 0, nullptr);
                
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer  (commandBuffer, indexBuffers, 0, VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(commandBuffer, indexSize, 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(commandBuffer);
    }
    result = vkEndCommandBuffer(commandBuffer);
    CHECK_VKRESULT(result, "failed to record command buffer!");
}

void GraphicMain::draw() {
    Renderer*   pRenderer  = System::Renderer();
    VkDevice    device     = pRenderer->getDevice();
    VkSemaphore imageSemaphore = m_imageSemaphores[m_currentFrame];
    
    uint32_t imageIndex;
    VkResult result =  vkAcquireNextImageKHR(device, m_swapchain,
                                             UINT64_MAX, imageSemaphore,
                                             VK_NULL_HANDLE, &imageIndex);
    
    Frame*          frame           = m_frames[imageIndex];
    VkCommandBuffer commandBuffer   = frame->m_commandBuffer;
    VkFence         commandFence    = frame->m_commandFence;
    VkSemaphore     renderSemaphore = frame->m_renderSemaphore;
    
    vkWaitForFences(device, 1, &commandFence, VK_TRUE, UINT64_MAX);
    std::vector<VkCommandBuffer> commandBuffers = System::Commander()->createCommandBuffers(2);

    drawCommand(frame);
    

    VkSemaphore waitSemaphore[]   = { imageSemaphore };
    VkSemaphore signalSemaphors[] = { renderSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &commandBuffer;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = waitSemaphore;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphors;
    
    vkResetFences(device, 1, &commandFence);
    result = vkQueueSubmit(pRenderer->m_graphicQueue, 1, &submitInfo, commandFence);
    CHECK_VKRESULT(result, "failed to submit draw command buffer!");

    
    VkSwapchainKHR swapchains[] = { m_swapchain };
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = swapchains;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = signalSemaphors;
    presentInfo.pImageIndices      = &imageIndex;

    result = vkQueuePresentKHR(pRenderer->m_presentQueue, &presentInfo);
    
    
    m_currentFrame = (m_currentFrame + 1) % m_totalFrame;
}

void GraphicMain::setShaders(std::vector<Shader*> shaders) { m_pShaders = shaders; }

// Private ==================================================

void GraphicMain::fillInput() {
    m_cameraMatrix = {};
}

void GraphicMain::createModel() {
    m_pCube = new Mesh();
    m_pCube->createCube();
    m_pCube->cmdCreateVertexBuffer();
    m_pCube->cmdCreateIndexBuffer();
    m_pFloor = new Mesh();
    m_pFloor->createPlane();
    m_pFloor->translate(glm::vec3(0, 0, 0));
    m_pFloor->cmdCreateVertexBuffer();
    m_pFloor->cmdCreateIndexBuffer();
}

void GraphicMain::createSwapchain() {
    m_size = m_pWindow->getFrameSize();
    Renderer* renderer = System::Renderer();
    
    VkSurfaceKHR surface = m_pWindow->getSurface();
    VkSurfaceFormatKHR surfaceFormat = renderer->getSwapchainSurfaceFormat();
    VkPresentModeKHR   presentMode   = renderer->getSwapchainPresentMode();
    uint32_t graphicFamilyIndex = renderer->getGraphicQueueIndex();
    uint32_t presentFamilyIndex = renderer->getPresentQueueIndex(surface);
    
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, surface, &capabilities);
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;
    
    m_extent = ChooseSwapExtent(capabilities, m_size);
    
    VkSwapchainCreateInfoKHR swapchainInfo{};
    swapchainInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainInfo.surface          = surface;
    swapchainInfo.minImageCount    = imageCount;
    swapchainInfo.imageFormat      = surfaceFormat.format;
    swapchainInfo.imageColorSpace  = surfaceFormat.colorSpace;
    swapchainInfo.imageExtent      = m_extent;
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
    
    VkResult result = vkCreateSwapchainKHR(m_device, &swapchainInfo, nullptr, &m_swapchain);
    CHECK_VKRESULT(result, "failed to create swap chain!");
    m_surfaceFormat = surfaceFormat.format;
}

void GraphicMain::createRenderPass() {
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
    
    VkResult result = vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass);
    CHECK_VKRESULT(result, "failed to create render pass!");
}

void GraphicMain::createFrames() {
    LOG("Swapchain::createFrames");
    Commander*     pCommander    = System::Commander();
    VkSwapchainKHR swapchain     = m_swapchain;
    VkRenderPass   renderPass    = m_renderPass;
    VkExtent2D     extent        = m_extent;
    VkFormat       surfaceFormat = m_surfaceFormat;
    
    std::vector<VkImage> swapchainImages = GetSwapchainImagesKHR(m_device, swapchain);
    m_totalFrame = UINT32(swapchainImages.size());
    
    std::vector<VkCommandBuffer> commandBuffers = pCommander->createCommandBuffers(m_totalFrame);
    
    std::vector<Frame*> frames;
    for (size_t i = 0; i < m_totalFrame; i++) {
        Frame* frame = new Frame();
        frame->setSize({extent.width, extent.height});
        frame->setCommandBuffer(commandBuffers[i]);
        frame->createDepthResource();
        frame->createImageResource(swapchainImages[i], surfaceFormat);
        frame->createFramebuffer(renderPass);
        frame->createUniformBuffer(sizeof(CameraMatrix));
        frame->createFinishSignal();
        frames.push_back(frame);
    }
    m_frames = frames;
}

void GraphicMain::createSyncObjects() {
    LOG("Swapchain::createSyncObjects");
    m_imageSemaphores.resize(m_totalFrame);
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (size_t i = 0; i < m_totalFrame; i++) {
        VkResult result = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageSemaphores[i]);
        CHECK_VKRESULT(result, "failed to create image available semaphores!");
    }
}

void GraphicMain::createDescriptor() {
    LOG("GraphicMain::createDescriptor");
    m_pDescriptor = new Descriptor();
    m_pDescriptor->setupLayout(L0, UINT32(m_totalFrame));
    m_pDescriptor->addLayoutBindings(L0, B0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                   VK_SHADER_STAGE_VERTEX_BIT);
    m_pDescriptor->createLayout(L0);
    
    m_pDescriptor->createPool();
    m_pDescriptor->allocate(L0);
    
    for (uint i = 0; i < m_totalFrame; i++) {
        VkDescriptorBufferInfo bufferInfo = m_frames[i]->getBufferInfo();
        m_pDescriptor->setupPointerBuffer(L0, i, B0, &bufferInfo);
        m_pDescriptor->update(L0);
        m_frames[i]->setDescriptorSet(m_pDescriptor->getDescriptorSets(L0)[i]);
    }
}

void GraphicMain::createPipeline() {
    m_pPipeline = new PipelineGraphic();
    m_pPipeline->setShaders(m_pShaders);
    m_pPipeline->setVertexInputInfo(m_pCube->createVertexInputInfo());
    
    m_pPipeline->setupViewportInfo(m_extent);
    m_pPipeline->createPipelineLayout({
        m_pDescriptor->getDescriptorLayout(L0)
    });
    
    m_pPipeline->setupInputAssemblyInfo();
    m_pPipeline->setupRasterizationInfo();
    m_pPipeline->setupMultisampleInfo();
    m_pPipeline->setupColorBlendInfo();
    m_pPipeline->setupDepthStencilInfo();
    m_pPipeline->setupDynamicInfo();
    m_pPipeline->create(m_renderPass);
}
