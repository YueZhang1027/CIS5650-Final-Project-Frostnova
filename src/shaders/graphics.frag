#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform CameraBufferObject {
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
} camera;

layout(set = 0, binding = 1) uniform CameraBufferObjectPrev {
    mat4 view;
    mat4 proj;
    vec4 cameraPos;
} cameraPrev;

layout(set = 1, binding = 0) uniform Time {
    float deltaTime;
    float totalTime;
};

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0.5 * (fragTexCoord + vec2(1.0)), 0.5 * (sin(3.14159 * totalTime * 0.1) + 1.0), 1.0); //texture(texSampler, fragTexCoord);
}
