#pragma once

#include "Device.h"
#include "SwapChain.h"
#include "Scene.h"
#include "Camera.h"

#include "Image.h"
#include "shaderprogram/ShaderProgramIncludes.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_vulkan.h"

struct UIControlBufferObject {
    float farclip = 500.f;
    float transmittance_limit = 0.01f;

    float tiling_freq = 0.05f;

    float animate_speed = 10.f;
    glm::vec3 animate_offset = glm::vec3(0.1, 0.1, 0);

    bool enable_godray = true;
    float godray_exposure = 0.1f;

    // TODO: more lighting environment
};

class Renderer {
public:
    Renderer() = delete;
    Renderer(GLFWwindow* window, Device* device, SwapChain* swapChain, Scene* scene, Camera* camera);
    ~Renderer();

    void CreateUI();
    ImGuiIO* GetIO() const { return io; }
    bool MouseOverImGuiWindow() const { return mouseOverImGuiWindow; }
    void UpdateUIBuffer();

    void CreateCommandPools();

    void CreateRenderPass();
    void CreateOffscreenRenderPass();

    void CreateModels();
    void CreateDescriptors();
    void CreatePipelines();

    void CreateFrameResources();
    void DestroyFrameResources();
    void RecreateFrameResources();

    void RecordCommandBuffer(uint32_t index);
    void RecordCommandBuffers();
    void RecordOffscreenCommandBuffers();
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
    VkRenderPass offscreenRenderPass;

    // --- Shader programs ---
    PostShader* backgroundShader;
    ReprojectShader* reprojectShader;
    ComputeShader* computeShader;
    ComputeNubisCubedShader* computeNubisCubedShader;
    ComputeLightGridShader* computeLightGridShader;
    ComputeNearShader* computeNearShader;
    ComputeFarShader* computeFarShader;

    // --- Frame resources ---
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;

    std::vector<Texture*> offscreenTextures;
    std::vector<VkFramebuffer> offscreenFramebuffers;

    Texture* depthTexture;
    Texture* imageCurTexture;
    Texture* imagePrevTexture;

    Texture* nearCloudColorTexture;
    Texture* nearCloudDensityTexture;

    Texture* hiResCloudShapeTexture;
    Texture* lowResCloudShapeTexture;
    Texture* weatherMapTexture;
    Texture* curlNoiseTexture;

    Texture* modelingDataTexture;
    // Texture* fieldDataTexture;
    Texture* cloudDetailNoiseTexture;

    Texture* lightGridTexture;

    // --- Geometries ---
    Model* backgroundQuad;

    // --- Command Buffers ---
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> computeCommandBuffers;
    std::vector<VkCommandBuffer> offscreenCommandBuffers;
    bool swapBackground = false;

    // --- UI ---
    GLFWwindow* window;
    ImGuiIO* io;

    VkDescriptorPool uiDescriptorPool;

    bool mouseOverImGuiWindow = false;

    UIControlBufferObject uiControlBufferObject;
    UniformBuffer uiControlBuffer;
};
