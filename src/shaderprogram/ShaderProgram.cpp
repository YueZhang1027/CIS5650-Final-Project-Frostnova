#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(Device* device, SwapChain* swapchain, VkRenderPass* renderPass)
{
	this->device = device;
	this->swapChain = swapchain;
	this->renderPass = renderPass;
}

void ShaderProgram::CleanUp() {
	vkDestroyPipelineLayout(device->GetVkDevice(), pipelineLayout, nullptr);
	vkDestroyPipeline(device->GetVkDevice(), pipeline, nullptr);

	CleanUniforms();
}
