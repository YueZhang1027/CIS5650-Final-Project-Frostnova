#pragma once

#include "Device.h"
#include "SwapChain.h"
#include "Scene.h"
#include "Camera.h"

#include "Image.h"
#include "shaderprogram/BackgroundShader.h"
#include "shaderprogram/ReprojectShader.h"
#include "shaderprogram/ComputeShader.h"

// TODO: setup 
struct OffscreenPass {
    VkRenderPass renderPass;
    VkSampler sampler;
    std::vector<VkCommandBuffer> commandBuffers;
    VkSemaphore semaphore = VK_NULL_HANDLE;
    std::array<VkFramebuffer, 3> framebuffers;
};

class Renderer {
public:
    Renderer() = delete;
    Renderer(Device* device, SwapChain* swapChain, Scene* scene, Camera* camera);
    ~Renderer();

    void CreateCommandPools();

    void CreateRenderPass();
    void CreateOffscreenRenderPass();

    void CreateModels();
    void CreateDescriptors();
    void CreatePipelines();

    void CreateFrameResources();
    void DestroyFrameResources();
    void RecreateFrameResources();

    void RecordCommandBuffers();
    void RecordComputeCommandBuffer();

    void UpdateUniformBuffers();
    void Frame();

private:
    Device* device;
    VkDevice logicalDevice;
    SwapChain* swapChain;
    Scene* scene;
    Camera* camera;

    VkCommandPool graphicsCommandPool;
    VkCommandPool computeCommandPool;

    VkRenderPass renderPass;

    // --- Shader programs ---
    BackgroundShader* backgroundShader;
    ReprojectShader* reprojectShader;
    ComputeShader* computeShader;

    // --- Frame resources ---
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;

    Texture* depthTexture;
    Texture* imageCurTexture;
    Texture* imagePrevTexture;

    Texture* hiResCloudShapeTexture;
    Texture* lowResCloudShapeTexture;

    // --- Geometries ---
    Model* backgroundQuad;

    // --- Command Buffers ---
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> computeCommandBuffers;
};
