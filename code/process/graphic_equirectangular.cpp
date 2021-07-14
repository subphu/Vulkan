//  Copyright © 2021 Subph. All rights reserved.
//

#include "graphic_equirectangular.h"

#include "../system.h"

GraphicEquirectangular::~GraphicEquirectangular() {}
GraphicEquirectangular::GraphicEquirectangular() { LOG("GraphicEquirectangular::GraphicEquirectangular"); }

void GraphicEquirectangular::cleanup() { m_cleaner.flush(); }

void GraphicEquirectangular::createAssets()  {
    createHDR();
    createCube();
}

void GraphicEquirectangular::setup()  {
    createDescriptor();
    createPipeline();
}

void GraphicEquirectangular::recordCommand()  {
    VkCommandBuffer cmdBuffer = m_cmdBuffer;

    
}

void GraphicEquirectangular::draw()  {
    
}

// Private ==================================================

void GraphicEquirectangular::createHDR()  {
    std::string hdrPath = m_equirecPath;
    Image* pHDR = new Image();
    pHDR->setupForHDRTexture(hdrPath);
    pHDR->createForTexture();
    pHDR->copyRawHDRToImage();
    { m_pHDR = pHDR; }
}

void GraphicEquirectangular::createCube()  {
    Mesh* pMesh = new Mesh();
    pMesh->createCube();
    pMesh->cmdCreateVertexBuffer();
    pMesh->cmdCreateIndexBuffer();
    { m_pCube = pMesh; }
}

void GraphicEquirectangular::createCmdBuffer() {
    VkCommandBuffer cmdBuffer = System::Commander()->createCommandBuffer();
    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkResult result = vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo);
    CHECK_VKRESULT(result, "failed to begin recording command buffer!");
    { m_cmdBuffer = cmdBuffer; }
}

void GraphicEquirectangular::createRenderPass() {
    VkDevice device = System::Renderer()->getDevice();
    
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format          = VK_FORMAT_R32G32B32_SFLOAT;
    colorAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass      = 0;
    dependency.srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount  = 1;
    renderPassInfo.pAttachments     = &colorAttachment;
    renderPassInfo.subpassCount     = 1;
    renderPassInfo.pSubpasses       = &subpass;
    renderPassInfo.dependencyCount  = 1;
    renderPassInfo.pDependencies    = &dependency;
    
    VkRenderPass renderPass;
    VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
    CHECK_VKRESULT(result, "failed to create render pass!");
    
    { m_renderPass = renderPass; }
}

void GraphicEquirectangular::createFramebuffer() {
    VkDevice device = System::Renderer()->getDevice();
    VkRenderPass renderPass = m_renderPass;
    VkImageView  attachment = m_pRenderTarget->getImageView();
    uint size = m_size;

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.renderPass      = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments    = &attachment;
    framebufferInfo.width  = size;
    framebufferInfo.height = size;
    framebufferInfo.layers = 1;

    VkFramebuffer framebuffer;
    VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);
    CHECK_VKRESULT(result, "failed to create framebuffer!");
    
    { m_framebuffer = framebuffer; }
}

void GraphicEquirectangular::createDescriptor()  {
    
}

void GraphicEquirectangular::createPipeline()  {
    
}

