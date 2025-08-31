
#version 330 core
out vec4 LightResult;
in vec2 TexCoord;

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
const int MAX_LIGHTS = 10;  // Maximum number of lights supported
uniform int numPointLights; // actual number of lights used
uniform samplerCube shadowCubeMaps[MAX_LIGHTS];
uniform vec3 pointLightPos[MAX_LIGHTS];
uniform vec3 pointLightIntensity[MAX_LIGHTS];
uniform float pointLightFar;

/*****************定向光源设置******************************************************************/
const int MAX_DIR_LIGHTS = 5; // Maximum number of lights supported
uniform int numDirLights;     // actual number of lights used
uniform sampler2D dirDepthMap[MAX_DIR_LIGHTS];
uniform vec3 dirLightPos[MAX_DIR_LIGHTS];
uniform vec3 dirLightIntensity[MAX_DIR_LIGHTS];
uniform mat4 dirLightSpaceMatrix[MAX_DIR_LIGHTS];

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

uniform sampler2D transmittanceLUT;
/*******************************RayMarching输入**************************************************/

/*******************************RayMarching******************************************************/

vec3 fragViewSpaceDir(vec2 uv)
{
    vec3 dir = vec3(uv - vec2(0.5f, 0.5f), 0.f);
    dir.y = dir.y * tan(radians(fov / 2)) * 2;
    dir.x = dir.x * tan(radians(fov / 2)) * (float(width) / float(height)) * 2;
    dir.z = -1.0f;
    dir = normalize(inverse(mat3(view)) * dir);
    return dir;
}

bool isPointInBox(vec3 testPoint, vec3 boxMin, vec3 boxMax)
{
    return testPoint.x < boxMax.x && testPoint.x > boxMin.x &&
           testPoint.z < boxMax.z && testPoint.z > boxMin.z &&
           testPoint.y < boxMax.y && testPoint.y > boxMin.y;
}
vec2 rayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 invRaydir)
{
    vec3 t0 = (boundsMin - rayOrigin) * invRaydir;
    vec3 t1 = (boundsMax - rayOrigin) * invRaydir;
    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);

    float dstA = max(max(tmin.x, tmin.y), tmin.z); // 进入点
    float dstB = min(tmax.x, min(tmax.y, tmax.z)); // 出去点

    float dstToBox = max(0, dstA);
    float dstInsideBox = max(0, dstB - dstToBox);
    return vec2(dstToBox, dstInsideBox);
}

vec4 lightMarching(
    vec3 startPoint,
    vec3 direction,
    float depthLimit,
    vec3 boxMin,
    vec3 boxMax)
{
    vec3 testPoint = startPoint;

    float interval = 0.1; // 每次步进间隔
    float intensity = 0.01 * interval;

    int hit = 0;
    float sum = 0.0;
    for (int i = 0; i < 8; ++i)
    {
        // 步进总长度
        if (i * interval > depthLimit)
        {
            break;
        }
        testPoint += direction * interval;

        // 光线在体积内行进
        if (isPointInBox(testPoint, boxMin, boxMax))
        {
            hit = 1;

            sum += intensity / depthLimit;
        }
    }
    return -sum * vec4(dirLightIntensity[0] / length(dirLightIntensity[0]), 1.0f) / 1;
}

vec4 computeCloudRayMarching(
    vec3 startPoint,
    vec3 direction,
    float depthLimit,
    vec3 boxMin,
    vec3 boxMax,
    vec4 color)
{
    vec3 testPoint = startPoint;

    float interval = 0.01; // 每次步进间隔
    float intensity = 0.1 * interval;

    float density = 0.2;

    int hit = 0;
    float sum = 0.0;
    for (int i = 0; i < 256; ++i)
    {
        // 步进总长度
        if (i * interval > depthLimit)
        {
            break;
        }
        testPoint += direction * interval;

        // 光线在体积内行进
        if (isPointInBox(testPoint, boxMin, boxMax))
        {
            hit = 1;
            sum += intensity;
            sum *= exp(density * interval);

            // vec3 lightDir = pointLightPos[0]-testPoint;
            vec3 lightDir = normalize(dirLightPos[0]);
            float limit = rayBoxDst(boxMin, boxMax, testPoint, 1 / lightDir).y;

            color += lightMarching(testPoint, lightDir, limit, boxMin, boxMax);
        }
    }
    if (hit == 0)
    {
        return vec4(0.0f);
    }

    return sum * color;
}
vec4 cloudRayMarching(
    vec3 startPoint,
    vec3 direction,
    vec3 fragPos,
    vec3 boxMin,
    vec3 boxMax,
    vec4 color)
{
    vec2 rst = rayBoxDst(boxMin, boxMax, startPoint, 1.0 / direction);
    float boxDepth = rst.x;
    float boxInsideDepth = rst.y;

    float fragDepth = length(fragPos - startPoint);
    // skybox
    if (length(fragPos) == 0)
    {
        fragDepth = 10000000.f;
    }

    if (fragDepth < boxDepth)
    {
        // 物体阻挡Box
        return vec4(0.f);
    }
    float depthLimit;
    if ((fragDepth - boxDepth) < boxInsideDepth)
    {
        // 物体嵌入box,步进深度限制减小
        depthLimit = fragDepth - boxDepth;
    }
    else
    {
        depthLimit = boxInsideDepth;
    }
    return computeCloudRayMarching(startPoint + direction * rst.x, direction, depthLimit, boxMin, boxMax, color);
}
/*********************************LightVolume******************************************************/
vec4 computeLightVolumeRayMarching(
    vec3 startPoint,
    vec3 direction,
    float depthLimit,
    vec3 boxMin,
    vec3 boxMax,
    vec4 color)
{
    vec3 testPoint = startPoint;
    float sum = 0.0;
    float interval = 0.01; // 每次步进间隔
    float intensity = 1.5 * interval;
    float density = 0.1f;
    for (int i = 0; i < 1024; ++i)
    {
        // 步进总长度
        if (i * interval > depthLimit)
        {
            break;
        }
        testPoint += direction * interval;
        if (testPoint.x < boxMax.x && testPoint.x > boxMin.x &&
            testPoint.z < boxMax.z && testPoint.z > boxMin.z &&
            testPoint.y < boxMax.y && testPoint.y > boxMin.y)
            sum += intensity;
        sum *= exp(-density * interval);
    }
    return sum * color;
}
vec4 lightVolumeRayMarching(
    vec3 startPoint,
    vec3 direction,
    vec3 fragPos,
    vec3 boxMin,
    vec3 boxMax,
    vec4 color)
{
    vec2 rst = rayBoxDst(boxMin, boxMax, startPoint, 1.0 / direction);
    float boxDepth = rst.x;
    float boxInsideDepth = rst.y;

    float fragDepth = length(fragPos - startPoint);
    // skybox
    if (length(fragPos) == 0)
    {
        fragDepth = 10000000.f;
    }

    if (fragDepth < boxDepth)
    {
        // 物体阻挡Box
        return vec4(0.f);
    }
    float depthLimit;
    if ((fragDepth - boxDepth) < boxInsideDepth)
    {
        // 物体嵌入box,步进深度限制减小
        depthLimit = fragDepth - boxDepth;
    }
    else
    {
        depthLimit = boxInsideDepth;
    }
    return computeLightVolumeRayMarching(startPoint + direction * rst.x, direction, depthLimit, boxMin, boxMax, color);
}

vec4 lightVolume(
    vec3 startPoint,
    vec3 direction,
    vec3 fragPos,
    vec3 boxMin,
    vec3 boxMax,
    vec4 color)
{
    return lightVolumeRayMarching(startPoint, direction, fragPos, boxMin, boxMax, color);
}
/*******************************Lighting******************************************************/

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
    vec3 ambient = textureLod(_skybox, dir, 6).rgb;

    return ambient;
}

float computeDirLightShadow(vec3 fragPos, vec3 fragNormal, mat4 _dirLightSpaceMatrix, sampler2D _dirDepthMap)
{
    // perform perspective divide
    if (DirShadow == 0)
    {
        return 0.f;
    }
    vec4 lightSpaceFragPos = _dirLightSpaceMatrix * vec4(fragPos, 1.0f);

    // // 计算遮挡物与接受物的平均距离
    // float d = 0.f;
    // int occlusion_times = 0;
    // for (int k = 0; k < n_samples; ++k)
    // {
    //     vec4 sampleOffset = _dirLightSpaceMatrix * vec4(TBN * shadowSamples[k] * blurRadius, 0.0f);
    //     vec4 fragPosLightSpace = lightSpaceFragPos + sampleOffset; // Dir Light View Space
    //     vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
    //     // transform to [0,1] range
    //     projCoords = projCoords * 0.5 + 0.5;
    //     // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    //     float closestDepth = texture(_dirDepthMap, projCoords.xy).r;
    //     // get depth of current fragment from light's perspective
    //     float currentDepth = projCoords.z;

    //     float bias = 0.00003f;

    //     if (currentDepth - closestDepth - bias > 0)
    //     {
    //         occlusion_times++;
    //         d += (currentDepth - closestDepth) * 2000; // 深度值换算与光源farplane数值有关
    //     }
    // }
    // d = d / occlusion_times;
    // PCF Only
    float factor = 0.f;
    for (int j = 0; j < n_samples; ++j)
    {
        // vec4 sampleOffset = _dirLightSpaceMatrix * vec4(
        //                                                TBN *
        //                                                    shadowSamples[j] *
        //                                                    blurRadius * 20 * pow(d, 2),
        //                                                0.0f);
        vec4 sampleOffset = _dirLightSpaceMatrix * vec4(
                                                       TBN *
                                                           shadowSamples[j] *
                                                           blurRadius * 5.f,
                                                       0.0f);
        vec4 fragPosLightSpace = lightSpaceFragPos + sampleOffset; // Dir Light View Space
        vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(_dirDepthMap, projCoords.xy).r;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        float cosine = dot(fragNormal, dirLightPos[0]) / length(fragNormal) / length(dirLightPos[0]); // 只考虑阳光
        float texel = 5e-6;
        float bias = sqrt(1 - cosine * cosine) / cosine * texel;
        // check whether current frag pos is in shadow
        factor += currentDepth - closestDepth - bias > 0 ? 1.0f : 0.0f;
    }

    return factor / n_samples;
}

float computePointLightShadow(vec3 fragPos, vec3 fragNorm, vec3 pointLightPos, samplerCube _depthMap)
{
    if (PointShadow == 0)
    {
        return 0.f;
    }
    vec3 dir = fragPos - pointLightPos;

    float curr_depth = length(dir);
    float omega = -dot(fragNorm, dir) / curr_depth;
    float bias = 0.05f / tan(omega); // idea: bias increase based on center distance
    // shadow test

    // 计算遮挡物与接受物的平均距离
    float d = 0.f;
    int occlusion_times = 0;
    for (int k = 0; k < n_samples; ++k)
    {
        vec3 sampleOffset = TBN * shadowSamples[k] * 4.f;
        vec3 dir_sample = sampleOffset + fragPos - pointLightPos;
        float curr_depth_sample = length(dir_sample);
        float cloest_depth_sample = texture(_depthMap, dir_sample).r;

        cloest_depth_sample *= pointLightFar;
        if (curr_depth_sample - cloest_depth_sample - bias > 0.01f)
        {
            occlusion_times++;
            d += curr_depth_sample - cloest_depth_sample;
        }
    }
    d = d / occlusion_times;
    float factor = 0.f;
    for (int j = 0; j < n_samples; ++j)
    {
        vec3 sampleOffset = TBN * shadowSamples[j] * blurRadius * pow(curr_depth / 12, 2) * d / n_samples * 64;
        vec3 dir_sample = sampleOffset + fragPos - pointLightPos;
        float curr_depth_sample = length(dir_sample);
        float cloest_depth_sample = texture(_depthMap, dir_sample).r;
        cloest_depth_sample *= pointLightFar;
        factor += (curr_depth_sample - cloest_depth_sample - bias > 0.f ? 1.0 : 0.0);
        // factor = curr_depth_sample;
    }
    return (factor) / n_samples; // return shadow
}

vec3 dirLightDiffuse(vec3 fragPos, vec3 n)
{

    vec3 diffuse = vec3(0.0f);
    for (int i = 0; i < numDirLights; ++i)
    {

        vec3 l = normalize(dirLightPos[i]);
        float rr = dot(l, l);
        float shadowFactor = 1 - computeDirLightShadow(fragPos, n, dirLightSpaceMatrix[i], dirDepthMap[i]);

        if (i == 0) // 太阳光处理
        {
            diffuse += shadowFactor * dirLightIntensity[i] / rr * max(0.f, dot(n, l)) * sunlightDecay;
        }
        else
        {
            diffuse +=
                shadowFactor * dirLightIntensity[i] / rr * max(0.f, dot(n, l));
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

        vec3 l = normalize(dirLightPos[i]);
        float specularStrength = 0.01f;
        vec3 viewDir = normalize(eyePos - fragPos);
        vec3 reflectDir = reflect(-l, n);
        float shadowFactor = 1 - computeDirLightShadow(fragPos, n, dirLightSpaceMatrix[i], dirDepthMap[i]);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);

        if (i == 0) // 太阳光处理
        {
            specular += shadowFactor * specularStrength * spec * dirLightIntensity[i] * sunlightDecay * 160;
        }
        else
        {
            specular += shadowFactor * specularStrength * spec * dirLightIntensity[i] * 160;
        }
    }
    return specular;
}

vec3 pointLightDiffuse(vec3 fragPos, vec3 n)
{
    vec3 diffuse = vec3(0.f, 0.f, 0.f);

    for (int i = 0; i < numPointLights; ++i)
    {
        vec3 l = pointLightPos[i] - fragPos;
        float rr = pow(dot(l, l), 0.6) * 10;
        l = normalize(l);

        float shadow_factor = 1 - computePointLightShadow(fragPos, n, pointLightPos[i], shadowCubeMaps[i]);
        diffuse += pointLightIntensity[i] / rr * max(0.f, dot(n, l)) * shadow_factor;
    }
    return diffuse;
}

vec3 pointLightSpec(vec3 fragPos, vec3 n)
{
    vec3 specular = vec3(0.f, 0.f, 0.f);

    for (int i = 0; i < numPointLights; ++i)
    {
        vec3 l = pointLightPos[i] - fragPos;
        float rr = pow(dot(l, l), 0.6) * 10;
        l = normalize(l);
        float spec = 0.f;
        float specularStrength = 0.005f;

        float shadow_factor = 1 - computePointLightShadow(fragPos, n, pointLightPos[i], shadowCubeMaps[i]);

        vec3 viewDir = normalize(eyePos - fragPos);
        vec3 reflectDir = reflect(-l, n);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        specular += specularStrength * spec * pointLightIntensity[i] * shadow_factor;
    }
    return specular;
}

/*****************************天空大气计算********************************************************** */
const float PI = 3.1415926535;
const vec4 betaRayleigh = vec4(5.8e-6, 1.35e-5, 3.31e-5, 1.0f); // 散射率(波长/RGB)
uniform vec4 betaMie = vec4(21e-6, 21e-6, 21e-6, 1.0f);
vec3 camDir = vec3(0.f);
vec3 camPos = vec3(0.f);
vec3 sunDir = vec3(0.f);
vec3 earthCenter;
float itvl;
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

#include "SkyTexPass/geometry.glsl"
#include "SkyTexPass/scatter.glsl"

vec3 uncharted2_tonemap(vec3 color)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.20;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2; // white point
    return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
}

vec4 computeSkyColor()
{
    vec4 skyColor;
    vec4 scatterRayleigh = vec4(0.0f);
    vec4 scatterMie = vec4(0.0f);

    vec3 camSkyIntersection = intersectSky(camPos, camDir); // 摄像机视线与天空交点
    itvl = length(camSkyIntersection - camPos) / float(maxStep);
    for (int i = 0; i < maxStep; ++i)
    {
        if (camSkyIntersection == vec3(0.0f))
        {
            return vec4(0.000f); // 散射点阳光被地面阻挡
        }
        vec3 scatterPoint = camPos + i * itvl * camDir;
        vec3 scatterSkyIntersection = intersectSky(scatterPoint, sunDir);     // 散射点与天空交点
        vec3 scatterEarthIntersection = intersectEarth(scatterPoint, sunDir); // 散射点与地面交点
        if (scatterEarthIntersection != vec3(0.0f) && length(scatterEarthIntersection - scatterPoint) < length(scatterSkyIntersection - scatterPoint))
        {
            continue; // 散射点阳光被地面阻挡
        }

        vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, scatterPoint);                    // 摄像机到散射点的透射率
        vec4 t2 = getTransmittanceFromLUTSky(transmittanceLUT, earthRadius, earthRadius + skyHeight, scatterPoint, scatterSkyIntersection); // 散射点到天空边界的透射率
        scatterRayleigh += scatterCoefficientRayleigh(scatterPoint) * t1 * t2;

        scatterMie += scatterCoefficientMie(scatterPoint) * t1 * t2;
    }

    scatterRayleigh *= phaseRayleigh(camDir, sunDir);

    scatterMie *= phaseMie(camDir, sunDir);

    skyColor += scatterRayleigh;
    skyColor += scatterMie * MieIntensity;
    // skyColor.rgb = uncharted2_tonemap(skyColor.rgb);

    return vec4(dirLightIntensity[0], 1.0f) * skyColor * skyIntensity * itvl;
    // return vec4(dirLightIntensity[0], 1.0f) * skyColor * skyIntensity;
}

vec4 computeAerialPerspective(vec3 camEarthIntersection)
{
    vec4 aerialColor;
    vec4 scatterRayleigh = vec4(0.0f);
    vec4 scatterMie = vec4(0.0f);

    itvl = length(camEarthIntersection - camPos) / float(maxStep);
    for (int i = 0; i < maxStep; ++i)
    {
        vec3 scatterPoint = camPos + i * itvl * camDir;
        vec3 scatterSkyIntersection = intersectSky(scatterPoint, sunDir);     // 散射点与天空交点
        vec3 scatterEarthIntersection = intersectEarth(scatterPoint, sunDir); // 散射点与地面交点
        if (scatterEarthIntersection != vec3(0.0f) && length(scatterEarthIntersection - scatterPoint) < length(scatterSkyIntersection - scatterPoint))
        {
            continue; // 散射点阳光被地面阻挡
        }
        vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, scatterPoint);                    // 摄像机到散射点的透射率
        vec4 t2 = getTransmittanceFromLUTSky(transmittanceLUT, earthRadius, earthRadius + skyHeight, scatterPoint, scatterSkyIntersection); // 散射点到天空边界的透射率

        scatterRayleigh += scatterCoefficientRayleigh(scatterPoint) * t1 * t2;

        scatterMie += scatterCoefficientMie(scatterPoint) * t1 * t2;
    }

    scatterRayleigh *= phaseRayleigh(camDir, sunDir);

    scatterMie *= phaseMie(camDir, sunDir);

    aerialColor += scatterRayleigh;
    aerialColor += scatterMie * MieIntensity;
    // aerialColor.rgb = uncharted2_tonemap(aerialColor.rgb);

    // return vec4(dirLightIntensity[0], 1.0f) * aerialColor * 2e3;
    return vec4(dirLightIntensity[0], 1.0f) * aerialColor * itvl;
}

vec3 computeSunlightDecay(vec3 camPos, vec3 fragDir, vec3 sunDir)
{
    vec3 skyIntersection = intersectSky(camPos, sunDir);
    // vec4 t1 = transmittance(camPos, skyIntersection, 1.0f); // 散射点到摄像机的透射率   决定天顶-地平线透射率差异
    vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, skyIntersection);

    return t1.rgb;
}

vec3 generateSunDisk(vec3 camPos, vec3 fragDir, vec3 sunDir, vec3 sunIntensity, float sunSize)
{

    // 计算太阳方向和片段方向之间的余弦值
    float exponent = 1e3; // 锐利程度
    float sunSizeInner = 1.f - 1e-4;
    float sunSizeOuter = 1.f - 1e-3;

    float sunDot = dot(normalize(fragDir), normalize(sunDir));
    float sunSmoothstep = smoothstep(sunSizeOuter, sunSizeInner, sunDot);

    // 返回太阳亮度，与透射率相乘
    return sunIntensity * 1e2 * pow(sunSmoothstep, exponent) * sunlightDecay;
}

void initialize()
{

    LightResult = vec4(0.f, 0.f, 0.f, 1.f); // Initialize LightResult

    camDir = fragViewSpaceDir(TexCoord);
    camPos = eyePos;
    earthCenter = vec3(0.0f, -earthRadius, 0.0f); // 地球球心，位于地面原点正下方
    sunDir = dirLightPos[0];
    sunlightDecay = computeSunlightDecay(camPos, camDir, dirLightPos[0]);
}
void main()
{
    // Diffuse Caculation

    vec3 FragPos = texture(gPosition, TexCoord).rgb;
    vec3 n = normalize(texture(gNormal, TexCoord).rgb);
    vec3 diffuse;
    vec3 specular;
    vec3 ambient;

    initialize();

    // 天空计算
    if (length(FragPos) == 0)
    {
        vec3 camEarthIntersection = intersectEarth(camPos, camDir);
        if (Skybox == 1)
        {
            if (camEarthIntersection == vec3(0.0f))
            {
                LightResult = vec4(sampleSkybox(TexCoord, skybox), 1.0f); // 采样天空盒
            }
            else
            {
                // 击中地球,渲染大气透视
                LightResult = computeAerialPerspective(camEarthIntersection);

                vec4 t1 = transmittance(camPos, camEarthIntersection, 1.0f);
                // vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, camEarthIntersection);// TODO: 修复近地面错误

                // 渲染地面
                vec3 normal = normalize(camEarthIntersection - earthCenter);
                vec3 lighting = dirLightDiffuse(camEarthIntersection, normal);
                vec3 earthBaseColor = vec3(0.3, 0.3f, 0.34f); // 地面颜色
                LightResult.rgb += lighting * earthBaseColor * t1.rgb;
            }
        }
        else
        {
            float camHeight = length(camPos - earthCenter) - earthRadius;

            if (camEarthIntersection != vec3(0.0f))
            {

                // 击中地球,渲染大气透视
                LightResult = computeAerialPerspective(camEarthIntersection);

                vec4 t1 = transmittance(camPos, camEarthIntersection, 1.0f);
                // vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, camEarthIntersection);

                // 渲染地面
                vec3 normal = normalize(camEarthIntersection - earthCenter);
                vec3 lighting = dirLightDiffuse(camEarthIntersection, normal);
                vec3 earthBaseColor = vec3(0.3, 0.3f, 0.34f); // 地面颜色
                LightResult.rgb += lighting * earthBaseColor * t1.rgb;
            }
            else
            {
                if (camHeight > skyHeight)
                {
                    // 摄像机在大气层外
                }
                else
                {
                    LightResult += computeSkyColor();
                }
            }
        }
        if (camEarthIntersection == vec3(0.0f))
        {
            LightResult.rgb += generateSunDisk(camPos, camDir, sunDir, dirLightIntensity[0], 2.0f);
        }
    }
    else
    {
        diffuse =
            pointLightDiffuse(FragPos, n) +
            dirLightDiffuse(FragPos, n);

        specular =
            pointLightSpec(FragPos, n) +
            dirLightSpec(FragPos, n);

        vec3 ambient = ambientLight;
        if (Skybox == 1)
        {
            ambient += computeSkyboxAmbientMipMap(skybox, n);
            // ambient = vec3(0.f);
        }
        // ambient = (ambientLight +
        //            computeSkyboxAmbient(skybox));

        LightResult += vec4(diffuse * texture(gAlbedoSpec, TexCoord).rgb, 1.f);
        LightResult += vec4(specular * texture(gAlbedoSpec, TexCoord).a, 1.f);
        LightResult += vec4(ambient * texture(gAlbedoSpec, TexCoord).rgb, 1.f);

        if (Skybox == 0) // 关闭天空盒
        {
            vec4 t1 = transmittance(camPos, FragPos, 1.0f);
            LightResult *= t1;
            LightResult += computeAerialPerspective(FragPos);
        }
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
            LightVolueBoxMin + pointLightPos[i],
            LightVolueBoxMax + pointLightPos[i],
            vec4(pow(pointLightIntensity[i] / 8.f, vec3(1.2f)), 1.0f));
    }
}
