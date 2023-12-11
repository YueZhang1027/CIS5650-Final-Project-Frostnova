#include "Scene.h"
#include "BufferUtils.h"

const float ONE_DAY = 30.0f;
const float SUN_DISTANCE = 400000.0f;

Scene::Scene(Device* device) : device(device) {
    BufferUtils::CreateBuffer(device, sizeof(Time), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, timeBuffer, timeBufferMemory);
    vkMapMemory(device->GetVkDevice(), timeBufferMemory, 0, sizeof(Time), 0, &mappedData);
    memcpy(mappedData, &time, sizeof(Time));
}

void Scene::UpdateTime(bool controlAngle, float customTheta) {
    high_resolution_clock::time_point currentTime = high_resolution_clock::now();
    duration<float> nextDeltaTime = duration_cast<duration<float>>(currentTime - startTime);
    startTime = currentTime;

    time.deltaTime = nextDeltaTime.count();
    time.totalTime += time.deltaTime;

    float dayTime = glm::mod(time.totalTime, ONE_DAY);
    theta = controlAngle ? (customTheta * PI / 180.f) : (dayTime * 2.0f * PI / ONE_DAY);
    float phi = 45;

    time.sunPositionY = SUN_DISTANCE * cos(theta) * cos(phi);
    time.sunPositionZ = -SUN_DISTANCE * sin(theta);
    time.sunPositionX = -SUN_DISTANCE * cos(theta) * sin(phi);
    
    memcpy(mappedData, &time, sizeof(Time));
}

VkBuffer Scene::GetTimeBuffer() const {
    return timeBuffer;
}

Scene::~Scene() {
    vkUnmapMemory(device->GetVkDevice(), timeBufferMemory);
    vkDestroyBuffer(device->GetVkDevice(), timeBuffer, nullptr);
    vkFreeMemory(device->GetVkDevice(), timeBufferMemory, nullptr);
}
