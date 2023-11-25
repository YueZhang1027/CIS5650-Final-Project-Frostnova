#include <iostream>

#define PI                3.1415926535897932384626422832795028841971f
#define GLM_FORCE_RADIANS
// Use Vulkan depth range of 0.0 to 1.0 instead of OpenGL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

Camera::Camera(Device* device, float aspectRatio) : device(device) {
    r = 10.0f;
    theta = 0.0f;
    phi = 0.0f;

    glm::vec3 eye = glm::vec3(0.0f, 325.0f, 10.0f);
    cameraBufferObject.viewMatrix = glm::lookAt(eye, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    cameraBufferObject.projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    cameraBufferObject.projectionMatrix[1][1] *= -1; // y-coordinate is flipped
    cameraBufferObject.cameraPosition = glm::vec4(eye, 1.0f);

    camBuffer.MapMemory(device, sizeof(CameraBufferObject));
    memcpy(camBuffer.mappedData, &cameraBufferObject, sizeof(CameraBufferObject));

    // Create a second buffer for the previous frame
    prevCameraBufferObject.CopyFrom(cameraBufferObject);
    prevCamBuffer.MapMemory(device, sizeof(CameraBufferObject));
    memcpy(prevCamBuffer.mappedData, &prevCameraBufferObject, sizeof(CameraBufferObject));

    // Create a buffer for camera parameters
    float fovy = 45.0f;
    float yscaled = tan(fovy * (PI / 180));
    float xscaled = (yscaled * 1920.f) / 1080.f;
    float fovx = (atan(xscaled) * 180) / PI;
    cameraParamBufferObject.fov = glm::vec2(fovx, fovy);
    cameraParamBufferObject.pixelLength = glm::vec2(2 * xscaled / (float)1920.f,
        2 * yscaled / (float)1080.f);

    cameraParamBuffer.MapMemory(device, sizeof(CameraParamBufferObject));
    memcpy(cameraParamBuffer.mappedData, &cameraParamBufferObject, sizeof(CameraParamBufferObject));
}

VkBuffer Camera::GetPrevBuffer() const {
    return prevCamBuffer.buffer;
}

VkBuffer Camera::GetBuffer() const {
    return camBuffer.buffer;
}

VkBuffer Camera::GetCameraParamBuffer() const {
	return cameraParamBuffer.buffer;
}

void Camera::UpdateOrbit(float deltaX, float deltaY, float deltaZ) {
    theta += deltaX;
    phi += deltaY;
    r = glm::clamp(r - deltaZ, 1.0f, 50.0f);

    float radTheta = glm::radians(theta);
    float radPhi = glm::radians(phi);

    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), radTheta, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), radPhi, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 finalTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)) * rotation * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, r));

    cameraBufferObject.viewMatrix = glm::inverse(finalTransform);
    cameraBufferObject.cameraPosition = glm::vec4(finalTransform[0][3], finalTransform[1][3], finalTransform[2][3], 1.0f);

    memcpy(camBuffer.mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}

void Camera::UpdatePrevBuffer() {
    prevCameraBufferObject.CopyFrom(cameraBufferObject);
    memcpy(prevCamBuffer.mappedData, &prevCameraBufferObject, sizeof(CameraBufferObject));
}

Camera::~Camera() {
    camBuffer.Clean(device);
    prevCamBuffer.Clean(device);
    cameraParamBuffer.Clean(device);
}
