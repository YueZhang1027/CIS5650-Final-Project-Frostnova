#pragma once

#include "Device.h"
#include "Vertex.h"
#include "ShaderModule.h"
#include "Descriptor.h"

class ShaderProgram {
public:
	ShaderProgram(Device* device, SwapChain* swapchain, VkRenderPass* renderPass);
	virtual ~ShaderProgram() { }
	void CleanUp();

	virtual void CreateShaderProgram() = 0;
	virtual void BindShaderProgram(VkCommandBuffer& commandBuffer) = 0; // Bind pipeline and descriptor sets
	//virtual void UnbindShaderProgram(VkCommandBuffer& commandBuffer) = 0;
	//virtual void Recreate() = 0;
protected:
	//virtual void CleanUniforms() = 0;
protected:
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	VkRenderPass* renderPass;

	Device* device;
	SwapChain* swapChain;
};