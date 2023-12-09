#pragma once

#include "ShaderProgram.h"

// Shader for raymarching computation
class ComputeLightGridShader : public ShaderProgram {
public:
	ComputeLightGridShader(Device* device, SwapChain* swapchain, VkRenderPass* renderPass);
	~ComputeLightGridShader() { }

	void CreateShaderProgram() override;
	void BindShaderProgram(VkCommandBuffer& commandBuffer) override;
protected:
	
};