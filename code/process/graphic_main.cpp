//  Copyright © 2021 Subph. All rights reserved.
//

#include "graphic_main.h"

#include "../system.h"

GraphicMain::~GraphicMain() {}
GraphicMain::GraphicMain() {
    LOG("GraphicMain::==============================");
}

void GraphicMain::cleanup() {
    LOG("GraphicMain::cleanup");
    
    for (Image* texture : m_pTextures) texture->cleanup();
    
    m_pMiscBuffer->cleanup();
    m_pInterBuffer->cleanup();
    m_pMesh->cleanup();
    m_pPipeline->cleanup();
    m_pDescriptor->cleanup();
    m_pSwapchain->cleanup();
}

void GraphicMain::setup(Size<int> size) {
    LOG("GraphicMain::setup");
    m_size = size;
    fillInput();
    createTexture();
    createModel();
    createBuffers();
    reset();
}

void GraphicMain::reset() {
    if (m_pPipeline   != nullptr) m_pPipeline->cleanup();
    if (m_pDescriptor != nullptr) m_pDescriptor->cleanup();
    if (m_pSwapchain  != nullptr) m_pSwapchain->cleanup();
    createSwapchain();
    createDescriptor();
    createPipeline();
    createDrawCommand();
}

void GraphicMain::createDrawCommand() {
    Commander*   pCommander = System::instance().getCommander();
    
    Swapchain* pSwapchain = m_pSwapchain;
    uint       totalFrame = pSwapchain->m_totalFrame;
    
    std::vector<VkCommandBuffer> commandBuffers = pCommander->createCommandBuffers(totalFrame);
    
    for (size_t i = 0; i < totalFrame; i++) {
        Frame* pFrame = pSwapchain->m_frames[i];
        pFrame->setCommandBuffer(commandBuffers[i]);
        drawCommand(pFrame);
    }
}

void GraphicMain::drawCommand(Frame* pFrame) {
    PipelineGraphic* pPipeline      = m_pPipeline;
    VkPipeline       pipeline       = pPipeline->m_pipeline;
    VkPipelineLayout pipelineLayout = pPipeline->m_pipelineLayout;
    VkExtent2D       extent         = m_pSwapchain->m_extent;
    
    Mesh* pMesh = m_pMesh;
    VkDeviceSize offsets[]   = {0};
    VkBuffer vertexBuffers[] = {pMesh->m_vertexBuffer->m_buffer};
    VkBuffer indexBuffers    =  pMesh->m_indexBuffer->m_buffer;
    uint32_t indexSize       = UINT32(pMesh->m_indices.size());
    
    Descriptor* pDescriptor = m_pDescriptor;
    VkDescriptorSet bufferDescSet  = pDescriptor->getDescriptorSets(L1)[0];
    VkDescriptorSet textureDescSet = pDescriptor->getDescriptorSets(L2)[0];
    VkDescriptorSet frameDescSet   = pFrame->m_descriptorSet;
    VkCommandBuffer commandBuffer  = pFrame->m_commandBuffer;
    
    std::vector<VkClearValue> clearValues = { CLEARCOLOR, CLEARDS };
    
    VkCommandBufferBeginInfo commandBeginInfo{};
    commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkResult result = vkBeginCommandBuffer(commandBuffer, &commandBeginInfo);
    CHECK_VKRESULT(result, "failed to begin recording command buffer!");
    {
    VkRenderPassBeginInfo renderBeginInfo = m_pSwapchain->getRenderBeginInfo();
    renderBeginInfo.framebuffer     = pFrame->m_framebuffer;
    renderBeginInfo.clearValueCount = UINT32(clearValues.size());
    renderBeginInfo.pClearValues    = clearValues.data();
    vkCmdBeginRenderPass(commandBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    
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
    
    vkCmdSetViewport(commandBuffer, 0, 1, viewport);
    vkCmdSetScissor (commandBuffer, 0, 1, scissor);
    
    vkCmdSetLineWidth(commandBuffer, 1.0f);
    
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S0, 1, &frameDescSet, 0, nullptr);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S1, 1, &bufferDescSet, 0, nullptr);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout, S2, 1, &textureDescSet, 0, nullptr);
        
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer  (commandBuffer, indexBuffers, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, indexSize, 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
    }
    result = vkEndCommandBuffer(commandBuffer);
    CHECK_VKRESULT(result, "failed to record command buffer!");
}

void GraphicMain::draw() {
    Renderer*   pRenderer  = System::instance().getRenderer();
    VkDevice    device     = pRenderer->getDevice();
    Swapchain*  pSwapchain = m_pSwapchain;
    VkSemaphore imageSemaphore = pSwapchain->m_imageSemaphores[m_currentFrame];
    
    uint32_t imageIndex;
    VkResult result =  vkAcquireNextImageKHR(device, pSwapchain->m_swapchain,
                                             UINT64_MAX, imageSemaphore,
                                             VK_NULL_HANDLE, &imageIndex);
    CHECK_VKRESULT(result, "failed to acquire swap chain image!");
    if (result == VK_ERROR_OUT_OF_DATE_KHR) return reset();
    
    Frame*          frame           = pSwapchain->m_frames[imageIndex];
    VkCommandBuffer commandBuffer   = frame->m_commandBuffer;
    VkFence         commandFence    = frame->m_commandFence;
    VkSemaphore     renderSemaphore = frame->m_renderSemaphore;
    CameraMatrix    cameraMatrix    = m_cameraMatrix;
    Misc            misc            = m_misc;
    Buffer*         miscBuffer      = m_pMiscBuffer;
    
    vkWaitForFences(device, 1, &commandFence, VK_TRUE, UINT64_MAX);
    
    frame->updateUniformBuffer(&cameraMatrix, sizeof(CameraMatrix));
    miscBuffer->fillBufferFull(&misc);

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

    
    VkSwapchainKHR swapchains[] = { pSwapchain->m_swapchain };
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = swapchains;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = signalSemaphors;
    presentInfo.pImageIndices      = &imageIndex;

    result = vkQueuePresentKHR(pRenderer->m_presentQueue, &presentInfo);
    CHECK_VKRESULT(result, "failed to present swap chain image!");
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) reset();
    
    m_currentFrame = (m_currentFrame + 1) % pSwapchain->m_totalFrame;
}



// Private ==================================================

void GraphicMain::fillInput() {
    m_misc = {};
    m_cameraMatrix = {};
}

void GraphicMain::createTexture() {
    std::vector<Image*> pTextures;
    for (std::string path : TEXURES_PATH) {
        Image* pTexture = new Image();
        pTexture->setupForTexture(path);
        pTexture->createForTexture();
        pTexture->copyRawDataToImage();
        pTextures.push_back(pTexture);
    }
    { m_pTextures = pTextures; }
}

void GraphicMain::createModel() {
    m_pMesh = new Mesh();
    m_pMesh->createCube();
    m_pMesh->cmdCreateVertexBuffer();
    m_pMesh->cmdCreateIndexBuffer();
}

void GraphicMain::createBuffers() {
    m_pMiscBuffer = new Buffer();
    m_pMiscBuffer->setup(sizeof(Misc), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    m_pMiscBuffer->create();
    m_pMiscBuffer->fillBuffer(&m_misc, sizeof(Misc));
}

void GraphicMain::createSwapchain() {
    m_pSwapchain = new Swapchain();
    m_pSwapchain->setup(m_size);
    m_pSwapchain->create();
    m_pSwapchain->createRenderPass();
    m_pSwapchain->createFrames(sizeof(CameraMatrix));
    m_pSwapchain->createSyncObjects();
}

void GraphicMain::createDescriptor() {
    Buffer* pMiscBuffer = m_pMiscBuffer;
    Buffer* pInterBuffer = m_pInterBuffer;
    std::vector<Image*> pTextures = m_pTextures;
    Swapchain *swapchain = m_pSwapchain;
    std::vector<Frame*> frames = swapchain->m_frames;
    
    Descriptor* pDescriptor = new Descriptor();
    pDescriptor->setupLayout(L0, UINT32(frames.size()));
    pDescriptor->addLayoutBindings(L0, B0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                   VK_SHADER_STAGE_VERTEX_BIT);
    pDescriptor->createLayout(L0);
    
    pDescriptor->setupLayout(L1);
    pDescriptor->addLayoutBindings(L1, B0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                   VK_SHADER_STAGE_FRAGMENT_BIT);
    pDescriptor->addLayoutBindings(L1, B1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                   VK_SHADER_STAGE_FRAGMENT_BIT);
    pDescriptor->createLayout(L1);
    
    pDescriptor->setupLayout(L2);
    for (uint i = 0; i < pTextures.size(); i++) {
        pDescriptor->addLayoutBindings(L2, i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                       VK_SHADER_STAGE_FRAGMENT_BIT);
    }
    pDescriptor->createLayout(L2);
    
    pDescriptor->createPool();
    pDescriptor->allocate(L0);
    pDescriptor->allocate(L1);
    pDescriptor->allocate(L2);
    
    for (uint i = 0; i < frames.size(); i++) {
        VkDescriptorBufferInfo bufferInfo = frames[i]->getBufferInfo();
        pDescriptor->setupPointerBuffer(L0, i, B0, &bufferInfo);
        pDescriptor->update(L0);
        frames[i]->setDescriptorSet(pDescriptor->getDescriptorSets(L0)[i]);
    }
    
    VkDescriptorBufferInfo outputBInfo = pInterBuffer->getBufferInfo();
    VkDescriptorBufferInfo miscBInfo   = pMiscBuffer->getBufferInfo();
    pDescriptor->setupPointerBuffer(L1, S0, B0, &outputBInfo);
    pDescriptor->setupPointerBuffer(L1, S0, B1, &miscBInfo);
    pDescriptor->update(L1);
    
    VkDescriptorImageInfo imageInfos[pTextures.size()];
    for (uint i = 0; i < pTextures.size(); i++) {
        imageInfos[i] = pTextures[i]->getImageInfo();
        pDescriptor->setupPointerImage(L2, S0, i, &imageInfos[i]);
    }
    pDescriptor->update(L2);
    
    { m_pDescriptor = pDescriptor; }
}

void GraphicMain::createPipeline() {
    Swapchain*  pSwapchain   = m_pSwapchain;
    Descriptor* pDdescriptor = m_pDescriptor;
    Mesh*       pMesh        = m_pMesh;
    
    std::vector<Shader*> shaders = {
        new Shader(VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT),
        new Shader(FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    
    PipelineGraphic* pPipeline = new PipelineGraphic();
    pPipeline->setShaders(shaders);
    pPipeline->setVertexInputInfo(pMesh->createVertexInputInfo());
    
    pPipeline->setupViewportInfo(pSwapchain->m_extent);
    pPipeline->createPipelineLayout({
        pDdescriptor->getDescriptorLayout(L0),
        pDdescriptor->getDescriptorLayout(L1),
        pDdescriptor->getDescriptorLayout(L2)
    });
    
    pPipeline->setupInputAssemblyInfo();
    pPipeline->setupRasterizationInfo();
    pPipeline->setupMultisampleInfo();
    pPipeline->setupColorBlendInfo();
    pPipeline->setupDepthStencilInfo();
    pPipeline->setupDynamicInfo();
    pPipeline->create(pSwapchain->m_renderPass);
    
    { m_pPipeline = pPipeline; }
}

