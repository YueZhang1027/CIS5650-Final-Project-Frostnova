#include "Renderer.h"
#include "Instance.h"
#include "ShaderModule.h"
#include "Vertex.h"
#include "Camera.h"

#include "Descriptor.h"

#define USE_UI 0

static constexpr unsigned int WORKGROUP_SIZE = 32;

Renderer::Renderer(GLFWwindow* window, Device* device, SwapChain* swapChain, Scene* scene, Camera* camera)
  : device(device),
    logicalDevice(device->GetVkDevice()),
    swapChain(swapChain),
    scene(scene),
    camera(camera),
    window(window) {

    CreateCommandPools();
    CreateRenderPass();
    CreateOffscreenRenderPass();

    // TODO: custom pipeline creation here
    CreateFrameResources();
    CreateModels();
    CreateDescriptors();
    CreatePipelines();

#if USE_UI
    CreateUI();
#endif

    commandBuffers.resize(swapChain->GetCount());
    //RecordCommandBuffers();
    RecordComputeCommandBuffer();
    //RecordOffscreenCommandBuffers();
}

void Renderer::CreateCommandPools() {
    VkCommandPoolCreateInfo graphicsPoolInfo = {};
    graphicsPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    graphicsPoolInfo.queueFamilyIndex = device->GetInstance()->GetQueueFamilyIndices()[QueueFlags::Graphics];
    graphicsPoolInfo.flags = 0;

    if (vkCreateCommandPool(logicalDevice, &graphicsPoolInfo, nullptr, &graphicsCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }

    VkCommandPoolCreateInfo computePoolInfo = {};
    computePoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    computePoolInfo.queueFamilyIndex = device->GetInstance()->GetQueueFamilyIndices()[QueueFlags::Compute];
    computePoolInfo.flags = 0;

    if (vkCreateCommandPool(logicalDevice, &computePoolInfo, nullptr, &computeCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }
}

void Renderer::CreateRenderPass() {
    // Color buffer attachment represented by one of the images from the swap chain
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChain->GetVkImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Create a color attachment reference to be used with subpass
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth buffer attachment
    VkFormat depthFormat = device->GetInstance()->GetSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Create a depth attachment reference
    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Create subpass description
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    // Specify subpass dependency
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Create render pass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass");
    }
}

void Renderer::CreateOffscreenRenderPass() {
    // Color buffer attachment represented by one of the images from the swap chain
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChain->GetVkImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Create a color attachment reference to be used with subpass
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth buffer attachment
    VkFormat depthFormat = device->GetInstance()->GetSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Create a depth attachment reference
    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Create subpass description
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

    // Create the actual renderpass
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &offscreenRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    // Create the offscreen framebuffer
    int renderPassCount = 2;
    offscreenTextures.resize((renderPassCount - 1) * 2);
    offscreenFramebuffers.resize(renderPassCount - 1);
    
    for (int i = 0; i < renderPassCount - 1; ++i) {
        offscreenTextures[i] = Image::CreateColorTexture(device, graphicsCommandPool, swapChain->GetVkExtent(), swapChain->GetVkImageFormat());
	    offscreenTextures[i + 1] = Image::CreateDepthTexture(device, graphicsCommandPool, swapChain->GetVkExtent());

        std::vector<VkImageView> attachments = {
			offscreenTextures[i]->imageView,
			offscreenTextures[i + 1]->imageView
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = offscreenRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChain->GetVkExtent().width;
		framebufferInfo.height = swapChain->GetVkExtent().height;
		framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &offscreenFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer");
		}
    }
}

void Renderer::CreateModels() {
    VkCommandPoolCreateInfo transferPoolInfo = {};
    transferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transferPoolInfo.queueFamilyIndex = device->GetInstance()->GetQueueFamilyIndices()[QueueFlags::Transfer];
    transferPoolInfo.flags = 0;

    VkCommandPool transferCommandPool;
    if (vkCreateCommandPool(device->GetVkDevice(), &transferPoolInfo, nullptr, &transferCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }

    backgroundQuad = new Model(device, transferCommandPool, ModelCreateFlags::BACKGROUND_QUAD);

    vkDestroyCommandPool(device->GetVkDevice(), transferCommandPool, nullptr);
}

void Renderer::CreateDescriptors() {
    Descriptor::CreateImageStorageDescriptorSetLayout(logicalDevice);
    Descriptor::CreateImageDescriptorSetLayout(logicalDevice);
    Descriptor::CreateCameraDescriptorSetLayout(logicalDevice);
    Descriptor::CreateComputeImagesDescriptorSetLayout(logicalDevice);
    Descriptor::CreateComputeNubisCubedImagesDescriptorSetLayout(logicalDevice);
    Descriptor::CreateSceneDescriptorSetLayout(logicalDevice);

    Descriptor::CreateDescriptorPool(logicalDevice, scene);

    // Storage image - cur, prev
    Descriptor::CreateImageStorageDescriptorSet(logicalDevice, imageCurTexture, Descriptor::imageCurDescriptorSet);
    Descriptor::CreateImageStorageDescriptorSet(logicalDevice, imagePrevTexture, Descriptor::imagePrevDescriptorSet);

    // Storage image - Light Grid
    Descriptor::CreateImageStorageDescriptorSet(logicalDevice, lightGridTexture, Descriptor::lightGridDescriptorSet);
    
    // Image - frame
    Descriptor::CreateImageDescriptorSet(logicalDevice, imageCurTexture, Descriptor::frameDescriptorSet);

    // Image - Compute shader images
    Descriptor::CreateComputeImagesDescriptorSet(logicalDevice, lowResCloudShapeTexture, hiResCloudShapeTexture, weatherMapTexture, curlNoiseTexture);

    // Image - Compute Nubis Cubed shader images
    Descriptor::CreateComputeNubisCubedImagesDescriptorSet(logicalDevice, modelingDataTexture, fieldDataTexture, cloudDetailNoiseTexture);

    // Camera
    Descriptor::CreateCameraDescriptorSet(logicalDevice, camera);

    // Scene (Time) 
    Descriptor::CreateSceneDescriptorSet(logicalDevice, scene);
}

void Renderer::CreatePipelines() {
    backgroundShader = new PostShader(device, swapChain, &renderPass, "shaders/post.vert.spv", "shaders/tone.frag.spv");
    reprojectShader = new ReprojectShader(device, swapChain, &renderPass);
    computeShader = new ComputeShader(device, swapChain, &renderPass);
    computeNubisCubedShader = new ComputeNubisCubedShader(device, swapChain, &renderPass);
    computeLightGridShader = new ComputeLightGridShader(device, swapChain, &renderPass);
}

void Renderer::CreateFrameResources() {
    imageViews.resize(swapChain->GetCount());

    // CREATE CUSTOM TEXTURES
    depthTexture = Image::CreateDepthTexture(device, graphicsCommandPool, swapChain->GetVkExtent()); // Special for depth texture

    // Two ping pong images for reprojection and compute
    imageCurTexture = Image::CreateStorageTexture(device, graphicsCommandPool, swapChain->GetVkExtent());
    imagePrevTexture = Image::CreateStorageTexture(device, graphicsCommandPool, swapChain->GetVkExtent());

    // Create images to sample in the shader
    hiResCloudShapeTexture = Image::CreateTexture3DFromFiles(device, graphicsCommandPool, "images/hiResClouds ", glm::ivec3(32, 32, 32));
    lowResCloudShapeTexture = Image::CreateTexture3DFromFiles(device, graphicsCommandPool, "images/lowResCloud", glm::ivec3(128, 128, 128));
    weatherMapTexture = Image::CreateTextureFromFile(device, graphicsCommandPool, "images/weather.png");
    curlNoiseTexture = Image::CreateTextureFromFile(device, graphicsCommandPool, "images/curlNoise.png");

    // modelingDataTexture = Image::CreateTextureFromVDBFile(device, graphicsCommandPool, "images/vdb/example2/StormbirdCloud.vdb");
    modelingDataTexture = Image::CreateTexture3DFromFiles(device, graphicsCommandPool, "images/modeling_data", glm::ivec3(512, 512, 64));
    fieldDataTexture = Image::CreateTexture3DFromFiles(device, graphicsCommandPool, "images/field_data", glm::ivec3(512, 512, 64));
    cloudDetailNoiseTexture = Image::CreateTexture3DFromFiles(device, graphicsCommandPool, "images/NubisVoxelCloudNoise", glm::ivec3(128, 128, 128));

    // Light grid 
    lightGridTexture = Image::CreateStorageTexture3D(device, graphicsCommandPool, glm::ivec3(512, 512, 64));

    for (uint32_t i = 0; i < swapChain->GetCount(); i++) {
        // --- Create an image view for each swap chain image ---
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChain->GetVkImage(i);

        // Specify how the image data should be interpreted
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChain->GetVkImageFormat();

        // Specify color channel mappings (can be used for swizzling)
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // Describe the image's purpose and which part of the image should be accessed
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        // Create the image view
        if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image views");
        }
    }
    
    // CREATE FRAMEBUFFERS
    framebuffers.resize(swapChain->GetCount());
    for (size_t i = 0; i < swapChain->GetCount(); i++) {
        std::vector<VkImageView> attachments = {
            imageViews[i],
            depthTexture->imageView
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChain->GetVkExtent().width;
        framebufferInfo.height = swapChain->GetVkExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer");
        }

    }
}

void Renderer::DestroyFrameResources() {
    for (size_t i = 0; i < imageViews.size(); i++) {
        vkDestroyImageView(logicalDevice, imageViews[i], nullptr);
    }

    depthTexture->CleanUp(logicalDevice);
    delete depthTexture;
    imageCurTexture->CleanUp(logicalDevice);
    delete imageCurTexture;
    imagePrevTexture->CleanUp(logicalDevice);
    delete imagePrevTexture;
    hiResCloudShapeTexture->CleanUp(logicalDevice);
    delete hiResCloudShapeTexture;
    lowResCloudShapeTexture->CleanUp(logicalDevice);
    delete lowResCloudShapeTexture;
    weatherMapTexture->CleanUp(logicalDevice);
    delete weatherMapTexture;
    curlNoiseTexture->CleanUp(logicalDevice);
    delete curlNoiseTexture;
    modelingDataTexture->CleanUp(logicalDevice);
    delete modelingDataTexture;

    for (size_t i = 0; i < framebuffers.size(); i++) {
        vkDestroyFramebuffer(logicalDevice, framebuffers[i], nullptr);
    }
}

void Renderer::RecreateFrameResources() {
    backgroundShader->CleanUp();
    vkFreeCommandBuffers(logicalDevice, graphicsCommandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    DestroyFrameResources();
    CreateFrameResources();

    backgroundShader->CreateShaderProgram();
    commandBuffers.resize(swapChain->GetCount());
    //RecordCommandBuffers();
}

void Renderer::RecordComputeCommandBuffer() {
    computeCommandBuffers.resize(2);
    // Specify the command pool and number of buffers to allocate
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = computeCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)computeCommandBuffers.size();

    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, computeCommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers");
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    // Ping pong between the two buffers
    for (int i = 0; i < 2; ++i) {
        // ~ Start recording ~
        if (vkBeginCommandBuffer(computeCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording compute command buffer");
        }

        // Reproject
        // reprojectShader->BindShaderProgram(computeCommandBuffers[i]);
        // const glm::ivec2 texDimsFull(swapChain->GetVkExtent().width, swapChain->GetVkExtent().height);
        // vkCmdDispatch(computeCommandBuffers[i],
        //     static_cast<uint32_t>((texDimsFull.x + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
        //     static_cast<uint32_t>((texDimsFull.y + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
        //     1);
        // 
        // computeShader->BindShaderProgram(computeCommandBuffers[i]);
        // const glm::ivec2 texDimsPartial(swapChain->GetVkExtent().width / 4, swapChain->GetVkExtent().height / 4);
        // vkCmdDispatch(computeCommandBuffers[i],
        //     static_cast<uint32_t>((texDimsPartial.x + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
        //     static_cast<uint32_t>((texDimsPartial.y + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
        //     1);

        // Light Grid Compute Shader
        computeLightGridShader->BindShaderProgram(computeCommandBuffers[i]);
        vkCmdDispatch(computeCommandBuffers[i], 512, 512, 64);

        computeNubisCubedShader->BindShaderProgram(computeCommandBuffers[i]);
        const glm::ivec2 texDimsPartial(swapChain->GetVkExtent().width, swapChain->GetVkExtent().height);
        vkCmdDispatch(computeCommandBuffers[i],
            static_cast<uint32_t>((texDimsPartial.x + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
            static_cast<uint32_t>((texDimsPartial.y + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
            1);

        // ~ End recording ~
        if (vkEndCommandBuffer(computeCommandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record compute command buffer");
        }
    }
}

void Renderer::RecordCommandBuffer(uint32_t index) {
    // Specify the command pool and number of buffers to allocate
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = graphicsCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers");
    }

    // Start command buffer recording
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    // ~ Start recording ~
    if (vkBeginCommandBuffer(commandBuffers[index], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer");
    }

    // Begin the render pass
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffers[index];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChain->GetVkExtent();

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffers[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    // Bind the graphics pipeline
    backgroundShader->BindShaderProgram(commandBuffers[index]);
    backgroundQuad->EnqueueDrawCommands(commandBuffers[index]);

#if USE_UI
    // UI
    mouseOverImGuiWindow = io->WantCaptureMouse;
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[index]);
#endif

    //// End render pass
    vkCmdEndRenderPass(commandBuffers[index]);

    // ~ End recording ~
    if (vkEndCommandBuffer(commandBuffers[index]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer");
    }
    
}

void Renderer::RecordCommandBuffers() {
    commandBuffers.resize(swapChain->GetCount());

    // Specify the command pool and number of buffers to allocate
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = graphicsCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers");
    }

    // Start command buffer recording
    for (size_t i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        // ~ Start recording ~
        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer");
        }

        // Begin the render pass
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = framebuffers[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChain->GetVkExtent();

        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        // Bind the graphics pipeline
        backgroundShader->BindShaderProgram(commandBuffers[i]);
        backgroundQuad->EnqueueDrawCommands(commandBuffers[i]);

#if USE_UI
        // UI
        mouseOverImGuiWindow = io->WantCaptureMouse;
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[i]);
#endif

        //// End render pass
        vkCmdEndRenderPass(commandBuffers[i]);

        // ~ End recording ~
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer");
        }
    }
}

void Renderer::RecordOffscreenCommandBuffers() {
    offscreenCommandBuffers.resize(2);

	// Specify the command pool and number of buffers to allocate
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = graphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(offscreenCommandBuffers.size());

    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, offscreenCommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers");
	}

	// Start command buffer recording
    for (size_t i = 0; i < offscreenCommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		// ~ Start recording ~
        if (vkBeginCommandBuffer(offscreenCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer");
		}

		// Begin the render pass
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = offscreenRenderPass;
		renderPassInfo.framebuffer = offscreenFramebuffers[0];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChain->GetVkExtent();

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(offscreenCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		// Bind the graphics pipeline

        vkCmdEndRenderPass(offscreenCommandBuffers[i]);

        // Use the next framebuffer in the offscreen pass

        // ~ End recording ~
        if (vkEndCommandBuffer(offscreenCommandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer");
        }
    }
}

void Renderer::UpdateUniformBuffers() {
    scene->UpdateTime(); // time
    camera->UpdatePrevBuffer(); // camera prev
    camera->UpdatePixelOffset(); // camera pixel offset
}

void Renderer::Frame() {
    VkSubmitInfo computeSubmitInfo = {};
    computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    computeSubmitInfo.commandBufferCount = 1;
    computeSubmitInfo.pCommandBuffers = &computeCommandBuffers[(swapBackground ? 1 : 0)];
    // swap buffers
    swapBackground = !swapBackground;

    if (vkQueueSubmit(device->GetQueue(QueueFlags::Compute), 1, &computeSubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    if (!swapChain->Acquire()) {
        RecreateFrameResources();
        return;
    }

    RecordCommandBuffer(swapChain->GetIndex());

    // Submit the command buffer
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { swapChain->GetImageAvailableVkSemaphore() };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    // Do all offscreen rendering
    /*VkSemaphore offscreenSemaphores[] = {swapChain->GetOffscreenFinishedVkSemaphore()};
    submitInfo.pSignalSemaphores = offscreenSemaphores;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &offscreenCommandBuffers[(swapBackground ? 1 : 0)];

    if (vkQueueSubmit(device->GetQueue(QueueFlags::Graphics), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }*/

    VkSemaphore signalSemaphores[] = { swapChain->GetRenderFinishedVkSemaphore() };
    submitInfo.signalSemaphoreCount = 1;
    //submitInfo.pWaitSemaphores = offscreenSemaphores;
    submitInfo.pSignalSemaphores = signalSemaphores;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[swapChain->GetIndex()];

    if (vkQueueSubmit(device->GetQueue(QueueFlags::Graphics), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    if (!swapChain->Present()) {
        RecreateFrameResources();
    }
}

Renderer::~Renderer() {
#if USE_UI
    // UI cleanup
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroyDescriptorPool(logicalDevice, uiDescriptorPool, nullptr);
#endif

    vkDeviceWaitIdle(logicalDevice);

    // TODO: destroy any resources you created
    vkFreeCommandBuffers(logicalDevice, graphicsCommandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    vkFreeCommandBuffers(logicalDevice, computeCommandPool, static_cast<uint32_t>(computeCommandBuffers.size()), computeCommandBuffers.data());

    // Destroy descrioptors and shader programs
    Descriptor::CleanUp(logicalDevice);

    backgroundShader->CleanUp();
    delete backgroundShader;
    reprojectShader->CleanUp();
    delete reprojectShader;
    computeShader->CleanUp();
    delete computeShader;
    computeNubisCubedShader->CleanUp();
    delete computeNubisCubedShader;
    computeLightGridShader->CleanUp();
    delete computeLightGridShader;

    vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
    DestroyFrameResources();
    vkDestroyCommandPool(logicalDevice, computeCommandPool, nullptr);
    vkDestroyCommandPool(logicalDevice, graphicsCommandPool, nullptr);
}

// UI section
void Renderer::CreateUI() {
    // Create UI descriptor pool
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapChain->GetCount() },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, swapChain->GetCount() * 2 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = swapChain->GetCount() * 2;
    pool_info.poolSizeCount = static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes));
    pool_info.pPoolSizes = pool_sizes;
    if (vkCreateDescriptorPool(logicalDevice, &pool_info, nullptr, &uiDescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Cannot allocate UI descriptor pool!");
    }

    ImGui::CreateContext();
    io = &ImGui::GetIO();

    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = device->GetInstance()->GetVkInstance();
    init_info.PhysicalDevice = device->GetInstance()->GetPhysicalDevice();
    init_info.Device = device->GetVkDevice();
    init_info.ImageCount = swapChain->GetCount();
    init_info.MinImageCount = 2;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.QueueFamily = device->GetInstance()->GetQueueFamilyIndices()[QueueFlags::Graphics];
    init_info.Queue = device->GetQueue(QueueFlags::Graphics);
    init_info.DescriptorPool = uiDescriptorPool;

    ImGui_ImplVulkan_Init(&init_info, renderPass);
}
