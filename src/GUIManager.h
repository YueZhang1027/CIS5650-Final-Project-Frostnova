#pragma once

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

namespace ImGui {
	void InitImGui(GLFWwindow* window);
	void RenderImGui();
}

