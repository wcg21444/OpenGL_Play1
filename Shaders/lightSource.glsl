#include "ShadowMapping/shadowUnit.glsl"

struct DirLight
{
    vec3 pos;
    vec3 intensity;
    mat4 spaceMatrix;
    sampler2D depthMap;
    float farPlane;
    float orthoScale;
    sampler2D VSMTexture;
    int useVSM;
    sampler2D SATTexture;
};

struct PointLight
{
    vec3 pos;
    vec3 intensity;
    samplerCube depthCubemap;
    float farPlane;
    samplerCube VSMCubemap;
    int useVSM;
};