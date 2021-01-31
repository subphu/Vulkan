//  Copyright © 2020 Subph. All rights reserved.
//

#include "renderer.h"

Renderer::Renderer() { }
Renderer::~Renderer() { cleanUp(); }

void Renderer::cleanUp() {
    vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

void Renderer::setupValidation(bool isEnable) {
    {
        if (!isEnable) return;
        USE_FUNC(DebugCallback);
    }
    m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
    m_debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    m_debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    m_debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    m_debugInfo.pfnUserCallback = DebugCallback;
    {
        CHECK_BOOL(CheckLayerSupport(m_validationLayers), "validation layers requested, but not available!");
        CHECK_ZERO(m_validationLayers.size(), "validation layers empty!");
        CHECK_ZERO(m_debugInfo.sType, "debug info empty!");
    }
}

void Renderer::createInstance(std::vector<const char*> extensions) {
    {
        CHECK_ZERO(extensions.size(), "extension empty!");
        USE_VAR(m_debugInfo);
        USE_VAR(m_validationLayers);
    }
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = extensions.data();
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
    instanceInfo.ppEnabledLayerNames = m_validationLayers.data();
    instanceInfo.pNext = &m_debugInfo;
 
    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_instance);
    {
        CHECK_VKRESULT(result, "failed to create vulkan instance!");
        CHECK_HANDLE(m_instance, "instance undefined!");
        std::cout << "vulkan instance created!\n";
    }
}

void Renderer::createDebugMessenger() {
    {
        USE_VAR(m_debugInfo);
        CHECK_HANDLE(m_instance, "instance undefined!");
    }
    VkResult result = CreateDebugUtilsMessengerEXT(m_instance, &m_debugInfo, nullptr, &m_debugMessenger);
    {
        CHECK_VKRESULT(result, "failed to set up debug messenger!");
        CHECK_HANDLE(m_debugMessenger, "debug messenger undefined!");
    }
}

void Renderer::setDeviceExtensions() {
    m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    CHECK_ZERO(m_deviceExtensions.size(), "device extensions empty!");
}

void Renderer::setSurface(VkSurfaceKHR* pSurface) { m_surface = *pSurface; }

void Renderer::pickPhysicalDevice() {
    {
        CHECK_HANDLE(m_instance, "instance undefined!");
        CHECK_HANDLE(m_surface, "surface undefined!");
        CHECK_ZERO(m_deviceExtensions.size(), "device extensions empty!");
    }
    std::vector<VkPhysicalDevice> physicalDevices = GetPhysicalDevices(m_instance);
    CHECK_ZERO(physicalDevices.size(), "failed to find GPUs with Vulkan support!");
    
    for (const auto& tempDevice : physicalDevices) {
        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(tempDevice, &supportedFeatures);
        m_surfaceFormats       = GetSurfaceFormatKHR(tempDevice, m_surface);
        m_presentModes         = GetSurfaceModeKHR(tempDevice, m_surface);
        int graphicFamilyIndex = FindGraphicFamilyIndex(tempDevice);
        int presentFamilyIndex = FindPresentFamilyIndex(tempDevice, m_surface);
        
        bool swapChainAdequate  = !m_surfaceFormats.empty() && !m_presentModes.empty();
        bool hasFamilyIndex     = graphicFamilyIndex > -1 && presentFamilyIndex > -1;
        bool extensionSupported = CheckDeviceExtensionSupport(tempDevice, m_deviceExtensions);
        
        if (swapChainAdequate && hasFamilyIndex && extensionSupported &&
            supportedFeatures.samplerAnisotropy) {
            m_physicalDevice = tempDevice;
            m_graphicFamilyIndex = graphicFamilyIndex;
            m_presentFamilyIndex = presentFamilyIndex;
            break;
        }
    }
    {
        CHECK_MINUS(m_graphicFamilyIndex, "graphics family index not found");
        CHECK_MINUS(m_presentFamilyIndex, "present family index not found");
        CHECK_MINUS(m_surfaceFormats.size(), "surface formats are empty");
        CHECK_MINUS(m_presentModes.size(), "surface present modes are empty");
        CHECK_HANDLE(m_physicalDevice, "failed to find a suitable GPU!");
    }
}

void Renderer::createLogicalDevice() {
    {
        CHECK_HANDLE(m_physicalDevice, "physical device undefined!");
        CHECK_HANDLE(m_surface, "surface undefined!");
        CHECK_ZERO(m_deviceExtensions.size(), "device extensions empty!");
        USE_VAR(m_validationLayers);
    }
    std::set<uint32_t> queueFamilyIndices = {m_graphicFamilyIndex, m_presentFamilyIndex};
    std::vector<VkDeviceQueueCreateInfo> queueInfos;
    float queuePriority = 1.f;
    for (uint32_t familyIndex : queueFamilyIndices) {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = familyIndex;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;
        queueInfos.push_back(queueInfo);
    }
    
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    
    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
    deviceInfo.pQueueCreateInfos = queueInfos.data();
    deviceInfo.pEnabledFeatures = &deviceFeatures;
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames = m_deviceExtensions.data();
    deviceInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
    deviceInfo.ppEnabledLayerNames = m_validationLayers.data();
    
    VkResult result = vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_device);
    {
        CHECK_VKRESULT(result, "failed to create logical device");
        CHECK_HANDLE(m_device, "logical device undefined!");
    }
}

void Renderer::createDeviceQueue() {
    vkGetDeviceQueue(m_device, m_graphicFamilyIndex, 0, &m_graphicQueue);
    vkGetDeviceQueue(m_device, m_presentFamilyIndex, 0, &m_presentQueue);
}

void Renderer::createCommandPool() {
    {
        CHECK_HANDLE(m_device, "logical device undefined!");
        USE_VAR(m_graphicFamilyIndex);
    }
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_graphicFamilyIndex;
    poolInfo.flags = 0;
     
    VkResult result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
    {
        CHECK_VKRESULT(result, "failed to create command pool!");
        CHECK_HANDLE(m_commandPool, "command pool undefined!");
    }
   
}

void Renderer::createSwapChain(Size<int> windowSize) {
    {
        CHECK_HANDLE(m_device, "logical device undefined!");
        CHECK_HANDLE(m_physicalDevice, "physical device undefined!");
        CHECK_HANDLE(m_surface, "surface undefined!");
        CHECK_MINUS(m_surfaceFormats.size(), "surface formats are empty");
        CHECK_MINUS(m_presentModes.size(), "surface present modes are empty");
        CHECK_MINUS(m_graphicFamilyIndex, "graphics family index not found");
        CHECK_MINUS(m_presentFamilyIndex, "present family index not found");
    }
    
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    
    VkExtent2D extent = ChooseSwapExtent(capabilities, windowSize);
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(m_surfaceFormats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(m_presentModes);
    
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    uint32_t queueFamilyIndices[] = {m_graphicFamilyIndex, m_presentFamilyIndex};
    if (m_graphicFamilyIndex != m_presentFamilyIndex) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    
    VkResult result = vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain);
    m_swapChainExtent = extent;
    m_swapChainImages = GetSwapchainImagesKHR(m_device, m_swapChain);
    m_swapChainImageFormat = surfaceFormat.format;
    {
        LOG("createSwapChain");
        CHECK_VKRESULT(result, "failed to create swap chain!");
        CHECK_HANDLE(m_swapChain, "logical device undefined!");
        CHECK_ZERO(m_swapChainImages.size(), "swap chain images empty!");
        CHECK_ZERO(m_swapChainImageFormat, "swap chain images format undefined!");
        CHECK_ZERO(m_swapChainExtent.height, "swap chain extent size zero!");
        CHECK_ZERO(m_swapChainExtent.width, "swap chain extent size zero!");
    }
}

void Renderer::createImageViews() {
    {
        CHECK_ZERO(m_swapChainImages.size(), "swap chain images empty!");
        CHECK_ZERO(m_swapChainImageFormat, "swap chain images format undefined!");
    }
    
    m_swapChainImageViews.resize(m_swapChainImages.size());
    for (size_t i = 0; i < m_swapChainImages.size(); i++) {
        m_swapChainImageViews[i] = createImageView(m_swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
    
    { LOG("createImageViews");
      CHECK_ZERO(m_swapChainImageViews.size(), "swap chain image view empty!"); }
}

void Renderer::createRenderPass() {
    {
        CHECK_ZERO(m_swapChainImageFormat, "swap chain images format undefined!");
        CHECK_HANDLE(m_physicalDevice, "physical device undefined!");
    }
    
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = ChooseDepthFormat(m_physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstSubpass = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    VkResult result = vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass);
    {
        LOG("createRenderPass");
        CHECK_VKRESULT(result, "failed to create render pass!");
        CHECK_HANDLE(m_renderPass, "failed to create render pass!");
    }
}

void Renderer::createDescriptorSetLayout() {
    { CHECK_HANDLE(m_device, "logical device undefined!"); }
    
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;
            
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    VkResult result = vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout);
    {
        LOG("createDescriptorSetLayout");
        CHECK_VKRESULT(result, "failed to create descriptor set layout!");
        CHECK_HANDLE(m_descriptorSetLayout, "failed to create descriptor set layout!");
    }
}

VkShaderModule Renderer::createShaderModule(const std::vector<char> & code) {
    { CHECK_HANDLE(m_device, "logical device undefined!"); }
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    
    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule);
    {
        CHECK_VKRESULT(result, "failed to create shader modul!");
        return shaderModule;
    }
}

VkPipelineShaderStageCreateInfo Renderer::createShaderStageInfo(const std::string& filename, VkShaderStageFlagBits stage) {
    auto shaderCode = readFile(filename);
    VkShaderModule shaderModule = createShaderModule(shaderCode);
    
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo.stage = stage;
    shaderStageInfo.module = shaderModule;
    shaderStageInfo.pName = "main";
    
    return shaderStageInfo;
}

void Renderer::createPipelineLayout(){
    {
        CHECK_HANDLE(m_descriptorSetLayout, "descriptor set layout not defined!");
        CHECK_HANDLE(m_device, "logical device undefined!");
    }
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    
    VkResult result = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
    {
        LOG("createPipelineLayout");
        CHECK_VKRESULT(result, "failed to create pipeline layout!");
        CHECK_HANDLE(m_pipelineLayout, "failed to create pipeline layout!");
    }
}

void Renderer::createGraphicsPipeline(VkPipelineShaderStageCreateInfo* shaderStages, VkPipelineVertexInputStateCreateInfo* vertexInputInfo) {
    {
        CHECK_ZERO(m_swapChainExtent.height, "swap chain extent size zero!");
        CHECK_ZERO(m_swapChainExtent.width, "swap chain extent size zero!");
        CHECK_HANDLE(m_pipelineLayout, "pipeline layout undefined!");
        CHECK_HANDLE(m_renderPass, "render pass undefined!");
        CHECK_HANDLE(m_device, "logical device undefined!");
    }
    
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) m_swapChainExtent.width;
    viewport.height = (float) m_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChainExtent;
    
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToOneEnable = VK_FALSE;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;
    
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    
    VkResult result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline);
    {
        LOG("createGraphicsPipeline");
        CHECK_VKRESULT(result, "failed to create graphics pipeline!");
        CHECK_HANDLE(m_graphicsPipeline, "failed to create graphics pipeline!");
    }
    
}

void Renderer::createDepthResources() {
    {
        CHECK_HANDLE(m_physicalDevice, "physical device undefined!");
        CHECK_ZERO(m_swapChainExtent.height, "swap chain extent size zero!");
        CHECK_ZERO(m_swapChainExtent.width, "swap chain extent size zero!");
        USE_VAR(m_mipLevels);
    }
    
    VkFormat depthFormat = ChooseDepthFormat(m_physicalDevice);
    m_depthImage = createImage(m_swapChainExtent.width,
                               m_swapChainExtent.height,
                               m_mipLevels,
                               ChooseDepthFormat(m_physicalDevice),
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_depthImageMemory = bindImageMemory(m_depthImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    m_depthImageView = createImageView(m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    
    {
        LOG("createDepthResources");
        CHECK_HANDLE(m_depthImage, "failed to create depth image!");
        CHECK_HANDLE(m_depthImageMemory, "failed to create depth image memory!");
        CHECK_HANDLE(m_depthImageView, "failed to create depth image view!");
    }
}

void Renderer::createFramebuffer() {
    {
        LOG("createFramebuffer");
        CHECK_HANDLE(m_device, "logical device undefined!");
        CHECK_ZERO(m_swapChainImageViews.size(), "swap chain image view empty!");
        CHECK_HANDLE(m_depthImageView, "depth image view undefined!");
        CHECK_HANDLE(m_renderPass, "render pass undefined!");
        CHECK_ZERO(m_swapChainExtent.height, "swap chain extent size zero!");
        CHECK_ZERO(m_swapChainExtent.width, "swap chain extent size zero!");
    }
    
    m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
    for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {m_swapChainImageViews[i], m_depthImageView};
        
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_swapChainExtent.width;
        framebufferInfo.height = m_swapChainExtent.height;
        framebufferInfo.layers = 1;
        
        VkResult result = vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]);
        CHECK_VKRESULT(result, "failed to create framebuffer!");
        
    }
    {
        CHECK_ZERO(m_swapChainFramebuffers.size(), "failed to create framebuffer!");
    }
    
}

// ==================================================


VkCommandBuffer Renderer::beginSingleTimeCommands() {
    {
        LOG("beginSingleTimeCommands");
        CHECK_HANDLE(m_commandPool, "command pool undefined!");
        CHECK_HANDLE(m_device, "logical device undefined!");
    }
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    {
        LOG("endSingleTimeCommands");
        CHECK_HANDLE(m_commandPool, "command pool undefined!");
        CHECK_HANDLE(m_device, "logical device undefined!");
        CHECK_HANDLE(m_graphicQueue, "graphic queue undefined!");
    }
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(m_graphicQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicQueue);
    
    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

VkBuffer Renderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage) {
    {
        LOG("createBuffer");
        CHECK_HANDLE(m_device, "logical device undefined!");
    }
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkBuffer buffer;
    VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer);
    {
        CHECK_VKRESULT(result, "failed to create buffer!");
        return buffer;
    }
}

VkDeviceMemory Renderer::allocateBufferMemory(VkBuffer& buffer, VkDeviceSize size, uint32_t memoryTypeIndex) {
    {
        LOG("allocateBufferMemory");
        CHECK_HANDLE(m_device, "logical device undefined!");
    }
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    
    VkDeviceMemory bufferMemory;
    VkResult result = vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory);
    {
        CHECK_VKRESULT(result, "failed to allocate buffer memory!");
        return bufferMemory;
    }
}

VkMemoryRequirements Renderer::getBufferMemoryRequirements(VkBuffer& buffer) {
    { LOG("getBufferMemoryRequirements");
      CHECK_HANDLE(m_device, "logical device undefined!"); }
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memoryRequirements);
    { return memoryRequirements; }
}

VkMemoryRequirements Renderer::getImageMemoryRequirements(VkImage& image) {
    { LOG("getImageMemoryRequirements");
      CHECK_HANDLE(m_device, "logical device undefined!"); }
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(m_device, image, &memoryRequirements);
    { return memoryRequirements; }
}

uint32_t Renderer::findMemoryTypeIdx(uint32_t typeFilter, VkMemoryPropertyFlags flags) {
    { LOG("findMemoryTypeIdx");
      CHECK_HANDLE(m_physicalDevice, "physical device undefined!"); }
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &properties);
    
    for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
        VkMemoryPropertyFlags propertyFlags = properties.memoryTypes[i].propertyFlags;
        if ((typeFilter & (1 << i)) && (propertyFlags & flags) == flags)
            return i;
    }
    { throw std::runtime_error("failed to find suitable memory type!"); }
}

VkImageView Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
    { LOG("createImageView");
      CHECK_HANDLE(m_device, "logical device undefined!"); }
    
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.image = image;
    viewInfo.format = format;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    
    VkImageView imageView;
    VkResult result = vkCreateImageView(m_device, &viewInfo, nullptr, &imageView);
    {
        CHECK_VKRESULT(result, "failed to create image views!");
        return imageView;
    }
}

VkImage Renderer::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage) {
    {
        LOG("createImage");
        CHECK_HANDLE(m_device, "logical device undefined!");
        CHECK_HANDLE(m_physicalDevice, "physical device undefined!");
    }
    VkImage image;
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.usage = usage;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkResult result = vkCreateImage(m_device, &imageInfo, nullptr, &image);
    {
        CHECK_VKRESULT(result, "failed to create image!");
        return image;
    }
}

VkDeviceMemory Renderer::bindImageMemory(VkImage image, VkMemoryPropertyFlags properties) {
    {
        LOG("bindImageMemory");
        CHECK_HANDLE(m_device, "logical device undefined!");
        CHECK_HANDLE(m_physicalDevice, "physical device undefined!");
    }
    VkDeviceMemory imageMemory;
    VkMemoryRequirements memRequirements = getImageMemoryRequirements(image);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(m_physicalDevice, memRequirements.memoryTypeBits, properties);
    
    VkResult result = vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory);
    {
        CHECK_VKRESULT(result, "failed to allocate image memory!");
        vkBindImageMemory(m_device, image, imageMemory, 0);
        return imageMemory;
    }
}

void Renderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
    LOG("transitionImageLayout");
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;
    
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
            format == VK_FORMAT_D24_UNORM_S8_UINT) // stencil format
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        throw std::runtime_error("unsupported layout transition!");
    }
    
    vkCmdPipelineBarrier(commandBuffer,
                         sourceStage, destinationStage,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
    
    endSingleTimeCommands(commandBuffer);
}

VkImage Renderer::createTextureImage(unsigned char* rawTexture, int width, int height, int channels, uint32_t mipLevels) {
    {
        LOG("createTextureImage");
        CHECK_HANDLE(m_device, "logical device undefined!");
    }
    VkDeviceSize imageSize = width * height * 4;
    
    VkBuffer tempBuffer = createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    VkMemoryRequirements requirements = getBufferMemoryRequirements(tempBuffer);
    uint32_t memoryTypeIdx = findMemoryTypeIdx(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkDeviceMemory tempBufferMemory = allocateBufferMemory(tempBuffer, requirements.size, memoryTypeIdx);
    vkBindBufferMemory(m_device, tempBuffer, tempBufferMemory, 0);
    
    void* data;
    vkMapMemory(m_device, tempBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, rawTexture, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_device, tempBufferMemory);
    
    VkImage textureImage = createImage(width,
                                       height,
                                       mipLevels,
                                       VK_FORMAT_R8G8B8A8_SRGB,
                                       VK_IMAGE_TILING_OPTIMAL,
                                       VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                       VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                       VK_IMAGE_USAGE_SAMPLED_BIT);
    VkDeviceMemory textureImageMemory = bindImageMemory(textureImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    
    copyBufferToImage(tempBuffer, textureImage, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    
    generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, width, height, mipLevels);
    
    vkDestroyBuffer(m_device, tempBuffer, nullptr);
    vkFreeMemory(m_device, tempBufferMemory, nullptr);
    
    {
        CHECK_HANDLE(textureImage, "failed to create texture image!");
        return textureImage;
    }
}

void Renderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    LOG("copyBufferToImage");
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};
    
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    
    endSingleTimeCommands(commandBuffer);
}

void Renderer::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
    {
        LOG("generateMipmaps");
        CHECK_HANDLE(m_device, "logical device undefined!");
        CHECK_HANDLE(m_physicalDevice, "physical device undefined!");
    }
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_physicalDevice, imageFormat, &formatProperties);
    
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }
    
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
        
        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        
        vkCmdBlitImage(commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);
        
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
        
        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }
    
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
    
    endSingleTimeCommands(commandBuffer);

}

VkSampler Renderer::createTextureSampler(uint32_t mipLevels) {
    {
        LOG("createTextureSampler");
        CHECK_HANDLE(m_device, "logical device undefined!");
    }
    VkSampler textureSampler;
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mipLevels);
    samplerInfo.mipLodBias = 0.0f;
    
    VkResult result = vkCreateSampler(m_device, &samplerInfo, nullptr, &textureSampler);
    {
        LOG("createTextureSampler");
        CHECK_VKRESULT(result, "failed to create texture sampler!");
        return textureSampler;
    }
}
