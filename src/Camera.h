
#pragma once

#include <glm/glm.hpp>
#include "Device.h"
#include "BufferUtils.h"

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

struct CameraParamBufferObject {
    float halfTanFOV;
    float aspectRatio;
};

class Camera {
private:
    Device* device;
    
    CameraBufferObject cameraBufferObject;
    UniformBuffer camBuffer;

    CameraBufferObject prevCameraBufferObject; // TODO: update this in the update loop
    UniformBuffer prevCamBuffer;

    CameraParamBufferObject cameraParamBufferObject;
    UniformBuffer cameraParamBuffer;

    float r, theta, phi;

public:
    Camera(Device* device, float aspectRatio);
    ~Camera();

    VkBuffer GetPrevBuffer() const;
    VkBuffer GetBuffer() const;
    VkBuffer GetCameraParamBuffer() const;
    
    void UpdateOrbit(float deltaX, float deltaY, float deltaZ);
    void UpdatePrevBuffer();
};
