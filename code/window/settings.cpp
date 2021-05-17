//  Copyright © 2021 Subph. All rights reserved.
//

#include "settings.h"

#include "../system.h"

Settings::~Settings() { }
Settings::Settings() { }

void Settings::cleanup() { m_cleaner.flush(); }

void Settings::setWindow(Window* window) { m_pWindow = window; }

void Settings::initGUI(VkRenderPass renderPass) {
    LOG("Settings::init");
    Renderer* renderer = System::Renderer();
    VkInstance       instance       = renderer->getInstance();
    VkDevice         device         = renderer->getDevice();
    VkPhysicalDevice physicalDevice = renderer->getPhysicalDevice();
    VkQueue          graphicQueue   = renderer->getGraphicQueue();
    Commander*       commander      = System::Commander();
    
    Window* pWindow = m_pWindow;
    
    // 1: create descriptor pool for IMGUI
    // the size of the pool is very oversize, but it's copied from imgui demo itself.
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER                , 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE          , 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE          , 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER   , 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER   , 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER         , 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER         , 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC , 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC , 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT       , 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = std::size(pool_sizes);
    poolInfo.pPoolSizes = pool_sizes;

    VkDescriptorPool imguiPool;
    VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &imguiPool);
    CHECK_VKRESULT(result, "failed to create descriptor pool!");

    // 2: initialize imgui library

    //this initializes the core structures of imgui
    ImGui::CreateContext();

    //this initializes imgui for GLFW for Vulkan
    ImGui_ImplGlfw_InitForVulkan(pWindow->getGLFWwindow(), nullptr);

    //this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance       = instance;
    initInfo.PhysicalDevice = physicalDevice;
    initInfo.Device         = device;
    initInfo.Queue          = graphicQueue;
    initInfo.DescriptorPool = imguiPool;
    initInfo.MinImageCount  = 3;
    initInfo.ImageCount     = 3;
    initInfo.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo, renderPass);
    
    VkCommandBuffer commandBuffer = commander->createCommandBuffer();
    commander->beginSingleTimeCommands(commandBuffer);
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    commander->endSingleTimeCommands(commandBuffer);

    //clear font textures from cpu data
    ImGui_ImplVulkan_DestroyFontUploadObjects();
    
    m_imguiPool = imguiPool;
    m_cleaner.push([=]() {
        vkDestroyDescriptorPool(device, imguiPool, nullptr);
        ImGui_ImplVulkan_Shutdown();
    });
}

void Settings::drawGUI() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    if (ShowDemo) ImGui::ShowDemoWindow(&ShowDemo);
    drawStatusWindow();
    
    ImGui::Render();
}

void Settings::drawStatusWindow() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("Status");
    
    ImGui::Text("%.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    
    ImGui::Checkbox("Lock FPS"  , &LockFPS);
    ImGui::Checkbox("Lock Focus", &LockFocus);
    
    ImGui::ColorEdit3("Clear Color", (float*) &ClearColor);

//    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
//    if (ImGui::Button("Button"))
//        counter++;
//    ImGui::SameLine();
//    ImGui::Text("counter = %d", counter);
    
    ImGui::Checkbox("Show ImGUI demo", &ShowDemo);
    
    ImGui::End();
}

void Settings::renderGUI(VkCommandBuffer commandBuffer) {
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}
