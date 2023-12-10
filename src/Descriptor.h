#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "Camera.h"
#include "Scene.h"
#include "Image.h"

namespace Descriptor {
    void CreateImageStorageDescriptorSetLayout(VkDevice logicalDevice);
    void CreateImageDescriptorSetLayout(VkDevice logicalDevice);
    void CreateComputeImagesDescriptorSetLayout(VkDevice logicalDevice);
    void CreateComputeNubisCubedImagesDescriptorSetLayout(VkDevice logicalDevice);
    void CreateUIParamDescriptorSetLayout(VkDevice logicalDevice);


    void CreateCameraDescriptorSetLayout(VkDevice logicalDevice);
    void CreateSceneDescriptorSetLayout(VkDevice logicalDevice);

    void CreateDescriptorPool(VkDevice logicalDevice, Scene* scene);

    void CreateImageStorageDescriptorSet(VkDevice logicalDevice, Texture* texture, VkDescriptorSet& imageDescriptorSet);
    void CreateImageDescriptorSet(VkDevice logicalDevice, Texture* texture, VkDescriptorSet& imageDescriptorSet);
    void CreateCameraDescriptorSet(VkDevice logicalDevice, Camera* camera);
    void CreateComputeImagesDescriptorSet(VkDevice logicalDevice, 
        Texture* lowResTex, Texture* hiResTex, Texture* weatherMap, Texture* curlNoise);
    void CreateComputeNubisCubedImagesDescriptorSet(VkDevice logicalDevice, 
        Texture* modelingNVDFTex, Texture* cloudDetailNoiseTex);
    void CreateSceneDescriptorSet(VkDevice logicalDevice, Scene* scene);
    void CreateUIParamDescriptorSet(VkDevice logicalDevice, VkBuffer& uiControlBufferObject, VkDeviceSize size);

    void CleanUp(VkDevice logicalDevice);

    extern VkDescriptorSetLayout imageStorageDescriptorSetLayout;
    extern VkDescriptorSetLayout imageDescriptorSetLayout;
    extern VkDescriptorSetLayout cameraDescriptorSetLayout;
    extern VkDescriptorSetLayout computeImagesDescriptorSetLayout;
    extern VkDescriptorSetLayout computeNubisCubedImagesDescriptorSetLayout;
    extern VkDescriptorSetLayout sceneDescriptorSetLayout;
    extern VkDescriptorSetLayout uiParamDescriptorSetLayout;

    extern VkDescriptorPool descriptorPool;

    extern VkDescriptorSet imageCurDescriptorSet;
    extern VkDescriptorSet imagePrevDescriptorSet;
    extern VkDescriptorSet frameDescriptorSet;
    extern VkDescriptorSet computeImagesDescriptorSet;
    extern VkDescriptorSet computeNubisCubedImagesDescriptorSet;
    extern VkDescriptorSet lightGridDescriptorSet;
    extern VkDescriptorSet lightGridSamplerDescriptorSet;
    extern VkDescriptorSet uiParamDescriptorSet;

    extern VkDescriptorSet cameraDescriptorSet;
    extern VkDescriptorSet sceneDescriptorSet;
}