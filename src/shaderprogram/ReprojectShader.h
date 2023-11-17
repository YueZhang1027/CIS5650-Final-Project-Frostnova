#pragma once

#include "ShaderProgram.h"

class ReprojectShader : public ShaderProgram {
public:
	ReprojectShader(Device* device, SwapChain* swapchain, VkRenderPass* renderPass);
	~ReprojectShader() { }

	void CreateShaderProgram() override;
	void BindShaderProgram(VkCommandBuffer& commandBuffer) override;
protected:
	bool swapBuffers;
};