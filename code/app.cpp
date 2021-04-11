//  Copyright © 2021 Subph. All rights reserved.
//

#include <array>

#include "app.h"
#include "helper.h"
#include "system.h"

void App::run() {
    initWindow();
    initVulkan();
    m_pCamera = new Camera();
    mainLoop();
    m_pModel->cleanup();
    m_pTexture->cleanup();
    m_pPipelineGraphic->cleanup();
    m_pPipelineCompute->cleanup();
    m_pDescriptor->cleanup();
    m_pRenderer->cleanUp();
    m_pWindow->close();
}


void App::initWindow() {
    m_pWindow = new Window();
    m_pWindow->create(WIDTH, HEIGHT, "Vulkan");
    m_pWindow->enableInput();
}


void App::initVulkan() {
    System &system = System::instance();
    m_pRenderer = new Renderer();
    system.m_renderer  = m_pRenderer;
    m_pRenderer->setupValidation(IS_DEBUG);
    m_pRenderer->createInstance(Window::getRequiredExtensions());
    m_pRenderer->createDebugMessenger();
    m_pRenderer->setSurface(m_pWindow->createSurface(m_pRenderer->m_instance));
    m_pRenderer->setupDeviceExtensions();
    m_pRenderer->pickPhysicalDevice();
    m_pRenderer->createLogicalDevice();
    m_pRenderer->createDeviceQueue();
    
    m_pRenderer->createCommander();
    m_pRenderer->createSwapchain();
    
    m_pCommander = m_pRenderer->getCommander();
    m_pSwapchain = m_pRenderer->getSwapchain();
    
    createTexture();
    createModel();
    createShaders();
    
    recreateSwapchain();
    
    createPipelineCompute();
}

void App::createTexture() {
    m_pTexture = new Image();
    m_pTexture->setupForTexture(TEXTURE_PATH);
    m_pTexture->createForTexture();
    m_pTexture->copyRawDataToImage();
}

void App::createModel() {
    m_pModel = new Mesh();
    m_pModel->createPlane();
    m_pModel->cmdCreateVertexBuffer();
    m_pModel->cmdCreateIndexBuffer();
}

void App::createShaders() {
    m_shaders = {
        new Shader(VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT),
        new Shader(FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT)
    };
}

void App::createDescriptor() {
    Swapchain *swapchain = m_pSwapchain;
    std::vector<Frame*> frames = swapchain->m_frames;
    
    m_pDescriptor = new Descriptor();
    
    m_pDescriptor->setupLayout(L0, UINT32(frames.size()));
    m_pDescriptor->addLayoutBindings(L0, B0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    m_pDescriptor->createLayout(L0);
    
    m_pDescriptor->setupLayout(L1, 1);
    m_pDescriptor->addLayoutBindings(L1, B0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    m_pDescriptor->createLayout(L1);
    
    m_pDescriptor->create();
    m_pDescriptor->allocate(L0);
    m_pDescriptor->allocate(L1);
    
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView   = m_pTexture->getImageView();
    imageInfo.sampler     = m_pTexture->getSampler();
    m_pDescriptor->setupPointerImage(L1, S0, B0, imageInfo);
    m_pDescriptor->update(L1);
    
    for (uint i = 0; i < frames.size(); i++) {
        m_pDescriptor->setupPointerBuffer(L0, i, B0, frames[i]->getBufferInfo());
        m_pDescriptor->update(L0);
        frames[i]->setDescriptorSet(m_pDescriptor->getDescriptorSets(L0)[i]);
    }
}

void App::createPipelineGraphic() {
    Swapchain  *swapchain  = m_pSwapchain;
    Descriptor *descriptor = m_pDescriptor;
    
    m_pPipelineGraphic = new PipelineGraphic();
    m_pPipelineGraphic->setShaders(m_shaders);
    m_pPipelineGraphic->setVertexInputInfo(m_pModel->createVertexInputInfo());
    
    m_pPipelineGraphic->setupViewportInfo(swapchain->m_extent);
    m_pPipelineGraphic->createPipelineLayout({
        descriptor->getDescriptorLayout(L0),
        descriptor->getDescriptorLayout(L1)
    });
    
    m_pPipelineGraphic->setupInputAssemblyInfo();
    m_pPipelineGraphic->setupRasterizationInfo();
    m_pPipelineGraphic->setupMultisampleInfo();
    m_pPipelineGraphic->setupColorBlendInfo();
    m_pPipelineGraphic->setupDepthStencilInfo();
    m_pPipelineGraphic->setupDynamicInfo();
    m_pPipelineGraphic->create(swapchain->m_renderPass);
}

void App::recreateSwapchain() {
    vkDeviceWaitIdle(m_pRenderer->m_device);
    Swapchain *swapchain = m_pSwapchain;
    swapchain->cleanup();
    
    swapchain->setup(m_pWindow->getSize());
    swapchain->create();
    swapchain->createRenderPass();
    swapchain->createFrames(sizeof(UniformBufferObject));
    swapchain->createSyncObjects();

    createDescriptor();
    createPipelineGraphic();
    recordCommandBuffer();
    
}

void App::recordCommandBuffer() {
    std::vector<VkCommandBuffer> commandBuffers = m_pCommander->createCommandBuffers(m_pSwapchain->m_totalFrame);
    VkDescriptorSet descriptorSet1 = m_pDescriptor->getDescriptorSets(L1)[S0];
    for (size_t i = 0; i < m_pSwapchain->m_totalFrame; i++) {
        Frame*          frame         = m_pSwapchain->m_frames[i];
        frame->m_commandBuffer = commandBuffers[i];
        VkCommandBuffer commandBuffer = frame->m_commandBuffer;
        VkFramebuffer   framebuffer   = frame->m_framebuffer;
        VkDescriptorSet descriptorSet = frame->m_descriptorSet;
        
        VkCommandBufferBeginInfo commandBeginInfo{};
        commandBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VkResult result = vkBeginCommandBuffer(commandBuffer, &commandBeginInfo);
        CHECK_VKRESULT(result, "failed to begin recording command buffer!");
            
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.8f, 0.8f, 0.8f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        
        VkRenderPassBeginInfo renderBeginInfo{};
        renderBeginInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderBeginInfo.renderPass  = m_pSwapchain->m_renderPass;
        renderBeginInfo.framebuffer = framebuffer;
        renderBeginInfo.renderArea.extent = m_pSwapchain->m_extent;
        renderBeginInfo.renderArea.offset = {0,0};
        renderBeginInfo.clearValueCount   = UINT32(clearValues.size());
        renderBeginInfo.pClearValues      = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pPipelineGraphic->m_pipeline);
        
        vkCmdSetViewport(commandBuffer, 0, 1, m_pPipelineGraphic->m_viewport);
        
        VkBuffer vertexBuffers[] = {m_pModel->m_vertexBuffer->m_buffer};
        VkDeviceSize offsets[] = {0};
        
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_pModel->m_indexBuffer->m_buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pPipelineGraphic->m_pipelineLayout, S0, 1, &descriptorSet, 0, nullptr);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pPipelineGraphic->m_pipelineLayout, S1, 1, &descriptorSet1, 0, nullptr);

        uint32_t indexSize = UINT32(m_pModel->m_indices.size());
        vkCmdDrawIndexed(commandBuffer, indexSize, 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        result = vkEndCommandBuffer(commandBuffer);
        CHECK_VKRESULT(result, "failed to record command buffer!");
    }
}

void App::createPipelineCompute() {
    Shader* computeShader = new Shader(COMP_SHADER_PATH, VK_SHADER_STAGE_COMPUTE_BIT);
    m_pPipelineCompute = new PipelineCompute();
    m_pPipelineCompute->setShader(computeShader);
    m_pPipelineCompute->setupConstant();
    m_pPipelineCompute->setupLayoutBindings(2);
    m_pPipelineCompute->createDescriptorSetLayout();
    m_pPipelineCompute->createPipelineLayout();
    m_pPipelineCompute->create();
    
    uint size = 1000;
    Buffer* iptBuffer = new Buffer();
    iptBuffer->setup(size * sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    iptBuffer->create();
    Buffer* optBuffer = new Buffer();
    optBuffer->setup(size * size * sizeof(float), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    optBuffer->create();
    
    iptBuffer->cleanup();
    optBuffer->cleanup();
}

void App::mainLoop() {
    auto lastTime = Time::now();
    float frameDelay = 1.f/60.f;
    float lag = frameDelay;
    
    long iteration = 0;
    while (m_pWindow->isOpen()) {
        iteration++;
        update(iteration);
        lag -= frameDelay;
        
        while (lag < frameDelay) {
            draw(iteration);
        
            lag += TimeDif(Time::now() - lastTime).count();
            lastTime = Time::now();
        }
    }
    vkDeviceWaitIdle(m_pRenderer->m_device);
}

void App::update(long iteration) {
    m_pWindow->pollEvents();
    glm::vec3 movement = glm::vec3(0.f, 0.f, 0.f);
    movement.x += m_pWindow->getKeyState(key_d) - m_pWindow->getKeyState(key_a);
    movement.y += m_pWindow->getKeyState(key_q) - m_pWindow->getKeyState(key_e);
    movement.z += m_pWindow->getKeyState(key_w) - m_pWindow->getKeyState(key_s);
    
    if (m_pWindow->getMouseBtnState(mouse_btn_left)) {
        glm::vec2 cursorOffset = m_pWindow->getCursorOffset();
        movement.x += cursorOffset.x * 0.2f;
        movement.y += cursorOffset.y * 0.2f;
    }
    movement.z += m_pWindow->getScrollOffset().y;
    m_pWindow->resetInput();
    
    m_pCamera->move(movement);
    
    m_ubo.model = glm::mat4(1.0f);
    m_ubo.model = glm::translate(m_ubo.model, glm::vec3(0.f, -0.5f, 0.f));
    m_ubo.view = m_pCamera->getViewMatrix();
    m_ubo.proj = m_pCamera->getProjection(m_pSwapchain->m_extent.width / (float) m_pSwapchain->m_extent.height);
}

void App::draw(long iteration) {
    vkWaitForFences(m_pRenderer->m_device, 1, &m_pSwapchain->m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
    VkResult result =  vkAcquireNextImageKHR(m_pRenderer->m_device, m_pSwapchain->m_swapchain, UINT64_MAX, m_pSwapchain->m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    
    if (m_pSwapchain->m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(m_pRenderer->m_device, 1, &m_pSwapchain->m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    
    m_pSwapchain->m_imagesInFlight[imageIndex] = m_pSwapchain->m_inFlightFences[m_currentFrame];
    
    m_pSwapchain->m_frames[imageIndex]->updateUniformBuffer(&m_ubo, sizeof(m_ubo));
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphore[] = { m_pSwapchain->m_imageAvailableSemaphores[m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_pSwapchain->m_frames[imageIndex]->m_commandBuffer;

    VkSemaphore signalSemaphors[] = { m_pSwapchain->m_renderFinishedSemaphores[m_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphors;
    
    vkResetFences(m_pRenderer->m_device, 1, &m_pSwapchain->m_inFlightFences[m_currentFrame]);
    
    if (vkQueueSubmit(m_pRenderer->m_graphicQueue, 1, &submitInfo, m_pSwapchain->m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphors;

    VkSwapchainKHR swapchains[] = { m_pSwapchain->m_swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_pRenderer->m_presentQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_pWindow->checkResized()) {
        recreateSwapchain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    
    m_currentFrame = (m_currentFrame + 1) % 3;
}
