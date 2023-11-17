#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "Camera.h"
#include "Scene.h"
#include "Image.h"

namespace Descriptor {
    void CreateImageStorageDescriptorSetLayout(VkDevice logicalDevice);
    void CreateCameraDescriptorSetLayout(VkDevice logicalDevice);
    void CreateTimeDescriptorSetLayout(VkDevice logicalDevice);

    void CreateDescriptorPool(VkDevice logicalDevice, Scene* scene);

    void CreateImageStorageDescriptorSet(VkDevice logicalDevice, Texture* texture, VkDescriptorSet& imageDescriptorSet);
    void CreateCameraDescriptorSet(VkDevice logicalDevice, Camera* camera);
    void CreateTimeDescriptorSet(VkDevice logicalDevice, Scene* scene);

    void CleanUp(VkDevice logicalDevice);

    extern VkDescriptorSetLayout imageStorageDescriptorSetLayout;
    extern VkDescriptorSetLayout cameraDescriptorSetLayout;
    extern VkDescriptorSetLayout timeDescriptorSetLayout;

    extern VkDescriptorPool descriptorPool;

    extern VkDescriptorSet imageCurDescriptorSet;
    extern VkDescriptorSet imagePrevDescriptorSet;
    extern VkDescriptorSet cameraDescriptorSet;
    extern VkDescriptorSet timeDescriptorSet;
}