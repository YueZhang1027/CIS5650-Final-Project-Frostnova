#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "Camera.h"
#include "Scene.h"

namespace Descriptor {
    void CreateCameraDescriptorSetLayout(VkDevice logicalDevice);
    void CreateModelDescriptorSetLayout(VkDevice logicalDevice);
    void CreateTimeDescriptorSetLayout(VkDevice logicalDevice);
    void CreateComputeDescriptorSetLayout(VkDevice logicalDevice);

    void CreateDescriptorPool(VkDevice logicalDevice, Scene* scene);

    void CreateCameraDescriptorSet(VkDevice logicalDevice, Camera* camera);
    void CreateModelDescriptorSets(VkDevice logicalDevice, Scene* scene);
    void CreateTimeDescriptorSet(VkDevice logicalDevice, Scene* scene);
    void CreateComputeDescriptorSets(VkDevice logicalDevice);

    void CleanUp(VkDevice logicalDevice);

    extern VkDescriptorSetLayout cameraDescriptorSetLayout;
    extern VkDescriptorSetLayout modelDescriptorSetLayout;
    extern VkDescriptorSetLayout timeDescriptorSetLayout;

    extern VkDescriptorPool descriptorPool;

    extern VkDescriptorSet cameraDescriptorSet;
    extern std::vector<VkDescriptorSet> modelDescriptorSets;
    extern VkDescriptorSet timeDescriptorSet;
}