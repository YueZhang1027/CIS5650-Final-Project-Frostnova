#include "ComputeShader.h"

ComputeShader::ComputeShader(Device* device, SwapChain* swapchain, VkRenderPass* renderPass)
	: ShaderProgram(device, swapchain, renderPass)
{
}

void ComputeShader::CreateShaderProgram()
{
}

void ComputeShader::BindShaderProgram(VkCommandBuffer& commandBuffer)
{
}