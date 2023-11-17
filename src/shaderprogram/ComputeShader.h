#pragma once

#include "ShaderProgram.h"

// Shader for raymarching computation
class ComputeShader : public ShaderProgram {
public:
	ComputeShader(Device* device, SwapChain* swapchain, VkRenderPass* renderPass);
	~ComputeShader() { }

	void CreateShaderProgram() override;
	void BindShaderProgram(VkCommandBuffer& commandBuffer) override;
};