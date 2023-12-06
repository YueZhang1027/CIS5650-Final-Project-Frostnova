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

    target = glm::vec3(0.0f, 30.0f, 0.0f);

    glm::vec3 eye = glm::vec3(0.0f, 0.0f, 0.0f);
    lookAtDir = target - eye;
    cameraBufferObject.viewMatrix = glm::lookAt(eye, eye + lookAtDir, glm::vec3(0.0f, 0.0f, 1.0f));
    cameraBufferObject.projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    cameraBufferObject.projectionMatrix[1][1] *= -1; // y-coordinate is flipped
    cameraBufferObject.cameraPosition = glm::vec4(eye, 1.0f);

    // phi, theta


    camBuffer.MapMemory(device, sizeof(CameraBufferObject));
    memcpy(camBuffer.mappedData, &cameraBufferObject, sizeof(CameraBufferObject));

    // Create a second buffer for the previous frame
    prevCameraBufferObject.CopyFrom(cameraBufferObject);
    prevCamBuffer.MapMemory(device, sizeof(CameraBufferObject));
    memcpy(prevCamBuffer.mappedData, &prevCameraBufferObject, sizeof(CameraBufferObject));

    // Create a buffer for camera parameters
    cameraParamBufferObject.aspectRatio = aspectRatio;
    cameraParamBufferObject.halfTanFOV = tan(glm::radians(45.0f / 2.0f));
    cameraParamBufferObject.pixelOffset = 0;

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
    glm::mat4 finalTransform = glm::translate(glm::mat4(1.0f), glm::vec3(cameraBufferObject.cameraPosition)) 
        * rotation 
        * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, r));

    cameraBufferObject.viewMatrix = glm::inverse(finalTransform);
    cameraBufferObject.cameraPosition = finalTransform * glm::vec4(0.f, 0.f, 0.f, 1.0f);

    target = glm::vec3(finalTransform * glm::vec4(0.f, 0.f, -30.f, 1.0f));
    lookAtDir = target - glm::vec3(cameraBufferObject.cameraPosition);
    std::cout << lookAtDir.x << " " << lookAtDir.y << " " << lookAtDir.z << std::endl;

    memcpy(camBuffer.mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}

void Camera::UpdatePosition(Direction dir)
{
    std::cout << cameraBufferObject.cameraPosition.x << " " << cameraBufferObject.cameraPosition.y << " " << cameraBufferObject.cameraPosition.z << std::endl;

    glm::vec3 vecDir;
    switch (dir) {
    case FORWARD:
        if(glm::length(lookAtDir) != 0.f)
            vecDir = glm::normalize(lookAtDir);
        break;
    case BACKWARD:
        if (glm::length(lookAtDir) != 0.f)
            vecDir = -glm::normalize(lookAtDir);
        break;
    default: return;
    }

    std::cout << vecDir.x << " " << vecDir.y << " " << vecDir.z << std::endl;
    
    cameraBufferObject.cameraPosition += glm::vec4(30.f * vecDir, 1.0);

    std::cout << cameraBufferObject.cameraPosition.x << " " << cameraBufferObject.cameraPosition.y << " " << cameraBufferObject.cameraPosition.z << std::endl;

    memcpy(camBuffer.mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}

void Camera::UpdatePrevBuffer() {
    prevCameraBufferObject.CopyFrom(cameraBufferObject);
    memcpy(prevCamBuffer.mappedData, &prevCameraBufferObject, sizeof(CameraBufferObject));
}

void Camera::UpdatePixelOffset() {
    cameraParamBufferObject.pixelOffset = (cameraParamBufferObject.pixelOffset + 1) % 16;
	memcpy(cameraParamBuffer.mappedData, &cameraParamBufferObject, sizeof(CameraParamBufferObject));
}

Camera::~Camera() {
    camBuffer.Clean(device);
    prevCamBuffer.Clean(device);
    cameraParamBuffer.Clean(device);
}
