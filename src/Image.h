#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "Device.h"

struct Texture {
	VkImage image;
    VkDeviceMemory imageMemory;
	VkImageView imageView;
	VkSampler sampler; // if exists

    Texture() = default;

    void CleanUp(VkDevice logicalDevice) {
		vkDestroyImageView(logicalDevice, imageView, nullptr);
		vkDestroyImage(logicalDevice, image, nullptr);
		vkFreeMemory(logicalDevice, imageMemory, nullptr);
	}
};

namespace Image {
    void Create(Device* device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void Create3D(Device* device, glm::ivec3 dimension, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void TransitionLayout(Device* device, VkCommandPool commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    VkImageView CreateView(Device* device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType viewType);
    VkSampler CreateSampler(Device* device);
    void CopyFromBuffer(Device* device, VkCommandPool commandPool, VkBuffer buffer, VkImage& image, uint32_t width, uint32_t height, uint32_t depth);
    void FromFile(Device* device, VkCommandPool commandPool, const char* path, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageLayout layout, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void FromFiles(Device* device, VkCommandPool commandPool, const char* path, glm::ivec3 dimension, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageLayout layout, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory); // to constuct 3D

    // --- Specific Texture Creation ---
    Texture* CreateDepthTexture(Device* device, VkCommandPool commandPool, VkExtent2D extent);
    Texture* CreateStorageTexture(Device* device, VkCommandPool commandPool, VkExtent2D extent);
    Texture* CreateTexture3DFromFiles(Device* device, VkCommandPool commandPool, const char* path, glm::ivec3 dimension);
}
