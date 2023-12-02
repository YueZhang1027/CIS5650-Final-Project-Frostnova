#include "GUIManager.h"

ImGuiIO* io = nullptr;
ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None | ImGuiWindowFlags_NoMove;
bool ui_hide = false;

VkDescriptorPool uiDescriptorPool = VK_NULL_HANDLE;
VkRenderPass uiRenderPass = VK_NULL_HANDLE;
VkCommandPool uiCommandPool = VK_NULL_HANDLE;
std::vector<VkCommandBuffer> uiCommandBuffers;
std::vector<VkFramebuffer> uiFramebuffers;

void ImGuiManager::InitUI(GLFWwindow* window, Device* device, SwapChain* swapChain, std::vector<VkImageView>& imageViews) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO(); (void)io;
	ImGui::StyleColorsLight(); // ImGui::StyleColorsDark();

	// Initialize some DearImgui specific resources
	CreateDescriptorPool(device);
	CreateUIRenderPass(device, swapChain->GetVkImageFormat());
	CreateUICommandPool(device);
	CreateUICommandBuffers(device, swapChain);
	CreateUIFramebuffers(device, swapChain, imageViews);
	
	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = device->GetInstance()->GetVkInstance();
	init_info.PhysicalDevice = device->GetInstance()->GetPhysicalDevice();
	init_info.Device = device->GetVkDevice();
	init_info.QueueFamily = device->GetQueueIndex(QueueFlags::Graphics);
	init_info.Queue = device->GetQueue(QueueFlags::Graphics);
	init_info.DescriptorPool = uiDescriptorPool;
	init_info.MinImageCount = 5;
	init_info.ImageCount = 5;
	ImGui_ImplVulkan_Init(&init_info, uiRenderPass);

	// VkCommandBuffer commandBuffer = beginSingleTimeCommands(uiCommandPool);
	// ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
	// endSingleTimeCommands(commandBuffer, uiCommandPool);
	// ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void ImGuiManager::RecreateUI(Device* device, SwapChain* swapChain, std::vector<VkImageView>& imageViews) {
	ImGui_ImplVulkan_SetMinImageCount(swapChain->GetCount());
	CreateUICommandBuffers(device, swapChain);
	CreateUIFramebuffers(device, swapChain, imageViews);
}

void ImGuiManager::CreateDescriptorPool(Device* device) {
	VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	if (vkCreateDescriptorPool(device->GetVkDevice(), &pool_info, nullptr, &uiDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool");
	}
}

void ImGuiManager::CreateUIRenderPass(Device* device, VkFormat imageFormat) {
	// Create an attachment description for the render pass
	VkAttachmentDescription attachmentDescription = {};
	attachmentDescription.format = imageFormat;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // Need UI to be drawn on top of main
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Last pass so we want to present after
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// Create a color attachment reference
	VkAttachmentReference attachmentReference = {};
	attachmentReference.attachment = 0;
	attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Create a subpass
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &attachmentReference;

	// Create a subpass dependency to synchronize our main and UI render passes
	// We want to render the UI after the geometry has been written to the framebuffer
	// so we need to configure a subpass dependency as such
	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Create external dependency
	subpassDependency.dstSubpass = 0; // The geometry subpass comes first
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // Wait on writes
	subpassDependency.dstStageMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Finally create the UI render pass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &attachmentDescription;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &subpassDependency;

	if (vkCreateRenderPass(device->GetVkDevice(), &renderPassInfo, nullptr, &uiRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Unable to create UI render pass!");
	}
}

void ImGuiManager::CreateUICommandPool(Device* device) {
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = device->GetQueueIndex(QueueFlags::Graphics);
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(device->GetVkDevice(), &commandPoolCreateInfo, nullptr, &uiCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("Could not create graphics command pool!");
	}
}

void ImGuiManager::CreateUICommandBuffers(Device* device, SwapChain* swapChain) {
	uiCommandBuffers.resize(swapChain->GetCount());

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = uiCommandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(uiCommandBuffers.size());

	if (vkAllocateCommandBuffers(device->GetVkDevice(), &commandBufferAllocateInfo, uiCommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Unable to allocate UI command buffers!");
	}
}

void ImGuiManager::CreateUIFramebuffers(Device* device, SwapChain* swapChain, std::vector<VkImageView>& imageViews) {
	// Create some UI framebuffers. These will be used in the render pass for the UI
	uiFramebuffers.resize(swapChain->GetCount());
	VkImageView attachment[1];
	VkFramebufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.renderPass = uiRenderPass;
	info.attachmentCount = 1;
	info.pAttachments = attachment;
	info.width = swapChain->GetVkExtent().width;
	info.height = swapChain->GetVkExtent().height;
	info.layers = 1;
	for (uint32_t i = 0; i < swapChain->GetCount(); ++i) {
		attachment[0] = imageViews[i];
		if (vkCreateFramebuffer(device->GetVkDevice(), &info, nullptr, &uiFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Unable to create UI framebuffers!");
		}
	}
}

void ImGuiManager::RecordUICommands(uint32_t bufferIdx, SwapChain* swapChain) {
	VkCommandBufferBeginInfo cmdBufferBegin = {};
	cmdBufferBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBegin.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(uiCommandBuffers[bufferIdx], &cmdBufferBegin) != VK_SUCCESS) {
		throw std::runtime_error("Unable to start recording UI command buffer!");
	}

	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = uiRenderPass;
	renderPassBeginInfo.framebuffer = uiFramebuffers[bufferIdx];
	renderPassBeginInfo.renderArea.extent.width = swapChain->GetVkExtent().width;
	renderPassBeginInfo.renderArea.extent.height = swapChain->GetVkExtent().height;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(uiCommandBuffers[bufferIdx], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Grab and record the draw data for Dear Imgui
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), uiCommandBuffers[bufferIdx]);

	// End and submit render pass
	vkCmdEndRenderPass(uiCommandBuffers[bufferIdx]);

	if (vkEndCommandBuffer(uiCommandBuffers[bufferIdx]) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffers!");
	}
}

void ImGuiManager::RenderUI() {
	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	static float f = 0.0f;
	static int counter = 0;

	ImGui::Begin("Renderer Options");
	ImGui::Text("This is some useful text.");
	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
	if (ImGui::Button("Button")) {
		counter++;
	}
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::Render();
}

void ImGuiManager::CleanUp(Device* device) {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	vkDestroyDescriptorPool(device->GetVkDevice(), uiDescriptorPool, nullptr);
}
