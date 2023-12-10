#pragma once

#include "ShaderProgram.h"

// Shader for raymarching computation
class ComputeNearShader : public ShaderProgram {
public:
	ComputeNearShader(Device* device, SwapChain* swapchain, VkRenderPass* renderPass);
	~ComputeNearShader() { }

	void CreateShaderProgram() override;
	void BindShaderProgram(VkCommandBuffer& commandBuffer) override;
};