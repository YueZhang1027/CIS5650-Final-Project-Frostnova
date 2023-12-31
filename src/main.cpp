#include <vulkan/vulkan.h>
#include "Instance.h"
#include "Window.h"
#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"
#include "Image.h"

Device* device;
SwapChain* swapChain;
Renderer* renderer;
Camera* camera;

namespace {
    void resizeCallback(GLFWwindow* window, int width, int height) {
        if (width == 0 || height == 0) return;

        vkDeviceWaitIdle(device->GetVkDevice());
        swapChain->Recreate();
        renderer->RecreateFrameResources();
    }

    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
            case GLFW_KEY_W:
                camera->UpdatePosition(Direction::FORWARD);
                break;
            case GLFW_KEY_S:
                camera->UpdatePosition(Direction::BACKWARD);
                break;
            case GLFW_KEY_A:
                camera->UpdatePosition(Direction::LEFT);
                break;
            case GLFW_KEY_D:
                camera->UpdatePosition(Direction::RIGHT);
                break;
            case GLFW_KEY_1:
                camera->UpdatePosition(Direction::UP);
                break;
            case GLFW_KEY_2:
                camera->UpdatePosition(Direction::DOWN);
                break;
            case GLFW_KEY_LEFT:
                camera->RotateCam(Direction::LEFT);
                break;
            case GLFW_KEY_RIGHT:
                camera->RotateCam(Direction::RIGHT);
                break;
            case GLFW_KEY_UP:
                camera->RotateCam(Direction::UP);
                break;
            case GLFW_KEY_DOWN:
                camera->RotateCam(Direction::DOWN);
                break;
        }
    }

    bool leftMouseDown = false;
    bool rightMouseDown = false;
    double previousX = 0.0;
    double previousY = 0.0;

    void mouseDownCallback(GLFWwindow* window, int button, int action, int mods) {
        renderer->GetIO()->AddMouseButtonEvent(button, action);

        if (renderer->MouseOverImGuiWindow()) {
            return;
        }

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                leftMouseDown = true;
                glfwGetCursorPos(window, &previousX, &previousY);
            }
            else if (action == GLFW_RELEASE) {
                leftMouseDown = false;
            }
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                rightMouseDown = true;
                glfwGetCursorPos(window, &previousX, &previousY);
            }
            else if (action == GLFW_RELEASE) {
                rightMouseDown = false;
            }
        }
    }

    void mouseMoveCallback(GLFWwindow* window, double xPosition, double yPosition) {
        renderer->GetIO()->AddMousePosEvent(xPosition, yPosition);

        if (renderer->MouseOverImGuiWindow()) {
            return;
        }

        if (leftMouseDown) {
            double sensitivity = 0.5;
            float deltaX = static_cast<float>((previousX - xPosition) * sensitivity);
            float deltaY = static_cast<float>((previousY - yPosition) * sensitivity);

            camera->UpdateOrbit(deltaX, deltaY, 0.0f);

            previousX = xPosition;
            previousY = yPosition;
        } else if (rightMouseDown) {
            double deltaZ = static_cast<float>((previousY - yPosition) * 0.05);

            camera->UpdateOrbit(0.0f, 0.0f, deltaZ);

            previousY = yPosition;
        }
    }
}

int main() {
    static constexpr char* applicationName = "Vulkan Cloud Rendering";
    InitializeWindow(1920, 1080, applicationName);

    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    Instance* instance = new Instance(applicationName, glfwExtensionCount, glfwExtensions);

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance->GetVkInstance(), GetGLFWWindow(), nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }

    instance->PickPhysicalDevice({ VK_KHR_SWAPCHAIN_EXTENSION_NAME }, QueueFlagBit::GraphicsBit | QueueFlagBit::TransferBit | QueueFlagBit::ComputeBit | QueueFlagBit::PresentBit, surface);

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.tessellationShader = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    device = instance->CreateDevice(QueueFlagBit::GraphicsBit | QueueFlagBit::TransferBit | QueueFlagBit::ComputeBit | QueueFlagBit::PresentBit, deviceFeatures);

    swapChain = device->CreateSwapChain(surface, 5); // TODO: check numBuffers
    // the length of the array is equal to the total number of render passes - 1

    camera = new Camera(device, 1920.f / 1080.f);

    Scene* scene = new Scene(device);
    renderer = new Renderer(GetGLFWWindow(), device, swapChain, scene, camera);

    glfwSetWindowSizeCallback(GetGLFWWindow(), resizeCallback);
    glfwSetKeyCallback(GetGLFWWindow(), keyCallback);
    glfwSetMouseButtonCallback(GetGLFWWindow(), mouseDownCallback);
    glfwSetCursorPosCallback(GetGLFWWindow(), mouseMoveCallback);

    while (!ShouldQuit()) {
        glfwPollEvents();
        renderer->Frame();
        renderer->UpdateUniformBuffers();
    }

    vkDeviceWaitIdle(device->GetVkDevice());

    delete scene;
    delete camera;
    delete renderer;
    delete swapChain;
    delete device;
    delete instance;
    DestroyWindow();
    return 0;
}
