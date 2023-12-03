#pragma once

#include "Device.h"
#include "SwapChain.h"
#include "Scene.h"
#include "Camera.h"

#include "Image.h"
#include "shaderprogram/ShaderProgramIncludes.h"

#include "GUIManager.h"

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
    void RecordOffscreenCommandBuffers();
    void RecordComputeCommandBuffer();

    void UpdateUniformBuffers();
    void Frame();

    std::vector<VkImageView> GetImageViews() const { return imageViews; }
private:
    Device* device;
    VkDevice logicalDevice;
    SwapChain* swapChain;
    Scene* scene;
    Camera* camera;

    VkCommandPool graphicsCommandPool;
    VkCommandPool computeCommandPool;

    VkRenderPass renderPass;
    VkRenderPass offscreenRenderPass;

    // --- Shader programs ---
    PostShader* backgroundShader;
    ReprojectShader* reprojectShader;
    ComputeShader* computeShader;

    // --- Frame resources ---
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;

    std::vector<Texture*> offscreenTextures;
    std::vector<VkFramebuffer> offscreenFramebuffers;

    Texture* depthTexture;
    Texture* imageCurTexture;
    Texture* imagePrevTexture;

    Texture* hiResCloudShapeTexture;
    Texture* lowResCloudShapeTexture;
    Texture* weatherMapTexture;
    Texture* curlNoiseTexture;
    Texture* modelingDataTexture;

    // --- Geometries ---
    Model* backgroundQuad;

    // --- Command Buffers ---
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> computeCommandBuffers;
    std::vector<VkCommandBuffer> offscreenCommandBuffers;
    bool swapBackground = false;
};
