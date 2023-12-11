#pragma once

#include "ShaderProgram.h"

// Shader for raymarching computation
class ComputeNubisCubedShader : public ShaderProgram {
public:
	ComputeNubisCubedShader(Device* device, SwapChain* swapchain, VkRenderPass* renderPass);
	~ComputeNubisCubedShader() { }

	void CreateShaderProgram() override;
	void BindShaderProgram(VkCommandBuffer& commandBuffer) override;
};