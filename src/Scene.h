#pragma once

#include <glm/glm.hpp>
#include <chrono>

#include "Model.h"

#define PI 3.14159265f

using namespace std::chrono;

struct Time {
    float deltaTime = 0.0f;
    float totalTime = 0.0f;
    float sunPositionX;
    float sunPositionY;
    float sunPositionZ;
};

class Scene {
private:
    Device* device;
    
    VkBuffer timeBuffer;
    VkDeviceMemory timeBufferMemory;
    Time time;
    float theta = 0.0f;
    
    void* mappedData;
    high_resolution_clock::time_point startTime = high_resolution_clock::now();

public:
    Scene() = delete;
    Scene(Device* device);
    ~Scene();

    VkBuffer GetTimeBuffer() const;

    void UpdateTime(bool controlAngle = false, float customTheta = 0.0f);
    float GetTheta() const { return theta * 180.f / PI; }
};
