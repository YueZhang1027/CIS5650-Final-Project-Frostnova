#pragma once

#include <vulkan/vulkan.h>
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
    void TransitionLayout(Device* device, VkCommandPool commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    VkImageView CreateView(Device* device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void CopyFromBuffer(Device* device, VkCommandPool commandPool, VkBuffer buffer, VkImage& image, uint32_t width, uint32_t height);
    void FromFile(Device* device, VkCommandPool commandPool, const char* path, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageLayout layout, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void ForStorage(Device* device, VkCommandPool commandPool, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageLayout layout, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    // --- Specific Texture Creation ---
    void CreateDepthTexture(Device* device, VkCommandPool commandPool, VkExtent2D extent, Texture* texture);
}
