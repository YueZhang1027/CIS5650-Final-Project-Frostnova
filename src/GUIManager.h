#pragma once

#define GLFW_INCLUDE_VULKAN

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_vulkan.h"

#include "Instance.h"
#include "Device.h"

namespace ImGuiManager {
	void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, Device* device, int width, int height);
	void InitImGui(GLFWwindow* window, VkSurfaceKHR surface, Device* device);
	void InitDescriptorPool(Device* device);
	void RenderImGui();
	void CleanUp(Device* device);
}

