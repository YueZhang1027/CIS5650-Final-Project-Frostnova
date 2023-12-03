#pragma once

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_vulkan.h"

#include "Instance.h"
#include "Device.h"

namespace ImGuiManager {
	void InitUI(GLFWwindow* window, Device* device, SwapChain* swapChain, std::vector<VkImageView>& imageViews);
	void RecreateUI(Device* device, SwapChain* swapChain, std::vector<VkImageView>& imageViews);

	void CreateDescriptorPool(Device* device);
	void CreateUIRenderPass(Device* device, VkFormat imageFormat);
	void CreateUICommandPool(Device* device);
	void CreateUICommandBuffers(Device* device, SwapChain* swapChain);
	void CreateUIFramebuffers(Device* device, SwapChain* swapChain, std::vector<VkImageView>& imageViews);

	void RecordUICommands(uint32_t bufferIdx, SwapChain* swapChain);
	void RenderUI();
	void CleanUp(Device* device);
}

