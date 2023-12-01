#include "GUIManager.h"

ImGui_ImplVulkanH_Window mainWindowData;
ImGuiIO* io = nullptr;
ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None | ImGuiWindowFlags_NoMove;
bool ui_hide = false;

VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;

void ImGuiManager::SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, Device* device, int width, int height) {
	wd->Surface = surface;
	wd->ImageCount = 5;

	VkBool32 res;
	vkGetPhysicalDeviceSurfaceSupportKHR(device->GetInstance()->GetPhysicalDevice(), 
		device->GetQueueIndex(QueueFlags::Graphics), wd->Surface, &res);
	if (res != VK_TRUE)
	{
		fprintf(stderr, "Error no WSI support on physical device 0\n");
		exit(-1);
	}

	const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(device->GetInstance()->GetPhysicalDevice(), wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

#ifdef IMGUI_UNLIMITED_FRAME_RATE
	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
#else
	VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif
	wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(device->GetInstance()->GetPhysicalDevice(), wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
	//printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

	// Create SwapChain, RenderPass, Framebuffer, etc.
	// IM_ASSERT(g_MinImageCount >= 2);
	ImGui_ImplVulkanH_CreateOrResizeWindow(
		device->GetInstance()->GetVkInstance(),
		device->GetInstance()->GetPhysicalDevice(), 
		device->GetVkDevice(), 
		&mainWindowData,
		device->GetQueueIndex(QueueFlags::Graphics), 
		nullptr, 
		width, 
		height, 
		5);
}

void ImGuiManager::InitImGui(GLFWwindow* window, VkSurfaceKHR surface, Device* device) {
	// int w, h;
	// glfwGetWindowSize(window, &w, &h);

	SetupVulkanWindow(&mainWindowData, surface, device, 1920, 1080);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO(); (void)io;
	ImGui::StyleColorsLight(); // ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForVulkan(window, true);
	
	InitDescriptorPool(device);
	
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = device->GetInstance()->GetVkInstance();
	init_info.PhysicalDevice = device->GetInstance()->GetPhysicalDevice();
	init_info.Device = device->GetVkDevice();
	init_info.QueueFamily = device->GetQueueIndex(QueueFlags::Graphics);
	init_info.Queue = device->GetQueue(QueueFlags::Graphics);
	// init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = g_DescriptorPool;
	init_info.Subpass = 0;
	init_info.MinImageCount = 5;
	init_info.ImageCount = mainWindowData.ImageCount;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	// init_info.Allocator = g_Allocator;
	// init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, mainWindowData.RenderPass);
}

void ImGuiManager::InitDescriptorPool(Device* device) {
	VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1;
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	if (vkCreateDescriptorPool(device->GetVkDevice(), &pool_info, nullptr, &g_DescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool");
	}
}

void ImGuiManager::RenderImGui() {
	// mouseOverImGuiWinow = io->WantCaptureMouse;

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// bool show_demo_window = true;
	// bool show_another_window = false;
	// ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	// static float f = 0.0f;
	// static int counter = 0;
	// 
	// ImGui::Begin("Control Panel", 0, windowFlags);
	// ImGui::SetWindowFontScale(1);
	// 
	// ImGui::Text("press H to hide GUI completely.");
	// if (ImGui::IsKeyPressed('H')) {
	// 	ui_hide = !ui_hide;
	// }
	// 
	// // ImGui::SliderInt("Iterations", &ui_iterations, 1, startupIterations);
	// // 
	// // ImGui::Checkbox("Denoise", &ui_denoise);
	// // 
	// // ImGui::SliderInt("Filter Size", &ui_filterSize, 0, 100);
	// // ImGui::SliderFloat("Color Weight", &ui_colorWeight, 0.0f, 1.0f);
	// // ImGui::SliderFloat("Normal Weight", &ui_normalWeight, 0.0f, 1.0f);
	// // ImGui::SliderFloat("Position Weight", &ui_positionWeight, 0.0f, 1.0f);
	// // 
	// // ImGui::Separator();
	// // 
	// // ImGui::Checkbox("Show GBuffer", &ui_showGbuffer);
	// // 
	// // ImGui::Separator();
	// 
	// ImGui::End();
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
	const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
	if (!is_minimized)
	{
		// (&mainWindowData)->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
		// (&mainWindowData)->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
		// (&mainWindowData)->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
		// (&mainWindowData)->ClearValue.color.float32[3] = clear_color.w;
		// FrameRender(&mainWindowData, draw_data);
		// FramePresent(&mainWindowData);
	}
}

void ImGuiManager::CleanUp(Device* device) {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	ImGui_ImplVulkanH_DestroyWindow(device->GetInstance()->GetVkInstance(), device->GetVkDevice(), &mainWindowData, nullptr);

	vkDestroyDescriptorPool(device->GetVkDevice(), g_DescriptorPool, nullptr);
}
