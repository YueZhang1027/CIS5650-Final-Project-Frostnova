#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"

struct UniformBuffer {
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    void* mappedData;

    void MapMemory(Device* device, VkDeviceSize size);
    void Clean(Device* device);
};

namespace BufferUtils {
    void CreateBuffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CopyBuffer(Device* device, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void CreateBufferFromData(Device* device, VkCommandPool commandPool, void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
}
