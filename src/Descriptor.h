#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "Camera.h"
#include "Scene.h"

namespace Descriptor {
    void CreateCameraDescriptorSetLayout(VkDevice logicalDevice);
    void CreateTimeDescriptorSetLayout(VkDevice logicalDevice);

    void CreateDescriptorPool(VkDevice logicalDevice, Scene* scene);

    void CreateCameraDescriptorSet(VkDevice logicalDevice, Camera* camera);
    void CreateTimeDescriptorSet(VkDevice logicalDevice, Scene* scene);

    void CleanUp(VkDevice logicalDevice);

    extern VkDescriptorSetLayout cameraDescriptorSetLayout;
    extern VkDescriptorSetLayout timeDescriptorSetLayout;

    extern VkDescriptorPool descriptorPool;

    extern VkDescriptorSet cameraDescriptorSet;
    extern VkDescriptorSet timeDescriptorSet;
}