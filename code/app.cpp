//  Copyright © 2021 Subph. All rights reserved.
//

#include <array>

#include "app.h"
#include "helper.h"
#include "system.h"

void App::run() {
    initWindow();
    initVulkan();
    m_camera = new Camera();
    mainLoop();
    m_model->cleanup();
    m_texture->cleanup();
    m_pipelineGraphic->cleanup();
    m_pipelineCompute->cleanup();
    m_descriptor->cleanup();
    m_renderer->cleanUp();
    m_window->close();
}


void App::initWindow() {
    m_window = new Window();
    m_window->create(WIDTH, HEIGHT, "Vulkan");
    m_window->enableInput();
}


void App::initVulkan() {
    System &system = System::instance();
    m_renderer = new Renderer();
    system.m_renderer  = m_renderer;
    m_renderer->setupValidation(IS_DEBUG);
    m_renderer->createInstance(Window::getRequiredExtensions());
    m_renderer->createDebugMessenger();
    m_renderer->setSurface(m_window->createSurface(m_renderer->m_instance));
    m_renderer->setupDeviceExtensions();
    m_renderer->pickPhysicalDevice();
    m_renderer->createLogicalDevice();
    m_renderer->createDeviceQueue();
    
    m_renderer->createCommander();
    m_renderer->createSwapchain();
    
    m_commander = m_renderer->getCommander();
    m_swapchain = m_renderer->getSwapchain();
    
    createTexture();
    createModel();
    createShaders();
    
    recreateSwapchain();
    
    createPipelineCompute();
}

void App::createTexture() {
    m_texture = new Image();
    m_texture->setupForTexture(TEXTURE_PATH);
    m_texture->createForTexture();
    m_texture->copyRawDataToImage();
}

void App::createModel() {
    m_model = new Mesh();
    m_model->createPlane();
    m_model->cmdCreateVertexBuffer();
    m_model->cmdCreateIndexBuffer();
}

void App::createShaders() {
    m_shaders = {
        new Shader(VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT),
        new Shader(FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT)
    };
}

void App::createDescriptor() {
    Swapchain *swapchain = m_swapchain;
    std::vector<Frame*> frames = swapchain->m_frames;
    
    m_descriptor = new Descriptor();
    
    m_descriptor->setupLayout(L0, UINT32(frames.size()));
    m_descriptor->addLayoutBindings(L0, B0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    m_descriptor->createLayout(L0);
    
    m_descriptor->setupLayout(L1, 1);
    m_descriptor->addLayoutBindings(L1, B0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    m_descriptor->createLayout(L1);
    
    m_descriptor->create();
    m_descriptor->allocate(L0);
    m_descriptor->allocate(L1);
    
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView   = m_texture->getImageView();
    imageInfo.sampler     = m_texture->getSampler();
    m_descriptor->setupPointerImage(L1, S0, B0, imageInfo);
    m_descriptor->update(L1);
    
    for (uint i = 0; i < frames.size(); i++) {
        m_descriptor->setupPointerBuffer(L0, i, B0, frames[i]->getBufferInfo());
        m_descriptor->update(L0);
        frames[i]->setDescriptorSet(m_descriptor->getDescriptorSets(L0)[i]);
    }
}

void App::createPipelineGraphic() {
    Swapchain  *swapchain  = m_swapchain;
    Descriptor *descriptor = m_descriptor;
    
    m_pipelineGraphic = new PipelineGraphics();
    m_pipelineGraphic->setShaders(m_shaders);
    m_pipelineGraphic->setVertexInputInfo(m_model->createVertexInputInfo());
    
    m_pipelineGraphic->setupViewportInfo(swapchain->m_extent);
    m_pipelineGraphic->createPipelineLayout({
        descriptor->getDescriptorLayout(L0),
        descriptor->getDescriptorLayout(L1)
    });
    
    m_pipelineGraphic->setupInputAssemblyInfo();
    m_pipelineGraphic->setupRasterizationInfo();
    m_pipelineGraphic->setupMultisampleInfo();
    m_pipelineGraphic->setupColorBlendInfo();
    m_pipelineGraphic->setupDepthStencilInfo();
    m_pipelineGraphic->setupDynamicInfo();
    m_pipelineGraphic->create(swapchain->m_renderPass);
}

void App::recreateSwapchain() {
    vkDeviceWaitIdle(m_renderer->m_device);
    Swapchain *swapchain = m_swapchain;
    swapchain->cleanup();
    
    swapchain->setup(m_window->getSize());
    swapchain->create();
    swapchain->createRenderPass();
    swapchain->createFrames(sizeof(UniformBufferObject));
    swapchain->createSyncObjects();

    createDescriptor();
    createPipelineGraphic();
    recordCommandBuffer();
    
}

void App::recordCommandBuffer() {
    std::vector<VkCommandBuffer> commandBuffers = m_commander->createCommandBuffers(m_swapchain->m_totalFrame);
    VkDescriptorSet descriptorSet1 = m_descriptor->getDescriptorSets(L1)[S0];
    for (size_t i = 0; i < m_swapchain->m_totalFrame; i++) {
        Frame*          frame         = m_swapchain->m_frames[i];
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
        renderBeginInfo.renderPass  = m_swapchain->m_renderPass;
        renderBeginInfo.framebuffer = framebuffer;
        renderBeginInfo.renderArea.extent = m_swapchain->m_extent;
        renderBeginInfo.renderArea.offset = {0,0};
        renderBeginInfo.clearValueCount   = UINT32(clearValues.size());
        renderBeginInfo.pClearValues      = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineGraphic->m_pipeline);
        
        vkCmdSetViewport(commandBuffer, 0, 1, m_pipelineGraphic->m_viewport);
        
        VkBuffer vertexBuffers[] = {m_model->m_vertexBuffer->m_buffer};
        VkDeviceSize offsets[] = {0};
        
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_model->m_indexBuffer->m_buffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineGraphic->m_pipelineLayout, S0, 1, &descriptorSet, 0, nullptr);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineGraphic->m_pipelineLayout, S1, 1, &descriptorSet1, 0, nullptr);

        uint32_t indexSize = UINT32(m_model->m_indices.size());
        vkCmdDrawIndexed(commandBuffer, indexSize, 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        result = vkEndCommandBuffer(commandBuffer);
        CHECK_VKRESULT(result, "failed to record command buffer!");
    }
}

void App::createPipelineCompute() {
    Shader* computeShader = new Shader(COMP_SHADER_PATH, VK_SHADER_STAGE_COMPUTE_BIT);
    m_pipelineCompute = new PipelineCompute();
    m_pipelineCompute->setShader(computeShader);
    m_pipelineCompute->setupConstant();
    m_pipelineCompute->setupLayoutBindings(2);
    m_pipelineCompute->createDescriptorSetLayout();
    m_pipelineCompute->createPipelineLayout();
    m_pipelineCompute->create();
    
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
    while (m_window->isOpen()) {
        iteration++;
        update(iteration);
        lag -= frameDelay;
        
        while (lag < frameDelay) {
            draw(iteration);
        
            lag += TimeDif(Time::now() - lastTime).count();
            lastTime = Time::now();
        }
    }
    vkDeviceWaitIdle(m_renderer->m_device);
}

void App::update(long iteration) {
    m_window->pollEvents();
    glm::vec3 movement = glm::vec3(0.f, 0.f, 0.f);
    movement.x += m_window->getKeyState(key_d) - m_window->getKeyState(key_a);
    movement.y += m_window->getKeyState(key_q) - m_window->getKeyState(key_e);
    movement.z += m_window->getKeyState(key_w) - m_window->getKeyState(key_s);
    
    if (m_window->getMouseBtnState(mouse_btn_left)) {
        glm::vec2 cursorOffset = m_window->getCursorOffset();
        movement.x += cursorOffset.x * 0.2f;
        movement.y += cursorOffset.y * 0.2f;
    }
    movement.z += m_window->getScrollOffset().y;
    m_window->resetInput();
    
    m_camera->move(movement);
    
    m_ubo.model = glm::mat4(1.0f);
    m_ubo.model = glm::translate(m_ubo.model, glm::vec3(0.f, -0.5f, 0.f));
//    m_ubo.model = glm::rotate(m_ubo.model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//        m_ubo.m_model = glm::rotate(m_ubo.m_model, glm::radians(210.0f) + iteration * glm::radians(.4f), glm::vec3(0.0f, 0.0f, 1.0f));
    m_ubo.view = m_camera->getViewMatrix();
    m_ubo.proj = m_camera->getProjection(m_swapchain->m_extent.width / (float) m_swapchain->m_extent.height);
}

void App::draw(long iteration) {
    vkWaitForFences(m_renderer->m_device, 1, &m_swapchain->m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
    VkResult result =  vkAcquireNextImageKHR(m_renderer->m_device, m_swapchain->m_swapchain, UINT64_MAX, m_swapchain->m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    
    if (m_swapchain->m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(m_renderer->m_device, 1, &m_swapchain->m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    
    m_swapchain->m_imagesInFlight[imageIndex] = m_swapchain->m_inFlightFences[m_currentFrame];
    
    m_swapchain->m_frames[imageIndex]->updateUniformBuffer(&m_ubo, sizeof(m_ubo));
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphore[] = { m_swapchain->m_imageAvailableSemaphores[m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_swapchain->m_frames[imageIndex]->m_commandBuffer;

    VkSemaphore signalSemaphors[] = { m_swapchain->m_renderFinishedSemaphores[m_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphors;
    
    vkResetFences(m_renderer->m_device, 1, &m_swapchain->m_inFlightFences[m_currentFrame]);
    
    if (vkQueueSubmit(m_renderer->m_graphicQueue, 1, &submitInfo, m_swapchain->m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphors;

    VkSwapchainKHR swapchains[] = { m_swapchain->m_swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_renderer->m_presentQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->checkResized()) {
        recreateSwapchain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    
    m_currentFrame = (m_currentFrame + 1) % 3;
}
