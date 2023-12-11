#include "Renderer.h"
#include "Instance.h"
#include "ShaderModule.h"
#include "Vertex.h"
#include "Camera.h"

#include "Descriptor.h"

#include <filesystem>

#define USE_UI 1

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

//#if USE_UI
    CreateUI();
//#endif

    CreateFrameResources();
    CreateModels();
    CreateDescriptors();
    CreatePipelines();

    commandBuffers.resize(swapChain->GetCount());
    //RecordCommandBuffers();
    //RecordComputeCommandBuffer();
}

void Renderer::UpdateUIBuffer() {
    memcpy(uiControlBuffer.mappedData, &uiControlBufferObject, sizeof(UIControlBufferObject));
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
    Descriptor::CreateUIParamDescriptorSetLayout(logicalDevice);

    Descriptor::CreateDescriptorPool(logicalDevice, scene);

    // Storage image - cur, prev, near cloud
    Descriptor::CreateImageStorageDescriptorSet(logicalDevice, imageCurTexture, Descriptor::imageCurDescriptorSet);
    // Descriptor::CreateImageStorageDescriptorSet(logicalDevice, imagePrevTexture, Descriptor::imagePrevDescriptorSet);

    // Storage and sampling image - Light Grid
    Descriptor::CreateImageStorageDescriptorSet(logicalDevice, lightGridTexture, Descriptor::lightGridDescriptorSet);
    Descriptor::CreateImageDescriptorSet(logicalDevice, lightGridTexture, Descriptor::lightGridSamplerDescriptorSet);

    // Storage and sampling image -  Near cloud
    Descriptor::CreateImageStorageDescriptorSet(logicalDevice, nearCloudColorTexture, Descriptor::nearCloudColorDescriptorSet);
    Descriptor::CreateImageDescriptorSet(logicalDevice, nearCloudColorTexture, Descriptor::nearCloudColorSamplerDescriptorSet);
    Descriptor::CreateImageStorageDescriptorSet(logicalDevice, nearCloudDensityTexture, Descriptor::nearCloudDensityDescriptorSet);
    Descriptor::CreateImageDescriptorSet(logicalDevice, nearCloudDensityTexture, Descriptor::nearCloudDensitySamplerDescriptorSet);
    
    // Image - frame
    Descriptor::CreateImageDescriptorSet(logicalDevice, imageCurTexture, Descriptor::frameDescriptorSet);

    // Image - Compute shader images
    Descriptor::CreateComputeImagesDescriptorSet(logicalDevice, lowResCloudShapeTexture, hiResCloudShapeTexture, weatherMapTexture, curlNoiseTexture);

    // Image - Compute Nubis Cubed shader images
    Descriptor::CreateComputeNubisCubedImagesDescriptorSet(logicalDevice, modelingDataParkourTexture, modelingDataStormBirdTexture, cloudDetailNoiseTexture);

    // Camera
    Descriptor::CreateCameraDescriptorSet(logicalDevice, camera);

    // Scene (Time) 
    Descriptor::CreateSceneDescriptorSet(logicalDevice, scene);

    // UI
    Descriptor::CreateUIParamDescriptorSet(logicalDevice, uiControlBuffer.buffer, sizeof(UIControlBufferObject));
}

void Renderer::CreatePipelines() {
    backgroundShader = new PostShader(device, swapChain, &renderPass, "shaders/post.vert.spv", "shaders/tone.frag.spv");
    // reprojectShader = new ReprojectShader(device, swapChain, &renderPass);
    computeShader = new ComputeShader(device, swapChain, &renderPass);
    computeNubisCubedShader = new ComputeNubisCubedShader(device, swapChain, &renderPass);
    computeLightGridShader = new ComputeLightGridShader(device, swapChain, &renderPass);
    computeNearShader = new ComputeNearShader(device, swapChain, &renderPass);
    computeFarShader = new ComputeFarShader(device, swapChain, &renderPass);
}

void Renderer::CreateFrameResources() {
    imageViews.resize(swapChain->GetCount());

    const std::filesystem::path src_dir = std::filesystem::path(PROJECT_DIRECTORY);

    // CREATE CUSTOM TEXTURES
    depthTexture = Image::CreateDepthTexture(device, graphicsCommandPool, swapChain->GetVkExtent()); // Special for depth texture

    // Two ping pong images for reprojection and compute
    imageCurTexture = Image::CreateStorageTexture(device, graphicsCommandPool, swapChain->GetVkExtent());
    // imagePrevTexture = Image::CreateStorageTexture(device, graphicsCommandPool, swapChain->GetVkExtent());

    // Create images to sample in the shader
    hiResCloudShapeTexture = Image::CreateTexture3DFromFiles(device, graphicsCommandPool, (src_dir / "images/hiResCloudShape/hiResClouds ").string().c_str(), glm::ivec3(32, 32, 32));
    lowResCloudShapeTexture = Image::CreateTexture3DFromFiles(device, graphicsCommandPool, (src_dir / "images/lowResCloudShape/lowResCloud").string().c_str(), glm::ivec3(128, 128, 128));
    weatherMapTexture = Image::CreateTextureFromFile(device, graphicsCommandPool, (src_dir / "images/weather.png").string().c_str());
    curlNoiseTexture = Image::CreateTextureFromFile(device, graphicsCommandPool, (src_dir / "images/curlNoise.png").string().c_str());

    // modelingDataTexture = Image::CreateTextureFromVDBFile(device, graphicsCommandPool, "images/vdb/example2/StormbirdCloud.vdb");
    
    modelingDataParkourTexture = Image::CreateTexture3DFromFiles(device, graphicsCommandPool, (src_dir / "images/vdb/example1/tga/modeling_data").string().c_str(), glm::ivec3(512, 512, 64));
    modelingDataStormBirdTexture = Image::CreateTexture3DFromFiles(device, graphicsCommandPool, (src_dir / "images/vdb/example2/tga/modeling_data").string().c_str(), glm::ivec3(512, 512, 64));
    // fieldDataTexture = Image::CreateTexture3DFromFiles(device, graphicsCommandPool, (src_dir / "images/vdb/example2/tga/field_data").string().c_str(), glm::ivec3(512, 512, 64));
    cloudDetailNoiseTexture = Image::CreateTexture3DFromFiles(device, graphicsCommandPool, (src_dir / "images/noise/tga/NubisVoxelCloudNoise").string().c_str(), glm::ivec3(128, 128, 128));

    // Light grid 
    lightGridTexture = Image::CreateStorageTexture3D(device, graphicsCommandPool, glm::ivec3(256, 256, 32));

    // Near Cloud 
    nearCloudColorTexture = Image::CreateStorageTextureHalfRes(device, graphicsCommandPool, swapChain->GetVkExtent());
    nearCloudDensityTexture = Image::CreateStorageTextureHalfRes(device, graphicsCommandPool, swapChain->GetVkExtent());

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
    // imagePrevTexture->CleanUp(logicalDevice);
    // delete imagePrevTexture;
    hiResCloudShapeTexture->CleanUp(logicalDevice);
    delete hiResCloudShapeTexture;
    lowResCloudShapeTexture->CleanUp(logicalDevice);
    delete lowResCloudShapeTexture;
    weatherMapTexture->CleanUp(logicalDevice);
    delete weatherMapTexture;
    curlNoiseTexture->CleanUp(logicalDevice);
    delete curlNoiseTexture;
    modelingDataParkourTexture->CleanUp(logicalDevice);
    delete modelingDataParkourTexture;
    modelingDataStormBirdTexture->CleanUp(logicalDevice);
    delete modelingDataStormBirdTexture;
    cloudDetailNoiseTexture->CleanUp(logicalDevice);
	delete cloudDetailNoiseTexture;
    lightGridTexture->CleanUp(logicalDevice);
    delete lightGridTexture;
    nearCloudColorTexture->CleanUp(logicalDevice);
    delete nearCloudColorTexture;
    nearCloudDensityTexture->CleanUp(logicalDevice);
    delete nearCloudDensityTexture;

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
    // Specify the command pool and number of buffers to allocate
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = computeCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &computeCommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers");
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    // ~ Start recording ~
    if (vkBeginCommandBuffer(computeCommandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording compute command buffer");
    }

    // Reproject
    // reprojectShader->BindShaderProgram(computeCommandBuffers[i]);
    // const glm::ivec2 texDimsFull(swapChain->GetVkExtent().width, swapChain->GetVkExtent().height);
    // vkCmdDispatch(computeCommandBuffers[i],
    //     static_cast<uint32_t>((texDimsFull.x + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
    //     static_cast<uint32_t>((texDimsFull.y + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
    //     1);


    if (useNubisCubed == 1) {
        // Light Grid Compute Shader
        const glm::ivec3 lightVoxelDims(256, 256, 32);
        computeLightGridShader->BindShaderProgram(computeCommandBuffer);
        vkCmdDispatch(computeCommandBuffer,
            static_cast<uint32_t>((lightVoxelDims.x + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
            static_cast<uint32_t>((lightVoxelDims.y + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
            static_cast<uint32_t>((lightVoxelDims.z)));

        computeNearShader->BindShaderProgram(computeCommandBuffer);
        const glm::ivec2 texDimsPartial(swapChain->GetVkExtent().width, swapChain->GetVkExtent().height);
        vkCmdDispatch(computeCommandBuffer,
            static_cast<uint32_t>((texDimsPartial.x / 2 + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
            static_cast<uint32_t>((texDimsPartial.y / 2 + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
            1);

        computeFarShader->BindShaderProgram(computeCommandBuffer);
        vkCmdDispatch(computeCommandBuffer,
            static_cast<uint32_t>((texDimsPartial.x + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
            static_cast<uint32_t>((texDimsPartial.y + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
            1);

        /*
            computeNubisCubedShader->BindShaderProgram(computeCommandBuffer);
            const glm::ivec2 texDimsPartial(swapChain->GetVkExtent().width, swapChain->GetVkExtent().height);
            vkCmdDispatch(computeCommandBuffer,
                static_cast<uint32_t>((texDimsPartial.x + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
                static_cast<uint32_t>((texDimsPartial.y + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
                1);
        */
    } else {
        computeShader->BindShaderProgram(computeCommandBuffer);
        const glm::ivec2 texDimsFull(swapChain->GetVkExtent().width, swapChain->GetVkExtent().height);
        // const glm::ivec2 texDimsPartial(swapChain->GetVkExtent().width / 4, swapChain->GetVkExtent().height / 4);
        vkCmdDispatch(computeCommandBuffer,
            static_cast<uint32_t>((texDimsFull.x + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
            static_cast<uint32_t>((texDimsFull.y + WORKGROUP_SIZE - 1) / WORKGROUP_SIZE),
            1);
    }

    // ~ End recording ~
    if (vkEndCommandBuffer(computeCommandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record compute command buffer");
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

    ImGui::SetNextWindowSize(ImVec2(500.f, 420.f));
    ImGui::Begin("Control Panel", 0, ImGuiWindowFlags_None | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowFontScale(1);

    ImGui::Text("Current Frame Rate: %.1f", ImGui::GetIO().Framerate);
    ImGui::RadioButton("Nubis 2", &useNubisCubed, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Nubis 3", &useNubisCubed, 1);

    ImGui::Separator();
    ImGui::Text("Cloud Parameter");
    ImGui::SliderFloat("Tiling Frequency", &uiControlBufferObject.tiling_freq, 0.01f, 0.1f);

    ImGui::RadioButton("Parkouring Cloud", &uiControlBufferObject.cloud_type, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Stormbird Cloud", &uiControlBufferObject.cloud_type, 1);

    ImGui::Separator();
    ImGui::Text("Cloud Animation Parameter");
    ImGui::SliderFloat("Floating Speed", &uiControlBufferObject.animate_speed, 0.0f, 100.0f);
    // ImGui::SliderFloat3("Floating Offset", &uiControlBufferObject.animate_offset[0], -1000.0f, 1000.0f);

    ImGui::Separator();
    ImGui::Text("Ray Marching Parameter");
    ImGui::SliderFloat("Max Distance", &uiControlBufferObject.farclip, 0.0f, 5000.0f);
    ImGui::SliderFloat("Transmittance Limit", &uiControlBufferObject.transmittance_limit, 0.0f, 1.0f);

    ImGui::Separator();
    ImGui::Text("Post Processing Parameter");
    if (ImGui::Checkbox("Enable Godray", &enableGodray)) {
        uiControlBufferObject.enable_godray = enableGodray ? 1.0f : 0.0f;
    }
    ImGui::SliderFloat("Godray Exposure", &uiControlBufferObject.godray_exposure, 0.01f, 0.15f);

    ImGui::Separator();
    ImGui::Text("Envionment Parameter");

    ImGui::SliderFloat("Sky Turbidity", &uiControlBufferObject.sky_turbidity, 1.0f, 20.0f);

    if (ImGui::Checkbox("Custom Control Sun Angle", &customSunAngle)) {
        if (customSunAngle) angle = scene->GetTheta();
    }
    if (customSunAngle) {
        ImGui::SliderFloat("Sun Angle", &angle, 0.0f, 360.0f);
    }

    ImGui::SliderFloat("Camera Speed", &camera->getStepSize(), 0.0f, 200.f);

    ImGui::End();

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

        //// End render pass
        vkCmdEndRenderPass(commandBuffers[i]);

        // ~ End recording ~
        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer");
        }
    }
}

void Renderer::UpdateUniformBuffers() {
    scene->UpdateTime(customSunAngle, angle); // time
    camera->UpdatePrevBuffer(); // camera prev
    camera->UpdatePixelOffset(); // camera pixel offset
    UpdateUIBuffer(); // ui control parameters
}

void Renderer::Frame() {
    RecordComputeCommandBuffer();

    VkSubmitInfo computeSubmitInfo = {};
    computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    computeSubmitInfo.commandBufferCount = 1;
    computeSubmitInfo.pCommandBuffers = &computeCommandBuffer;
    // swap buffers

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

    VkSemaphore signalSemaphores[] = { swapChain->GetRenderFinishedVkSemaphore() };
    submitInfo.signalSemaphoreCount = 1;
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
    vkFreeCommandBuffers(logicalDevice, computeCommandPool, 1, &computeCommandBuffer);

    // Destroy descrioptors and shader programs
    Descriptor::CleanUp(logicalDevice);

    backgroundShader->CleanUp();
    delete backgroundShader;
    //reprojectShader->CleanUp();
    //delete reprojectShader;
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

    // Create UI buffer object
    uiControlBuffer.MapMemory(device, sizeof(UIControlBufferObject));
    memcpy(uiControlBuffer.mappedData, &uiControlBufferObject, sizeof(UIControlBufferObject));
}
