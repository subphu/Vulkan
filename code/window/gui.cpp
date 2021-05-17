//  Copyright © 2021 Subph. All rights reserved.
//

#include "gui.h"

#include "../libraries/imgui/imgui.h"
#include "../libraries/imgui/backends/imgui_impl_glfw.h"
#include "../libraries/imgui/backends/imgui_impl_vulkan.h"

#include "../system.h"

GUI::~GUI() { }
GUI::GUI() { }

void GUI::cleanup() { m_cleaner.flush(); }

void GUI::setWindow(Window* window) { m_pWindow = window; }

void GUI::init(VkRenderPass renderPass) {
    LOG("GUI::init");
    System &system  = System::instance();
    VkInstance       instance       = system.getRenderer()->m_instance;
    VkDevice         device         = system.getRenderer()->m_device;
    VkPhysicalDevice physicalDevice = system.getRenderer()->m_physicalDevice;
    VkQueue          graphicQueue   = system.getRenderer()->m_graphicQueue;
    Commander*       commander      = system.getCommander();
    
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

void GUI::draw() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::ShowDemoWindow();
    
    ImGui::Render();
}

void GUI::render(VkCommandBuffer commandBuffer) {
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}
