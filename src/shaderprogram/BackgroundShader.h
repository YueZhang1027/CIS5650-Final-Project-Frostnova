#pragma once

#include "ShaderProgram.h"

class BackgroundShader : public ShaderProgram {
public:
	BackgroundShader(Device* device, SwapChain* swapchain, VkRenderPass* renderPass);
	~BackgroundShader() { }

	void CreateShaderProgram() override;
	void BindShaderProgram(VkCommandBuffer& commandBuffer) override; // Check pipeline layout
};