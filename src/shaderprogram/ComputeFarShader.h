#pragma once

#include "ShaderProgram.h"

class ComputeFarShader : public ShaderProgram {
public:
	ComputeFarShader(Device* device, SwapChain* swapchain, VkRenderPass* renderPass);
	~ComputeFarShader() { }

	void CreateShaderProgram() override;
	void BindShaderProgram(VkCommandBuffer& commandBuffer) override;
};