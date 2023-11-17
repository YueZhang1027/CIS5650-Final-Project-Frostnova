
#pragma once

#include <glm/glm.hpp>
#include "Device.h"

struct CameraBufferObject {
  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;
  glm::vec4 cameraPosition;

  void CopyFrom(CameraBufferObject& orig) {
      viewMatrix = orig.viewMatrix;
	  projectionMatrix = orig.projectionMatrix;
	  cameraPosition = orig.cameraPosition;
  }
};

class Camera {
private:
    Device* device;
    
    CameraBufferObject cameraBufferObject;
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    void* mappedData;

    CameraBufferObject prevCameraBufferObject; // TODO: update this in the update loop
    VkBuffer prevBuffer;
    VkDeviceMemory prevBufferMemory;
    void* prevMappedData;

    float r, theta, phi;

public:
    Camera(Device* device, float aspectRatio);
    ~Camera();

    VkBuffer GetPrevBuffer() const;
    VkBuffer GetBuffer() const;
    
    void UpdateOrbit(float deltaX, float deltaY, float deltaZ);
    void UpdatePrevBuffer();
};
