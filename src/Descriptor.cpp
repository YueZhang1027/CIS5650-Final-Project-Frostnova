#include "Descriptor.h"

#include <array>
#include <stdexcept>

VkDescriptorSetLayout Descriptor::cameraDescriptorSetLayout;
VkDescriptorSetLayout Descriptor::timeDescriptorSetLayout;
VkDescriptorSetLayout Descriptor::imageStorageDescriptorSetLayout;

VkDescriptorPool Descriptor::descriptorPool;

VkDescriptorSet Descriptor::imageCurDescriptorSet;
VkDescriptorSet Descriptor::imagePrevDescriptorSet;
VkDescriptorSet Descriptor::cameraDescriptorSet;
VkDescriptorSet Descriptor::timeDescriptorSet;

void Descriptor::CreateImageStorageDescriptorSetLayout(VkDevice logicalDevice) {
    // Describe the binding of the descriptor set layout
	VkDescriptorSetLayoutBinding imageLayoutBinding = {};
	imageLayoutBinding.binding = 0;
	imageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	imageLayoutBinding.descriptorCount = 1;
	imageLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	imageLayoutBinding.pImmutableSamplers = nullptr;

	std::vector<VkDescriptorSetLayoutBinding> bindings = { imageLayoutBinding };

	// Create the descriptor set layout
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &imageStorageDescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout: Image Storage");
	}
}

void Descriptor::CreateCameraDescriptorSetLayout(VkDevice logicalDevice) {
    // Describe the binding of the descriptor set layout
    VkDescriptorSetLayoutBinding curLayoutBinding = {};
    curLayoutBinding.binding = 0;
    curLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    curLayoutBinding.descriptorCount = 1;
    curLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
    curLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding prevLayoutBinding = {};
    prevLayoutBinding.binding = 1;
    prevLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    prevLayoutBinding.descriptorCount = 1;
    prevLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
    prevLayoutBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> bindings = { curLayoutBinding, prevLayoutBinding };

    // Create the descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &cameraDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout: Camera");
    }
}

void Descriptor::CreateTimeDescriptorSetLayout(VkDevice logicalDevice) {
    // Describe the binding of the descriptor set layout
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding };

    // Create the descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &timeDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout: Time");
    }
}

void Descriptor::CreateDescriptorPool(VkDevice logicalDevice, Scene* scene) {
    // Describe which descriptor types that the descriptor sets will contain
    std::vector<VkDescriptorPoolSize> poolSizes = {
        // Image Storage (prev, cur)
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2},

        // Camera, PrevCamera
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2},

        // Time
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},

        // TODO: Add any additional types and counts of descriptors you will need to allocate
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 10;

    if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool");
    }
}

void Descriptor::CreateImageStorageDescriptorSet(VkDevice logicalDevice, Texture* texture, VkDescriptorSet& imageDescriptorSet) {
    // Describe the desciptor set
	VkDescriptorSetLayout layouts[] = { imageStorageDescriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	// Allocate descriptor sets
    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &imageDescriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor set");
	}

	// Configure the descriptors to refer to buffers
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageInfo.imageView = texture->imageView;
	imageInfo.sampler = texture->sampler;

	std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = imageDescriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = nullptr;
	descriptorWrites[0].pImageInfo = &imageInfo;
	descriptorWrites[0].pTexelBufferView = nullptr;

	// Update descriptor sets
	vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void Descriptor::CreateCameraDescriptorSet(VkDevice logicalDevice, Camera* camera) {
    // Describe the desciptor set
    VkDescriptorSetLayout layouts[] = { cameraDescriptorSetLayout };
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    // Allocate descriptor sets
    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &cameraDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set");
    }

    // Configure the descriptors to refer to buffers
    VkDescriptorBufferInfo cameraBufferInfo = {};
    cameraBufferInfo.buffer = camera->GetBuffer();
    cameraBufferInfo.offset = 0;
    cameraBufferInfo.range = sizeof(CameraBufferObject);

    VkDescriptorBufferInfo prevCameraBufferInfo = {};
    prevCameraBufferInfo.buffer = camera->GetPrevBuffer();
    prevCameraBufferInfo.offset = 0;
    prevCameraBufferInfo.range = sizeof(CameraBufferObject);

    std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = cameraDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &cameraBufferInfo;
    descriptorWrites[0].pImageInfo = nullptr;
    descriptorWrites[0].pTexelBufferView = nullptr;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = cameraDescriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &prevCameraBufferInfo;
    descriptorWrites[1].pImageInfo = nullptr;
    descriptorWrites[1].pTexelBufferView = nullptr;

    // Update descriptor sets
    vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void Descriptor::CreateTimeDescriptorSet(VkDevice logicalDevice, Scene* scene) {
    // Describe the desciptor set
    VkDescriptorSetLayout layouts[] = { timeDescriptorSetLayout };
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    // Allocate descriptor sets
    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &timeDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set");
    }

    // Configure the descriptors to refer to buffers
    VkDescriptorBufferInfo timeBufferInfo = {};
    timeBufferInfo.buffer = scene->GetTimeBuffer();
    timeBufferInfo.offset = 0;
    timeBufferInfo.range = sizeof(Time);

    std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = timeDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &timeBufferInfo;
    descriptorWrites[0].pImageInfo = nullptr;
    descriptorWrites[0].pTexelBufferView = nullptr;

    // Update descriptor sets
    vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void Descriptor::CleanUp(VkDevice logicalDevice) {
    vkDestroyDescriptorSetLayout(logicalDevice, cameraDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, timeDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, imageStorageDescriptorSetLayout, nullptr);

    vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
}
