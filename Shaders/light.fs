
#version 330 core
out vec4 LightResult;
in vec2 TexCoord;

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
/*****************视口大小******************************************************************/
uniform int width = 1600;
uniform int height = 900;

/*****************GBuffer输入*****************************************************************/
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform samplerCube depthMap; // debug

/*****************SSAO输入******************************************************************/
uniform sampler2D ssaoTex;

/*****************点光源设置******************************************************************/
const int MAX_POINT_LIGHTS = 3; // Maximum number of lights supported
uniform int numPointLights;     // actual number of lights used
uniform PointLight pointLightArray[MAX_POINT_LIGHTS];

/*****************定向光源设置******************************************************************/
const int MAX_DIR_LIGHTS = 5; // Maximum number of lights supported
uniform int numDirLights;     // actual number of lights used
uniform DirLight dirLightArray[MAX_DIR_LIGHTS];
vec3 sunlightDecay;
/*****************阴影采样设置******************************************************************/
vec2 noiseScale = vec2(width / 16.0, height / 16.0);
uniform sampler2D shadowNoiseTex;
uniform vec3 shadowSamples[128];
uniform int n_samples;
uniform float blurRadius = 0.1f;

/*****************环境光******************************************************************/
uniform vec3 ambientLight;

/*****************天空盒******************************************************************/
uniform vec3 skyboxSamples[32];
uniform samplerCube skybox;
uniform mat4 view; // 视图矩阵

/*****************TBN******************************************************************/
vec3 randomVec = vec3(texture(shadowNoiseTex, TexCoord *noiseScale).xy / 2, 1.0f);
vec3 normal = texture(gNormal, TexCoord).rgb;
vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal)); // WorldSpace
vec3 bitangent = cross(normal, tangent);
mat3 TBN = mat3(tangent, bitangent, normal);

/*****************Camera设置******************************************************************/
uniform float nearPlane;
uniform float farPlane;
uniform vec3 eyePos;
uniform vec3 eyeFront;
uniform vec3 eyeUp;
uniform float fov;

/*****************toggle设置******************************************************************/
uniform int SSAO;
uniform int DirShadow;
uniform int PointShadow;
uniform int Skybox;

/*****************************天空大气计算********************************************************** */

uniform vec4 betaMie = vec4(21e-6, 21e-6, 21e-6, 1.0f);
uniform int maxStep;
uniform float atmosphereDensity; // 大气密度
uniform float MieDensity;
uniform float gMie;
uniform float absorbMie;
uniform float MieIntensity;
uniform float skyHeight;
uniform float earthRadius;
uniform float skyIntensity;
uniform float HRayleigh;
uniform float HMie;
uniform sampler2D transmittanceLUT;

#include "SkyTexPass/geometry.glsl"
#include "SkyTexPass/scatter.glsl"
#include "SkyTexPass/skyAtmosphere.glsl"

#include "Raymarching/raymarching.glsl"
/*******************************Lighting******************************************************/
#include "ShadowMapping/shadow.glsl"

vec3 fragViewSpaceDir(vec2 uv)
{
    vec3 dir = vec3(uv - vec2(0.5f, 0.5f), 0.f);
    dir.y = dir.y * tan(radians(fov / 2)) * 2;
    dir.x = dir.x * tan(radians(fov / 2)) * (float(width) / float(height)) * 2;
    dir.z = -1.0f;
    dir = normalize(inverse(mat3(view)) * dir);
    return dir;
}

vec3 sampleSkybox(vec2 uv, samplerCube _skybox)
{
    // vec3 uv_centered = vec3(uv-vec2(0.5f,0.5f),0.f);
    vec3 dir = fragViewSpaceDir(uv);
    // vec3 dir = normalize(eyeFront+nearPlane*uv_centered*2*tan(radians(fov)/2));
    return vec3(texture(_skybox, dir));
    // return dir;
}

vec3 computeSkyboxAmbient(samplerCube _skybox)
{
    vec3 ambient = vec3(0.f);
    int samplesN = 32;
    for (int i = 0; i < samplesN; ++i)
    {
        vec3 sample_dir = TBN * (skyboxSamples[i] + vec3(0.0, 0.0, 1.f));
        ambient += texture(_skybox, normalize(sample_dir)).rgb;
    }
    return ambient / samplesN;
}

vec3 computeSkyboxAmbientMipMap(samplerCube _skybox, vec3 dir)
{
    vec3 ambient = textureLod(_skybox, dir, 8).rgb;

    return ambient;
}

vec3 dirLightDiffuse(vec3 fragPos, vec3 n)
{

    vec3 diffuse = vec3(0.0f);
    for (int i = 0; i < numDirLights; ++i)
    {

        vec3 l = normalize(dirLightArray[i].pos);
        float rr = dot(l, l);
        float shadowFactor = 0.0f;
        if (dirLightArray[i].useVSM == 0)
        {
            shadowFactor = 1 - computeDirLightShadow(fragPos, n, dirLightArray[i]);
        }
        else
        {
            shadowFactor = 1 - computeDirLightShadowVSM(fragPos, n, dirLightArray[i]);
        }
        if (i == 0) // 太阳光处理
        {
            diffuse += shadowFactor * dirLightArray[i].intensity / rr * max(0.f, dot(n, l)) * sunlightDecay;
        }
        else
        {
            diffuse +=
                shadowFactor * dirLightArray[i].intensity / rr * max(0.f, dot(n, l));
        }
    }
    return diffuse;
}

vec3 dirLightSpec(vec3 fragPos, vec3 n)
{
    vec3 specular = vec3(0.0f);
    for (int i = 0; i < numDirLights; ++i)
    {
        // 太阳光处理

        vec3 l = normalize(dirLightArray[i].pos);
        float specularStrength = 0.01f;
        vec3 viewDir = normalize(eyePos - fragPos);
        vec3 reflectDir = reflect(-l, n);
        float shadowFactor = 0.0f;
        if (dirLightArray[i].useVSM == 0)
        {
            shadowFactor = 1 - computeDirLightShadow(fragPos, n, dirLightArray[i]);
        }
        else
        {
            shadowFactor = 1 - computeDirLightShadowVSM(fragPos, n, dirLightArray[i]);
        }
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);

        if (i == 0) // 太阳光处理
        {
            specular += shadowFactor * specularStrength * spec * dirLightArray[i].intensity * sunlightDecay * 160;
        }
        else
        {
            specular += shadowFactor * specularStrength * spec * dirLightArray[i].intensity * 160;
        }
    }
    return specular;
}

vec3 pointLightDiffuse(vec3 fragPos, vec3 n)
{
    vec3 diffuse = vec3(0.f, 0.f, 0.f);

    for (int i = 0; i < numPointLights; ++i)
    {
        vec3 l = pointLightArray[i].pos - fragPos;
        float rr = pow(dot(l, l), 0.6) * 10;
        l = normalize(l);

        float shadow_factor = 0.0f;
        if (pointLightArray[i].useVSM == 0)
        {
            shadow_factor = 1 - computePointLightShadow(fragPos, n, pointLightArray[i]);
        }
        else
        {
            shadow_factor = 1 - computePointLightShadowVSM(fragPos, n, pointLightArray[i]);
        }
        diffuse += pointLightArray[i].intensity / rr * max(0.f, dot(n, l)) * shadow_factor;
    }
    return diffuse;
}

vec3 pointLightSpec(vec3 fragPos, vec3 n)
{
    vec3 specular = vec3(0.f, 0.f, 0.f);

    for (int i = 0; i < numPointLights; ++i)
    {
        vec3 l = pointLightArray[i].pos - fragPos;
        float rr = pow(dot(l, l), 0.6) * 10;
        l = normalize(l);
        float spec = 0.f;
        float specularStrength = 0.005f;

        float shadow_factor = 0.0f;
        if (pointLightArray[i].useVSM == 0)
        {
            shadow_factor = 1 - computePointLightShadow(fragPos, n, pointLightArray[i]);
        }
        else
        {
            shadow_factor = 1 - computePointLightShadowVSM(fragPos, n, pointLightArray[i]);
        }

        vec3 viewDir = normalize(eyePos - fragPos);
        vec3 reflectDir = reflect(-l, n);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        specular += specularStrength * spec * pointLightArray[i].intensity * shadow_factor;
    }
    return specular;
}

void initializeAtmosphereParameters()
{

    camDir = fragViewSpaceDir(TexCoord);
    camPos = eyePos;
    earthCenter = vec3(0.0f, -earthRadius, 0.0f); // 地球球心，位于地面原点正下方
    sunDir = dirLightArray[0].pos;
    sunlightDecay = computeSunlightDecay(camPos, camDir, dirLightArray[0].pos);
}

void computeSkyAtmosphere(in out vec4 LightResult, in vec3 ambient, in vec3 n)
{
    vec3 camEarthIntersection = intersectEarth(camPos, camDir);

    if (camEarthIntersection == NO_INTERSECTION)
    {
        if (Skybox == 1) // 开启天空盒
        {
            LightResult.rgb += vec4(sampleSkybox(TexCoord, skybox), 1.0f).rgb; // 采样天空盒
        }
        else
        {
            LightResult.rgb += computeSkyColor(dirLightArray[0].intensity).rgb;
        }
    }
    else
    {
        // 击中地球,渲染大气透视
        LightResult.rgb += computeAerialPerspective(camEarthIntersection, dirLightArray[0].intensity).rgb;

        vec4 t1 = transmittance(camPos, camEarthIntersection, 1.0f);
        // vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, camEarthIntersection);
        ambient += computeSkyboxAmbientMipMap(skybox, n);
        // 渲染地面
        vec3 normal = normalize(camEarthIntersection - earthCenter);
        vec3 lighting = dirLightDiffuse(camEarthIntersection, normal) + ambient;
        vec3 earthBaseColor = vec3(0.3, 0.3f, 0.34f); // 地面颜色
        LightResult.rgb += (lighting) * (earthBaseColor)*t1.rgb;
    }

    if (camEarthIntersection == NO_INTERSECTION)
    {
        LightResult.rgb += generateSunDisk(camPos, camDir, sunDir, dirLightArray[0].intensity, 2.0f);
    }
}

void computeSceneAtmosphere(in vec3 FragPos, in out vec4 SceneColor)
{
    vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, FragPos);
    SceneColor *= t1;
    SceneColor.rgb += computeAerialPerspective(FragPos, dirLightArray[0].intensity).rgb;
}
void main()
{
    // Diffuse Caculation

    vec3 FragPos = texture(gPosition, TexCoord).rgb;
    vec3 n = normalize(texture(gNormal, TexCoord).rgb);
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;
    LightResult = vec4(0.f, 0.f, 0.f, 1.f);

    initializeAtmosphereParameters();

    // 天空计算
    if (length(FragPos) == 0)
    {
        computeSkyAtmosphere(LightResult, ambient, n);
    }
    else
    {
        vec4 sceneColor = vec4(0.0f);
        diffuse =
            pointLightDiffuse(FragPos, n) +
            dirLightDiffuse(FragPos, n);

        specular =
            pointLightSpec(FragPos, n) +
            dirLightSpec(FragPos, n);

        vec3 ambient = ambientLight;

        ambient += computeSkyboxAmbientMipMap(skybox, n);
        // ambient = (ambientLight +
        //            computeSkyboxAmbient(skybox));

        sceneColor += vec4(diffuse * texture(gAlbedoSpec, TexCoord).rgb, 1.f);
        sceneColor += vec4(specular * texture(gAlbedoSpec, TexCoord).a, 1.f);
        sceneColor += vec4(ambient * texture(gAlbedoSpec, TexCoord).rgb, 1.f);

        computeSceneAtmosphere(FragPos, sceneColor);
        LightResult += sceneColor;
    }

    const vec3 BoxMin = vec3(2.0f, -2.0f, -2.0f);
    const vec3 BoxMax = vec3(6.0f, 2.0f, 2.0f);
    vec3 dir = fragViewSpaceDir(TexCoord);

    // vec4 cloud = cloudRayMarching(eyePos.xyz, dir, FragPos, BoxMin, BoxMax, vec4(0.5f, 0.5f, 0.5f, 1.0f));
    // LightResult -= cloud; // 散射吸收,减色

    const vec3 LightVolueBoxMin = vec3(-0.5f, -0.5f, -0.5f);
    const vec3 LightVolueBoxMax = vec3(0.5f, 0.5f, 0.5f);
    for (int i = 0; i < numPointLights; ++i)
    {
        LightResult += lightVolume(
            eyePos.xyz,
            dir,
            FragPos,
            LightVolueBoxMin + pointLightArray[i].pos,
            LightVolueBoxMax + pointLightArray[i].pos,
            vec4(pow(pointLightArray[i].intensity / 8.f, vec3(1.2f)), 1.0f));
    }
}
