//  Copyright © 2021 Subph. All rights reserved.
//

#include <array>

#include "app.h"
#include "helper.h"

void App::run() {
    initWindow();
    initVulkan();
    m_camera = new Camera();
    mainLoop();
    m_model->cleanup();
    m_renderer->cleanUp();
    m_window->close();
}


void App::initWindow() {
    m_window = new Window();
    m_window->create(WIDTH, HEIGHT, "Vulkan");
    m_window->enableInput();
}


void App::initVulkan() {
    m_renderer = new Renderer();
    m_renderer->setupValidation(IS_DEBUG);
    m_renderer->createInstance(Window::getRequiredExtensions());
    m_renderer->createDebugMessenger();
    m_renderer->setSurface(m_window->createSurface(m_renderer->m_instance));
    m_renderer->setDeviceExtensions();
    m_renderer->pickPhysicalDevice();
    m_renderer->createLogicalDevice();
    m_renderer->createDeviceQueue();
    m_renderer->createCommandPool();
    
    createTexture();
    createModel();
    
    m_renderer->createDescriptorSetLayout();
    
    recreateSwapChain();

    m_renderer->createSyncObjects();
}

void App::recreateSwapChain() {
    vkDeviceWaitIdle(m_renderer->m_device);

    m_renderer->cleanUpSwapChain();
    
    m_renderer->createSwapChain(m_window->getSize());
    m_renderer->createImageViews();
    m_renderer->createRenderPass();
    m_renderer->createPipelineLayout();
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {
        m_renderer->createShaderStageInfo(VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT),
        m_renderer->createShaderStageInfo(FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT)
    };
    
    m_renderer->createGraphicsPipeline(shaderStages, m_model->createVertexInputInfo());
    
    m_renderer->createDepthResources();
    m_renderer->createFramebuffer();
    m_renderer->createUniformBuffers(sizeof(UniformBufferObject));
    m_renderer->createDescriptorPool();
    m_renderer->createDescriptorSets(sizeof(UniformBufferObject), m_texture.imageView, m_texture.sampler);
    recordCommandBuffer();
}

void App::recordCommandBuffer() {
    m_renderer->createCommandBuffers();

    for (size_t i = 0; i < m_renderer->m_commandBuffers.size(); i++) {
        VkCommandBuffer commandBuffer= m_renderer->m_commandBuffers[i];
        VkFramebuffer framebuffer = m_renderer->m_swapChainFramebuffers[i];
        VkDescriptorSet descriptorSet = m_renderer->m_descriptorSets[i];
        
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        CHECK_VKRESULT(result, "failed to begin recording command buffer!");
            
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.8f, 0.8f, 0.8f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderer->m_renderPass;
        renderPassInfo.framebuffer = framebuffer;
        renderPassInfo.renderArea.extent = m_renderer->m_swapChainExtent;
        renderPassInfo.renderArea.offset = {0,0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderer->m_graphicsPipeline);

        VkBuffer vertexBuffers[] = {m_model->vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, m_model->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_renderer->m_pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

        uint32_t indexSize = static_cast<uint32_t>(m_model->m_indices.size());
        vkCmdDrawIndexed(commandBuffer, indexSize, 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        result = vkEndCommandBuffer(commandBuffer);
        CHECK_VKRESULT(result, "failed to record command buffer!");
    }
}

void App::createTexture() {
    int width, height, channels;
    unsigned char* data = ReadImage(TEXTURE_PATH, &width, &height, &channels);
    uint32_t maxMipLevels = MaxMapLevel(width, height);

    m_texture.image = m_renderer->createTextureImage(data, width, height, channels, maxMipLevels);

    m_texture.imageView = m_renderer->createImageView(m_texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, maxMipLevels);

    m_texture.sampler = m_renderer->createTextureSampler(maxMipLevels);
}

void App::createModel() {
    m_model = new Mesh();
    m_model->setRenderer(m_renderer);
    m_model->createPlane();
    m_model->createVertexBuffer();
    m_model->createIndexBuffer();
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
    m_ubo.proj = m_camera->getProjection(m_renderer->m_swapChainExtent.width / (float) m_renderer->m_swapChainExtent.height);
}

void App::draw(long iteration) {
    vkWaitForFences(m_renderer->m_device, 1, &m_renderer->m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
    VkResult result =  vkAcquireNextImageKHR(m_renderer->m_device, m_renderer->m_swapChain, UINT64_MAX, m_renderer->m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    
    if (m_renderer->m_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(m_renderer->m_device, 1, &m_renderer->m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    
    m_renderer->m_imagesInFlight[imageIndex] = m_renderer->m_inFlightFences[m_currentFrame];
    
    m_renderer->updateUniformBuffer(&m_ubo, sizeof(m_ubo), imageIndex);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphore[] = { m_renderer->m_imageAvailableSemaphores[m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_renderer->m_commandBuffers[imageIndex];

    VkSemaphore signalSemaphors[] = { m_renderer->m_renderFinishedSemaphores[m_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphors;
    
    vkResetFences(m_renderer->m_device, 1, &m_renderer->m_inFlightFences[m_currentFrame]);
    
    if (vkQueueSubmit(m_renderer->m_graphicQueue, 1, &submitInfo, m_renderer->m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphors;

    VkSwapchainKHR swapChains[] = { m_renderer->m_swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_renderer->m_presentQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window->checkResized()) {
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
    
    m_currentFrame = (m_currentFrame + 1) % DOUBLEBUFFER;
}
