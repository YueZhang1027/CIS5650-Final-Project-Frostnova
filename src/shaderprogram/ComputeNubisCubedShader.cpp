#include "ComputeNubisCubedShader.h"

ComputeNubisCubedShader::ComputeNubisCubedShader(Device* device, SwapChain* swapchain, VkRenderPass* renderPass)
	: ShaderProgram(device, swapchain, renderPass) {
	CreateShaderProgram();
	swapBuffers = false;
}

void ComputeNubisCubedShader::CreateShaderProgram() {
	VkShaderModule compShaderModule = ShaderModule::Create("shaders/computeNubisCubed.comp.spv", device->GetVkDevice());

	VkPipelineShaderStageCreateInfo computeShaderStageInfo = {};
	computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	computeShaderStageInfo.module = compShaderModule;
	computeShaderStageInfo.pName = "main";

	// Add the compute dsecriptor set layout you create to this list
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {
		Descriptor::imageStorageDescriptorSetLayout,
		// Descriptor::imageStorageDescriptorSetLayout,
		Descriptor::cameraDescriptorSetLayout,
		Descriptor::computeNubisCubedImagesDescriptorSetLayout,
		Descriptor::sceneDescriptorSetLayout,
		Descriptor::imageDescriptorSetLayout,
		Descriptor::uiParamDescriptorSetLayout
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

void ComputeNubisCubedShader::BindShaderProgram(VkCommandBuffer& commandBuffer) {
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &Descriptor::imageCurDescriptorSet, 0, nullptr);
	// vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, swapBuffers ? 0 : 1, 1, &Descriptor::imagePrevDescriptorSet, 0, nullptr);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 1, 1, &Descriptor::cameraDescriptorSet, 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 2, 1, &Descriptor::computeNubisCubedImagesDescriptorSet, 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 3, 1, &Descriptor::sceneDescriptorSet, 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 4, 1, &Descriptor::lightGridSamplerDescriptorSet, 0, nullptr);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 5, 1, &Descriptor::uiParamDescriptorSet, 0, nullptr);

	swapBuffers = !swapBuffers;
}