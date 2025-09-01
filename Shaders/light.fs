
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
#include "Raymarching/raymarching.glsl"
vec3 fragViewSpaceDir(vec2 uv)
{
    vec3 dir = vec3(uv - vec2(0.5f, 0.5f), 0.f);
    dir.y = dir.y * tan(radians(fov / 2)) * 2;
    dir.x = dir.x * tan(radians(fov / 2)) * (float(width) / float(height)) * 2;
    dir.z = -1.0f;
    dir = normalize(inverse(mat3(view)) * dir);
    return dir;
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
        float texel = 4e-5;                                                                           // texel = (ortho_scale,farPlane)
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

#include "SkyTexPass/geometry.glsl"
#include "SkyTexPass/scatter.glsl"
#include "SkyTexPass/skyAtmosphere.glsl"

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
            if (camEarthIntersection == NO_INTERSECTION)
            {
                LightResult = vec4(sampleSkybox(TexCoord, skybox), 1.0f); // 采样天空盒
            }
            else
            {
                // 击中地球,渲染大气透视
                LightResult = computeAerialPerspective(camEarthIntersection, dirLightIntensity[0]);

                vec4 t1 = transmittance(camPos, camEarthIntersection, 1.0f); // 走样
                                                                             // vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, camEarthIntersection); // TODO: 修复近地面错误
                if (Skybox == 1)
                {
                    ambient += computeSkyboxAmbientMipMap(skybox, n);
                    // ambient = vec3(0.f);
                }
                // 渲染地面
                vec3 normal = normalize(camEarthIntersection - earthCenter);
                vec3 lighting = dirLightDiffuse(camEarthIntersection, normal) + ambient;
                vec3 earthBaseColor = vec3(0.3, 0.3f, 0.34f); // 地面颜色
                LightResult.rgb += lighting * earthBaseColor * t1.rgb;
            }
        }
        else
        {
            float camHeight = length(camPos - earthCenter) - earthRadius;

            if (camEarthIntersection != NO_INTERSECTION)
            {

                // 击中地球,渲染大气透视
                LightResult = computeAerialPerspective(camEarthIntersection, dirLightIntensity[0]);

                vec4 t1 = transmittance(camPos, camEarthIntersection, 1.0f); // 走样
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
                    LightResult += computeSkyColor(dirLightIntensity[0]);
                }
            }
        }
        if (camEarthIntersection == NO_INTERSECTION)
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
            // vec4 t1 = transmittance(camPos, FragPos, 1.0f);
        }
        vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, FragPos);
        LightResult *= t1;
        LightResult += computeAerialPerspective(FragPos, dirLightIntensity[0]);
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
