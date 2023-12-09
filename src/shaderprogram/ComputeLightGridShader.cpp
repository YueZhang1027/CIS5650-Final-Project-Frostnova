#include "ComputeLightGridShader.h"

ComputeLightGridShader::ComputeLightGridShader(Device* device, SwapChain* swapchain, VkRenderPass* renderPass)
	: ShaderProgram(device, swapchain, renderPass) {
	CreateShaderProgram();
}

void ComputeLightGridShader::CreateShaderProgram() {
	VkShaderModule compShaderModule = ShaderModule::Create("shaders/lightGrid.comp.spv", device->GetVkDevice());

	VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
	computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	computeShaderStageInfo.module = compShaderModule;
	computeShaderStageInfo.pName = "main";

	// Add the compute dsecriptor set layout you create to this list
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
		Descriptor::imageStorageDescriptorSetLayout,
		Descriptor::sceneDescriptorSetLayout
	};

	// Create pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;

	if (vkCreatePipelineLayout(device->GetVkDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout");
	}

	// Create compute pipeline
	VkComputePipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineInfo.stage = computeShaderStageInfo;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.flags = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateComputePipelines(device->GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create compute pipeline");
	}

	vkDestroyShaderModule(device->GetVkDevice(), compShaderModule, nullptr);
}

void ComputeLightGridShader::BindShaderProgram(VkCommandBuffer& commandBuffer) {
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &Descriptor::lightGridDescriptorSet, 0, nullptr);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 1, 1, &Descriptor::sceneDescriptorSet, 0, nullptr);
}