#pragma once

#include "ShaderProgram.h"

class PostShader : public ShaderProgram {
public:
	PostShader(Device* device, SwapChain* swapchain, VkRenderPass* renderPass, std::string vertPath, std::string fragPath);
	~PostShader() { }

	void CreateShaderProgram() override;
	void BindShaderProgram(VkCommandBuffer& commandBuffer) override; // Check pipeline layout
protected:
	std::vector<std::string> shaderFiles;
};