
#pragma once

#include <glm/glm.hpp>
#include "Device.h"
#include "BufferUtils.h"

enum Direction
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

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
    int pixelOffset; // [0 - 16)
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
    float radius;
    glm::vec3 target;

    glm::vec3 lookAtDir;
    glm::vec3 right;
    glm::vec3 up;

public:
    Camera(Device* device, float aspectRatio);
    ~Camera();

    VkBuffer GetPrevBuffer() const;
    VkBuffer GetBuffer() const;
    VkBuffer GetCameraParamBuffer() const;
    
    void UpdateOrbit(float deltaX, float deltaY, float deltaZ);
    void UpdatePosition(Direction dir);
    void UpdatePrevBuffer();
    void UpdatePixelOffset();
};
