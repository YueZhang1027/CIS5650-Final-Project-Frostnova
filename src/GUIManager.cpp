#include "GUIManager.h"

ImGuiIO* io = nullptr;
ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None | ImGuiWindowFlags_NoMove;
bool ui_hide = false;

void ImGui::InitImGui(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO(); (void)io;
	ImGui::StyleColorsLight();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 120");
}

void ImGui::RenderImGui() {
	// mouseOverImGuiWinow = io->WantCaptureMouse;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	static float f = 0.0f;
	static int counter = 0;

	ImGui::Begin("Control Panel", 0, windowFlags);
	ImGui::SetWindowFontScale(1);

	ImGui::Text("press H to hide GUI completely.");
	if (ImGui::IsKeyPressed('H')) {
		ui_hide = !ui_hide;
	}

	// ImGui::SliderInt("Iterations", &ui_iterations, 1, startupIterations);
	// 
	// ImGui::Checkbox("Denoise", &ui_denoise);
	// 
	// ImGui::SliderInt("Filter Size", &ui_filterSize, 0, 100);
	// ImGui::SliderFloat("Color Weight", &ui_colorWeight, 0.0f, 1.0f);
	// ImGui::SliderFloat("Normal Weight", &ui_normalWeight, 0.0f, 1.0f);
	// ImGui::SliderFloat("Position Weight", &ui_positionWeight, 0.0f, 1.0f);
	// 
	// ImGui::Separator();
	// 
	// ImGui::Checkbox("Show GBuffer", &ui_showGbuffer);
	// 
	// ImGui::Separator();

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
