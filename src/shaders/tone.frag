#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D texColor;

layout(set = 1, binding = 0) uniform CameraObject {
    mat4 view;
    mat4 proj;
    vec4 cameraPosition;
} camera;

layout(set = 2, binding = 0) uniform TimeObject {
    float deltaTime;
    float totalTime;
    float sunPositionX;
    float sunPositionY;
    float sunPositionZ;
} time;

layout (set = 3, binding = 0) uniform UIParamObject {
    float farclip;
    float transmittance_limit;

    int cloud_type;
    float tiling_freq;

    float animate_speed;
    vec3 animate_offset;

    float enable_godray;
    float godray_exposure;

    float sky_turbidity;
} uiParam;

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

vec4 GodRay()
{
    if(time.sunPositionZ > 0)
    {
        return vec4(0);
    }

    vec3 sunPos = vec3(time.sunPositionX, time.sunPositionY, time.sunPositionZ);
    vec3 sunDir = normalize(sunPos);

    float decay = 0.96;
    float exposure = mix(0.08, 0.02, clamp(-sunDir.z, 0, 1));
    float density = 0.2;
    float weight = 0.58767;

    int NUM_SAMPLES = 100;

    vec2 tc = fragTexCoord;
    vec4 sunScreenPos = camera.proj * camera.view * vec4(sunPos, 1.0);
    sunScreenPos /= sunScreenPos.w;

    vec2 deltaTexCoord = tc * 2.0 - 1.0 - sunScreenPos.xy;
    deltaTexCoord *= 1.0 / float(NUM_SAMPLES) * density;
    float illuminationDecay = 1.0;
    vec4 color = texture(texColor, tc) * 0.4;

    for(int i=0; i < NUM_SAMPLES ; i++)
    {
        tc -= deltaTexCoord;
        vec4 samp = texture(texColor, tc) * 0.4;
        samp *= illuminationDecay * weight;
        color += samp;
        illuminationDecay *= decay;
    }
    color.a = exposure;
    return color;
}

void main() {
    vec4 sceneCol = texture(texColor, fragTexCoord);

    //if (uiParam.enable_godray == 1.0f) {
        vec4 GodRayCol = GodRay();
        sceneCol += GodRayCol * GodRayCol.a;
    //}

    vec3 col = sceneCol.xyz;
    float whitepoint = 1.0;
    col = tonemap(col, 0.7, 1.0 / 2.2, whitepoint);
    
    float vignette = dot(fragTexCoord - 0.5, fragTexCoord - 0.5);
    
    col = mix(col, vec3(0.1, 0.05, 0.13), vignette);
   
    outColor = vec4(col, 1.0);
}
