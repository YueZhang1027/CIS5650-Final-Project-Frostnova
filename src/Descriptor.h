#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "Camera.h"
#include "Scene.h"
#include "Image.h"

namespace Descriptor {
    void CreateImageStorageDescriptorSetLayout(VkDevice logicalDevice);
    void CreateImageDescriptorSetLayout(VkDevice logicalDevice);

    void CreateCameraDescriptorSetLayout(VkDevice logicalDevice);

    void CreateDescriptorPool(VkDevice logicalDevice, Scene* scene);

    void CreateImageStorageDescriptorSet(VkDevice logicalDevice, Texture* texture, VkDescriptorSet& imageDescriptorSet);
    void CreateImageDescriptorSet(VkDevice logicalDevice, Texture* texture, VkDescriptorSet& imageDescriptorSet);
    void CreateCameraDescriptorSet(VkDevice logicalDevice, Camera* camera);

    void CleanUp(VkDevice logicalDevice);

    extern VkDescriptorSetLayout imageStorageDescriptorSetLayout;
    extern VkDescriptorSetLayout imageDescriptorSetLayout;
    extern VkDescriptorSetLayout cameraDescriptorSetLayout;

    extern VkDescriptorPool descriptorPool;

    extern VkDescriptorSet imageCurDescriptorSet;
    extern VkDescriptorSet imagePrevDescriptorSet;
    extern VkDescriptorSet frameDescriptorSet;
    extern VkDescriptorSet cameraDescriptorSet;
}