
struct DirShadowUnit
{
    mat4 spaceMatrix;
    sampler2D depthMap;
    sampler2D VSMTexture;
    sampler2D SATTexture;
    vec3 pos;
    float farPlane;
    float nearPlane;
    float orthoScale;
};

const int MAX_CASCADES = 6;
struct CSMComponent
{
    DirShadowUnit units[MAX_CASCADES];
    int useVSM;
    int useVSSM;
};

// struct CSMShadowUnit
// {
//     mat4 spaceMatrixArray[MAX_CASCADES];
//     sampler2DArray depthMapArray;
//     sampler2DArray VSMTextureArray;
//     sampler2DArray SATTextureArray;
//     float farPlaneArray[MAX_CASCADES];
//     float nearPlaneArray[MAX_CASCADES];
//     float orthoScaleArray[MAX_CASCADES];
//     int cascadeCount;
// };