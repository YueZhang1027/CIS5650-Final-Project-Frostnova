#include "Descriptor.h"

#include <array>
#include <stdexcept>

VkDescriptorSetLayout Descriptor::cameraDescriptorSetLayout;
VkDescriptorSetLayout Descriptor::imageDescriptorSetLayout;
VkDescriptorSetLayout Descriptor::imageStorageDescriptorSetLayout;
VkDescriptorSetLayout Descriptor::computeImagesDescriptorSetLayout;
VkDescriptorSetLayout Descriptor::computeNubisCubedImagesDescriptorSetLayout;
VkDescriptorSetLayout Descriptor::sceneDescriptorSetLayout;

VkDescriptorPool Descriptor::descriptorPool;

VkDescriptorSet Descriptor::imageCurDescriptorSet;
VkDescriptorSet Descriptor::imagePrevDescriptorSet;
VkDescriptorSet Descriptor::frameDescriptorSet;
VkDescriptorSet Descriptor::computeImagesDescriptorSet;
VkDescriptorSet Descriptor::computeNubisCubedImagesDescriptorSet;
VkDescriptorSet Descriptor::cameraDescriptorSet;
VkDescriptorSet Descriptor::sceneDescriptorSet;

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

void Descriptor::CreateImageDescriptorSetLayout(VkDevice logicalDevice) {
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    
    std::vector<VkDescriptorSetLayoutBinding> bindings = { samplerLayoutBinding };

    // Create the descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &imageDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout: Image");
    }
}

void Descriptor::CreateComputeImagesDescriptorSetLayout(VkDevice logicalDevice) {
    VkDescriptorSetLayoutBinding lowResLayoutBinding = {};
    lowResLayoutBinding.binding = 0;
    lowResLayoutBinding.descriptorCount = 1;
    lowResLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    lowResLayoutBinding.pImmutableSamplers = nullptr;
    lowResLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding hiResLayoutBinding = {};
    hiResLayoutBinding.binding = 1;
    hiResLayoutBinding.descriptorCount = 1;
    hiResLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    hiResLayoutBinding.pImmutableSamplers = nullptr;
    hiResLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding weatherMapLayoutBinding = {};
    weatherMapLayoutBinding.binding = 2;
    weatherMapLayoutBinding.descriptorCount = 1;
    weatherMapLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    weatherMapLayoutBinding.pImmutableSamplers = nullptr;
    weatherMapLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding curlNoiseLayoutBinding = {};
    curlNoiseLayoutBinding.binding = 3;
    curlNoiseLayoutBinding.descriptorCount = 1;
    curlNoiseLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    curlNoiseLayoutBinding.pImmutableSamplers = nullptr;
    curlNoiseLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings = { 
        lowResLayoutBinding, // lowRes
        hiResLayoutBinding, // hiRes
        weatherMapLayoutBinding, // weatherMap
        curlNoiseLayoutBinding, // curlNoise
    };

    // Create the descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &computeImagesDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout: Compute shader images");
    }
}

void Descriptor::CreateComputeNubisCubedImagesDescriptorSetLayout(VkDevice logicalDevice) {
    VkDescriptorSetLayoutBinding modelingNVDFLayoutBinding = {};
    modelingNVDFLayoutBinding.binding = 0;
    modelingNVDFLayoutBinding.descriptorCount = 1;
    modelingNVDFLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    modelingNVDFLayoutBinding.pImmutableSamplers = nullptr;
    modelingNVDFLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding fieldNVDFLayoutBinding = {};
    fieldNVDFLayoutBinding.binding = 1;
    fieldNVDFLayoutBinding.descriptorCount = 1;
    fieldNVDFLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    fieldNVDFLayoutBinding.pImmutableSamplers = nullptr;
    fieldNVDFLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding cloudDetailNoiseLayoutBinding = {};
    cloudDetailNoiseLayoutBinding.binding = 2;
    cloudDetailNoiseLayoutBinding.descriptorCount = 1;
    cloudDetailNoiseLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    cloudDetailNoiseLayoutBinding.pImmutableSamplers = nullptr;
    cloudDetailNoiseLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        modelingNVDFLayoutBinding, // modelingNVDF
        fieldNVDFLayoutBinding, // fieldNVDF
        cloudDetailNoiseLayoutBinding, // cloudDetailNoise
    };

    // Create the descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &computeNubisCubedImagesDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout: Compute Nubis Cubed shader images");
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

    VkDescriptorSetLayoutBinding paramLayoutBinding = {};
    paramLayoutBinding.binding = 2;
    paramLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    paramLayoutBinding.descriptorCount = 1;
    paramLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
    paramLayoutBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> bindings = { curLayoutBinding, prevLayoutBinding, paramLayoutBinding };

    // Create the descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &cameraDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout: Camera");
    }
}

void Descriptor::CreateSceneDescriptorSetLayout(VkDevice logicalDevice) {
    // Describe the binding of the descriptor set layout
    VkDescriptorSetLayoutBinding curLayoutBinding = {};
    curLayoutBinding.binding = 0;
    curLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    curLayoutBinding.descriptorCount = 1;
    curLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
    curLayoutBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> bindings = { curLayoutBinding};

    // Create the descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &sceneDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout: Scene");
    }
}

void Descriptor::CreateDescriptorPool(VkDevice logicalDevice, Scene* scene) {
    // Describe which descriptor types that the descriptor sets will contain
    std::vector<VkDescriptorPoolSize> poolSizes = {
        // Image Storage (prev, cur)
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},

        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},

        // Image samplers: frame
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},

        // Image samplers: compute images: lowres, hires, weather map
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5},

        // Camera, PrevCamera, Parameter
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},

        // Time(Scene)Buffer
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, 

        // Compute nubis cubed images: modelingNVDF, fieldNVDF, cloudDetailNoise
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3},
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 15; // TODO: check

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

void Descriptor::CreateImageDescriptorSet(VkDevice logicalDevice, Texture* texture, VkDescriptorSet& imageDescriptorSet) {
    // Describe the desciptor set
	VkDescriptorSetLayout layouts[] = { imageDescriptorSetLayout };
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
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = texture->imageView;
	imageInfo.sampler = texture->sampler;

	std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = imageDescriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
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

    VkDescriptorBufferInfo paramBufferInfo = {};
    prevCameraBufferInfo.buffer = camera->GetCameraParamBuffer();
    prevCameraBufferInfo.offset = 0;
    prevCameraBufferInfo.range = sizeof(CameraParamBufferObject);

    std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};
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

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = cameraDescriptorSet;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &prevCameraBufferInfo;
    descriptorWrites[2].pImageInfo = nullptr;
    descriptorWrites[2].pTexelBufferView = nullptr;

    // Update descriptor sets
    vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void Descriptor::CreateComputeImagesDescriptorSet(VkDevice logicalDevice, 
    Texture* lowResTex, Texture* hiResTex, Texture* weatherMap, Texture* curlNoise) {
    // Describe the desciptor set
	VkDescriptorSetLayout layouts[] = { computeImagesDescriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	// Allocate descriptor sets
    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &computeImagesDescriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor set");
	}

	// Configure the descriptors to refer to buffers
	VkDescriptorImageInfo lowResImageInfo = {};
	lowResImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	lowResImageInfo.imageView = lowResTex->imageView;
	lowResImageInfo.sampler = lowResTex->sampler;

	VkDescriptorImageInfo hiResImageInfo = {};
	hiResImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	hiResImageInfo.imageView = hiResTex->imageView;
	hiResImageInfo.sampler = hiResTex->sampler;

	VkDescriptorImageInfo weatherMapImageInfo = {};
	weatherMapImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	weatherMapImageInfo.imageView = weatherMap->imageView;
	weatherMapImageInfo.sampler = weatherMap->sampler;

    VkDescriptorImageInfo curlImageInfo = {};
    curlImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    curlImageInfo.imageView = curlNoise->imageView;
    curlImageInfo.sampler = curlNoise->sampler;

    VkDescriptorImageInfo modelingDataInfo = {};
    modelingDataInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    modelingDataInfo.imageView = curlNoise->imageView;
    modelingDataInfo.sampler = curlNoise->sampler;

	std::array<VkWriteDescriptorSet, 4> descriptorWrites = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = computeImagesDescriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = nullptr;
	descriptorWrites[0].pImageInfo = &lowResImageInfo;
	descriptorWrites[0].pTexelBufferView = nullptr;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = computeImagesDescriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = nullptr;
    descriptorWrites[1].pImageInfo = &hiResImageInfo;
    descriptorWrites[1].pTexelBufferView = nullptr;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = computeImagesDescriptorSet;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = nullptr;
    descriptorWrites[2].pImageInfo = &weatherMapImageInfo;
    descriptorWrites[2].pTexelBufferView = nullptr;

    descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[3].dstSet = computeImagesDescriptorSet;
    descriptorWrites[3].dstBinding = 3;
    descriptorWrites[3].dstArrayElement = 0;
    descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[3].descriptorCount = 1;
    descriptorWrites[3].pBufferInfo = nullptr;
    descriptorWrites[3].pImageInfo = &curlImageInfo;
    descriptorWrites[3].pTexelBufferView = nullptr;

    // Update descriptor sets
    vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void Descriptor::CreateComputeNubisCubedImagesDescriptorSet(VkDevice logicalDevice, Texture* modelingNVDFTex, Texture* fieldNVDFTex, Texture* cloudDetailNoiseTex) {
    // Describe the desciptor set
    VkDescriptorSetLayout layouts[] = { computeNubisCubedImagesDescriptorSetLayout };
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    // Allocate descriptor sets
    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &computeNubisCubedImagesDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set");
    }

    // Configure the descriptors to refer to buffers
    VkDescriptorImageInfo modelingNVDFImageInfo = {};
    modelingNVDFImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    modelingNVDFImageInfo.imageView = modelingNVDFTex->imageView;
    modelingNVDFImageInfo.sampler = modelingNVDFTex->sampler;

    VkDescriptorImageInfo fieldNVDFImageInfo = {};
    fieldNVDFImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    fieldNVDFImageInfo.imageView = fieldNVDFTex->imageView;
    fieldNVDFImageInfo.sampler = fieldNVDFTex->sampler;

    VkDescriptorImageInfo cloudDetailNoiseImageInfo = {};
    cloudDetailNoiseImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    cloudDetailNoiseImageInfo.imageView = cloudDetailNoiseTex->imageView;
    cloudDetailNoiseImageInfo.sampler = cloudDetailNoiseTex->sampler;

    std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = computeNubisCubedImagesDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = nullptr;
    descriptorWrites[0].pImageInfo = &modelingNVDFImageInfo;
    descriptorWrites[0].pTexelBufferView = nullptr;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = computeNubisCubedImagesDescriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = nullptr;
    descriptorWrites[1].pImageInfo = &fieldNVDFImageInfo;
    descriptorWrites[1].pTexelBufferView = nullptr;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = computeNubisCubedImagesDescriptorSet;
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = nullptr;
    descriptorWrites[2].pImageInfo = &cloudDetailNoiseImageInfo;
    descriptorWrites[2].pTexelBufferView = nullptr;

    // Update descriptor sets
    vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void Descriptor::CreateSceneDescriptorSet(VkDevice logicalDevice, Scene* scene) {
    // Describe the desciptor set
    VkDescriptorSetLayout layouts[] = { sceneDescriptorSetLayout };
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    // Allocate descriptor sets
    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &sceneDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set");
    }

    // Configure the descriptors to refer to buffers
    VkDescriptorBufferInfo sceneBufferInfo = {};
    sceneBufferInfo.buffer = scene->GetTimeBuffer();
    sceneBufferInfo.offset = 0;
    sceneBufferInfo.range = sizeof(Time);

    std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = sceneDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &sceneBufferInfo;
    descriptorWrites[0].pImageInfo = nullptr;
    descriptorWrites[0].pTexelBufferView = nullptr;

    // Update descriptor sets
    vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void Descriptor::CleanUp(VkDevice logicalDevice) {
    vkDestroyDescriptorSetLayout(logicalDevice, cameraDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, imageStorageDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, imageDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, sceneDescriptorSetLayout, nullptr);

    vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
}
