#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D texColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

vec3 uc2Tonemap(vec3 x)
{
   return ((x*(0.15*x+0.1*0.5)+0.2*0.02)/(x*(0.15*x+0.5)+0.2*0.3))-0.02/0.3;
}

vec3 tonemap(vec3 x, float exposure, float invGamma, float whiteBalance) {
    vec3 white = vec3(whiteBalance);
    vec3 color = uc2Tonemap(exposure * x);
    vec3 whitemap = 1.0 / uc2Tonemap(white);
    color *= whitemap;
    return pow(color, vec3(invGamma));
}

void main() {
    vec3 col = texture(texColor, fragTexCoord).xyz;

    // float whitepoint = 1.0;
    // col = tonemap(col, 0.7, 1.0 / 2.2, whitepoint);
    // 
    // float vignette = dot(fragTexCoord - 0.5, fragTexCoord - 0.5);
    // 
    // col = mix(col, vec3(0.1, 0.05, 0.13), vignette);
    outColor = vec4(col, 1.0);
}
