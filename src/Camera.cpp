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
    radius = 450;
    target = glm::vec3(0.f, 0.f, 30.f);

    lookAtDir = glm::vec3(0.0f, 1.f, 0.0f);
    right = glm::vec3(1.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 0.0f, 1.0f);

    offset = glm::vec3(0.f);

    glm::vec3 eye = glm::vec3(0.0f, 450.0f, 30.f);
    cameraBufferObject.viewMatrix = glm::lookAt(eye, target, glm::vec3(0.0f, 0.0f, 1.0f));
    cameraBufferObject.projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    cameraBufferObject.projectionMatrix[1][1] *= -1; // y-coordinate is flipped
    cameraBufferObject.cameraPosition = glm::vec4(eye, 1.f);

    upRadius = sqrt(450 * 450 + 1);
    upDeg = atan(1.f / 450.f) * 180 / PI;

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

    UpdateOrbit(0.f, 0.f, 0.f);
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
    r = r - deltaZ;

    float radTheta = glm::radians(theta);
    float radPhi = glm::radians(phi);

    float upRad = glm::radians(phi + upDeg);
    glm::vec4 upPos = glm::vec4(-radius * glm::sin(radTheta), -radius * glm::cos(radTheta) * glm::cos(upRad), cameraBufferObject.cameraPosition.z + radius * glm::sin(upRad), 1.0f) + glm::vec4(offset.x, offset.y, 0.0, 0.0);
    //cameraBufferObject.cameraPosition = glm::vec4(-radius * glm::sin(radTheta), -radius * glm::cos(radTheta) * glm::cos(radPhi), cameraBufferObject.cameraPosition.z + radius * glm::sin(radPhi), 1.0f) + glm::vec4(offset.x, offset.y, 0.0, 0.0);
    //cameraBufferObject.cameraPosition = glm::vec4(cameraBufferObject.cameraPosition.x, radius * glm::cos(radPhi), radius * glm::sin(radPhi) + 30.f, 1.0f) + glm::vec4(offset.x, offset.y, 0.0, 0.0);
    cameraBufferObject.cameraPosition = glm::vec4(-radius * glm::sin(radTheta), -radius * glm::cos(radTheta), cameraBufferObject.cameraPosition.z, 1.0f) + glm::vec4(offset.x, offset.y, 0.0, 0.0);

    glm::vec3 vecUp = glm::normalize(glm::vec3(upPos - cameraBufferObject.cameraPosition));

    cameraBufferObject.viewMatrix = glm::lookAt(glm::vec3(cameraBufferObject.cameraPosition), target, glm::normalize(up));

    //std::cout << cameraBufferObject.cameraPosition.x << " " << cameraBufferObject.cameraPosition.y << " " << cameraBufferObject.cameraPosition.z << std::endl;
    //std::cout << vecUp.x << " " << vecUp.y << " " << vecUp.z << std::endl;
    //std::cout << target.x << " " << target.y << " " << target.z << std::endl;
    std::cout << glm::dot(vecUp, glm::vec3(cameraBufferObject.cameraPosition) - target) << std::endl;
    std::cout << upPos.x << " " << upPos.y << " " << upPos.z << std::endl;
    std::cout << phi + upDeg << std::endl;

    lookAtDir = -glm::vec3(cameraBufferObject.viewMatrix[0][2], cameraBufferObject.viewMatrix[1][2], cameraBufferObject.viewMatrix[2][2]);
    right = glm::vec3(cameraBufferObject.viewMatrix[0][0], cameraBufferObject.viewMatrix[1][0], cameraBufferObject.viewMatrix[2][0]);
    up = glm::vec3(cameraBufferObject.viewMatrix[0][1], cameraBufferObject.viewMatrix[1][1], cameraBufferObject.viewMatrix[2][1]);

    memcpy(camBuffer.mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}

void Camera::RotateCam(Direction dir)
{
    switch (dir) {
    case UP:
        target += 0.5;
        break;
    case DOWN:
        theta -= 0.5;
        break;
    case LEFT:
        phi += 0.01;
        break;
    case RIGHT:
        phi -= 0.01;
        break;
    default: return;
    }

    

    float radTheta = glm::radians(theta);
    float radPhi = glm::radians(phi);

    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), radTheta, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), radPhi, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 finalTransform = glm::translate(glm::mat4(1.0f), glm::vec3(cameraBufferObject.cameraPosition))
        * rotation
        * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, r));

    cameraBufferObject.viewMatrix = glm::inverse(finalTransform);

    lookAtDir = -glm::vec3(cameraBufferObject.viewMatrix[0][2], cameraBufferObject.viewMatrix[1][2], cameraBufferObject.viewMatrix[2][2]);
    right = glm::vec3(cameraBufferObject.viewMatrix[0][0], cameraBufferObject.viewMatrix[1][0], cameraBufferObject.viewMatrix[2][0]);
    up = glm::vec3(cameraBufferObject.viewMatrix[0][1], cameraBufferObject.viewMatrix[1][1], cameraBufferObject.viewMatrix[2][1]);

    memcpy(camBuffer.mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}

void Camera::UpdatePosition(Direction dir)
{
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
    case LEFT:
        if (glm::length(right) != 0.f)
            vecDir = glm::normalize(right);
        break;
    case RIGHT:
        if (glm::length(right) != 0.f)
            vecDir = -glm::normalize(right);
        break;
    case UP:
        if (glm::length(up) != 0.f)
            vecDir = -glm::normalize(up);
        break;
    case DOWN:
        if (glm::length(up) != 0.f)
            vecDir = glm::normalize(up);
        break;
    default: return;
    } 
    cameraBufferObject.cameraPosition += glm::vec4(5.f * vecDir, 1.0);
    target += 5.f * vecDir;
    offset += 5.f * vecDir;
    radius = glm::abs(450.f - 5.f * vecDir.y);
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
