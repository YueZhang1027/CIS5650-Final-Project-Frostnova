#include "Model.h"
#include "BufferUtils.h"
#include "Image.h"

// Create a model with a predefined shape
Model::Model(Device* device, VkCommandPool commandPool, ModelCreateFlags flag)
    : device(device), vertices(vertices) {
    switch (flag) {
        case ModelCreateFlags::BACKGROUND_QUAD:
            vertices = {
                { { -1.0f,  1.0f, 0.99f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
                { {  1.0f,  1.0f, 0.99f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
                { {  1.0f, -1.0f, 0.99f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
                { { -1.0f, -1.0f, 0.99f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } }
			};

            indices = {
				0, 1, 2, 2, 3, 0
			};

			break;
    }

    if (vertices.size() > 0) {
        BufferUtils::CreateBufferFromData(device, commandPool, this->vertices.data(), vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, vertexBufferMemory);
    }

    if (indices.size() > 0) {
        BufferUtils::CreateBufferFromData(device, commandPool, this->indices.data(), indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer, indexBufferMemory);
    }

    modelBufferObject.modelMatrix = glm::mat4(1.0f);
    BufferUtils::CreateBufferFromData(device, commandPool, &modelBufferObject, sizeof(ModelBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, modelBuffer, modelBufferMemory);
}

Model::Model(Device* device, VkCommandPool commandPool, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
  : device(device), vertices(vertices), indices(indices) {

    if (vertices.size() > 0) {
        BufferUtils::CreateBufferFromData(device, commandPool, this->vertices.data(), vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, vertexBufferMemory);
    }

    if (indices.size() > 0) {
        BufferUtils::CreateBufferFromData(device, commandPool, this->indices.data(), indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer, indexBufferMemory);
    }

    modelBufferObject.modelMatrix = glm::mat4(1.0f);
    BufferUtils::CreateBufferFromData(device, commandPool, &modelBufferObject, sizeof(ModelBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, modelBuffer, modelBufferMemory);
}

Model::~Model() {
    if (indices.size() > 0) {
        vkDestroyBuffer(device->GetVkDevice(), indexBuffer, nullptr);
        vkFreeMemory(device->GetVkDevice(), indexBufferMemory, nullptr);
    }

    if (vertices.size() > 0) {
        vkDestroyBuffer(device->GetVkDevice(), vertexBuffer, nullptr);
        vkFreeMemory(device->GetVkDevice(), vertexBufferMemory, nullptr);
    }

    vkDestroyBuffer(device->GetVkDevice(), modelBuffer, nullptr);
    vkFreeMemory(device->GetVkDevice(), modelBufferMemory, nullptr);
}

const std::vector<Vertex>& Model::getVertices() const {
    return vertices;
}

VkBuffer Model::getVertexBuffer() const {
    return vertexBuffer;
}

const std::vector<uint32_t>& Model::getIndices() const {
    return indices;
}

VkBuffer Model::getIndexBuffer() const {
    return indexBuffer;
}

const ModelBufferObject& Model::getModelBufferObject() const {
    return modelBufferObject;
}

VkBuffer Model::GetModelBuffer() const {
    return modelBuffer;
}

void Model::EnqueueDrawCommands(VkCommandBuffer& commandBuffer) {
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
}
