
struct DirShadowUnit
{
    mat4 spaceMatrix;
    sampler2D depthMap;
    sampler2D VSMTexture;
    sampler2D SATTexture;
    float farPlane;
    float nearPlane;
    float orthoScale;
};

struct PointShadowUnit
{
    samplerCube depthCubemap;
    samplerCube VSMCubemap;
    float farPlane;
    float nearPlane;
};

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