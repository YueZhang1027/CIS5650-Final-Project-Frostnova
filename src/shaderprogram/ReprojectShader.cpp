#include "ReprojectShader.h"

ReprojectShader::ReprojectShader(Device* device, SwapChain* swapchain, VkRenderPass* renderPass)
	: ShaderProgram(device, swapchain, renderPass)
{
}

void ReprojectShader::CreateShaderProgram()
{
}

void ReprojectShader::BindShaderProgram(VkCommandBuffer& commandBuffer)
{
}

void ReprojectShader::CleanUniforms()
{
}
