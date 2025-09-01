
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
const int MAX_LIGHTS = 10;  // Maximum number of lights supported
uniform int numPointLights; // actual number of lights used
uniform samplerCube shadowCubeMaps[MAX_LIGHTS];
uniform vec3 pointLightPos[MAX_LIGHTS];
uniform vec3 pointLightIntensity[MAX_LIGHTS];
uniform float pointLightFar;

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

#define NO_INTERSECTION vec3(1.0f / 0.0f)
const float PI = 3.1415926535;
vec3 earthCenter;

vec3 intersectSky(vec3 ori, vec3 dir) {
    float kAtmosphereRadius = earthRadius + skyHeight;
    float kEarthRadius = earthRadius;

    vec3 relativeOrigin = ori - earthCenter;

    // 计算二次方程的系数 A, B, C
    float a = dot(dir, dir);
    float b = 2.0f * dot(relativeOrigin, dir);
    float cS = dot(relativeOrigin, relativeOrigin) - kAtmosphereRadius * kAtmosphereRadius;
    float cE = dot(relativeOrigin, relativeOrigin) - kEarthRadius * kEarthRadius;

    // --- 检查光线与大气层的交点 ---
    float discrS = b * b - 4.0f * a * cS;
    if (discrS < 0.0f) {
        // 判别式小于0，光线没有与大气层相交
        return NO_INTERSECTION;
    }

    // 计算大气层的两个交点参数 t
    float tS1 = (-b - sqrt(discrS)) / (2.0f * a);
    float tS2 = (-b + sqrt(discrS)) / (2.0f * a);

    // --- 确定进入大气的第一个交点 tS ---
    float tS;
    if (tS1 > 0.0f) {
        tS = tS1;
    }
    else if (tS2 > 0.0f) {
        tS = tS2;
    }
    else {
        // 两个交点都在光线起点之后（tS < 0），光线朝远离大气的方向传播
        // return vec3(0.0f);
    }

    // 返回进入大气层的交点坐标
    return ori + dir * tS;
}
vec3 intersectEarth(vec3 ori, vec3 dir) {
    float kEarthRadius = earthRadius;

    vec3 relativeOrigin = ori - earthCenter;

    // 计算二次方程的系数 A, B, C
    float a = dot(dir, dir);
    float b = 2.0f * dot(relativeOrigin, dir);
    float c = dot(relativeOrigin, relativeOrigin) - kEarthRadius * kEarthRadius;

    float discr = b * b - 4.0f * a * c;

    // 如果判别式小于0，说明没有交点
    if (discr < 0.0f) {
        // 返回一个特殊值来表示没有交点，例如一个“无效”向量
        return vec3(1.f / 0.0f);
    }

    // 计算两个交点参数t
    float t1 = (-b - sqrt(discr)) / (2.0f * a);
    float t2 = (-b + sqrt(discr)) / (2.0f * a);

    // 通常我们只关心“最近”的那个交点（t值最小且大于0）
    float t_intersect;

    if (t1 > 0.0f) {
        t_intersect = t1;
    }
    else if (t2 > 0.0f) {
        t_intersect = t2;
    }
    else {
        // 两个t值都小于等于0，表示光线从球体内部射出，或球体在光线背后
        // 此时可以认为没有有效交点
        return vec3(1.f / 0.0f); // 同样，返回一个特殊值
    }

    // 根据找到的有效t值计算交点坐标
    vec3 intersectionPoint = ori + t_intersect * dir;
    return intersectionPoint;
}

float heightToGround(vec3 p) {
    return (length(p - earthCenter) - earthRadius);
    // return p.y;
}

/// @brief 透射率lut uv映射
/// @param earthRadius 地面半径
/// @param skyRadius 天空半径
/// @param mu 天顶角余弦值
/// @param r 起始点半径
/// @return uv in LUT
vec2 transmittanceUVMapping(float earthRadius, float skyRadius, float mu, float r) {
    float H = sqrt(skyRadius * skyRadius - earthRadius * earthRadius); // 天顶与地面切线长度.
    float rho = sqrt(r * r - earthRadius * earthRadius);               // 起始点与地面相切的长度

    float discriminant = r * r * (mu * mu - 1.0f) + skyRadius * skyRadius;
    float d = max(0.0f, (-r * mu + sqrt(discriminant)));

    float d_min = skyRadius - r;
    float d_max = rho + H; // 起始点为r时,地平线路径长度最大值

    float x_mu = (d - d_min) / (d_max - d_min);
    float x_r = rho / H;

    return vec2(x_mu, x_r);
}

/// @brief 透射率lut uv反向映射
/// @param earthRadius 地面半径
/// @param skyRadius 天空半径
/// @param uv uv in LUT
/// @return mu 和 r
vec2 transmittanceUVInverseMapping(float earthRadius, float skyRadius, vec2 uv) {
    float x_mu = uv.x;
    float x_r = uv.y;

    float H = sqrt(skyRadius * skyRadius - earthRadius * earthRadius);
    float rho = x_r * H;
    float r = sqrt(rho * rho + earthRadius * earthRadius);

    float d_min = skyRadius - r;
    float d_max = rho + H;
    float d = x_mu * (d_max - d_min) + d_min;

    float mu = 0.0f;
    if (2.0f * d * r > 0.0f) {
        mu = (skyRadius * skyRadius - r * r - d * d) / (2.0f * d * r);
    }

    return vec2(mu, r);
}
vec2 rayToMuR(float earthRadius, float skyRadius, vec3 ori, vec3 end) {
    float R = length(ori - earthCenter);

    vec3 n = normalize(ori - earthCenter);

    vec3 dir = normalize(end - ori);

    float mu = dot(dir, n);
    return vec2(mu, R);
}

void MuRToRay(vec2 MuR, float earthRadius, out vec3 ori, out vec3 dir) {
    ori = vec3(0.0, MuR.y - earthRadius, 0.0);

    float dir_x = sqrt(max(0.0, 1.0 - MuR.x * MuR.x));
    float dir_y = MuR.x;
    float dir_z = 0.0;

    dir = vec3(dir_x, dir_y, dir_z);
}

// 从LUT获取两点透射率
vec4 getTransmittanceFromLUT(sampler2D LUT, float earthRadius, float skyRadius, vec3 ori, vec3 end) {
    vec3 dir = normalize(ori - end);
    vec3 n = normalize(ori - earthCenter);
    vec3 earthIntersection = intersectEarth(ori, -dir);
    int hitEarth = 0;

    if (earthIntersection != NO_INTERSECTION) {
        hitEarth = 1;
    }
    vec2 MuR_ori;
    vec2 MuR_end;
    if (hitEarth == 1) {
        MuR_ori = rayToMuR(earthRadius, skyRadius, ori, 2 * ori - end);
        MuR_end = rayToMuR(earthRadius, skyRadius, end, ori);
        float bias = 0.3f; // 加上这个magic bias 近地面透射率的artifcat减少了
        MuR_end.y += bias;
    }
    else {
        MuR_ori = rayToMuR(earthRadius, skyRadius, ori, end); // 高空地平线走样严重
        MuR_end = rayToMuR(earthRadius, skyRadius, end, 2 * end - ori);
    }
    vec4 tori = texture(LUT, transmittanceUVMapping(earthRadius, skyRadius, MuR_ori.x, MuR_ori.y));
    vec4 tend = texture(LUT, transmittanceUVMapping(earthRadius, skyRadius, MuR_end.x, MuR_end.y));

    if (hitEarth == 1) {
        return tend / tori;
    }
    return tori / tend;
}
// 从LUT获取起点到天空透射率
vec4 getTransmittanceFromLUTSky(sampler2D LUT, float earthRadius, float skyRadius, vec3 ori, vec3 skyIntersection) {
    vec3 dir = normalize(ori - skyIntersection);
    vec3 n = normalize(ori - earthCenter);

    vec2 MuR_ori = rayToMuR(earthRadius, skyRadius, ori, skyIntersection);
    vec4 tori = texture(LUT, transmittanceUVMapping(earthRadius, skyRadius, MuR_ori.x, MuR_ori.y));

    return tori;
}
const vec4 betaRayleigh = vec4(5.8e-6, 1.35e-5, 3.31e-5, 1.0f); // 散射率(波长/RGB)
float phaseRayleigh(vec3 _camRayDir, vec3 _sunDir) {
    float cosine = dot(_camRayDir, _sunDir) / length(_camRayDir) / length(_sunDir);
    return 3.0f / 16.0f * PI * (1 + cosine * cosine);
}
float phaseMie(vec3 _camRayDir, vec3 _sunDir) {
    float gMie2 = gMie * gMie;
    float cosine = dot(_camRayDir, _sunDir) / length(_camRayDir) / length(_sunDir);
    return (1.0 - gMie2) / pow(1.0 + gMie2 - 2.0 * gMie * cosine, 1.5);

    // return 3.0f / 8.0f * PI * (1.0 - gMie2) * (1 + cosine * cosine) / (2 + gMie2) / pow((1 + gMie2 - 2.0 * gMie * cosine), 1.5f);
}
float rhoRayleigh(float h) {
    if (h < 0.0f) {
        h = 0.0f;
    }
    return atmosphereDensity * exp(-abs(h) / HRayleigh); // 大气密度近似
}
float rhoMie(float h) {
    if (h < 0.0f) {
        h = 0.0f;
    }
    return MieDensity * exp(-abs(h) / HMie);
}
float rhoOzone(float h) {
    const float ozoneCenterHeight = 2.5e4;
    const float ozoneWidth = 2.0e4;
    if (h < 0.0f) {
        h = 0.0f;
    }
    return max(0.0f, 1.0f - abs(h - ozoneCenterHeight) / ozoneWidth);
}

vec4 scatterCoefficientRayleigh(vec3 p) {
    // vec3 intersection = intersectSky(camPos, camRayDir);

    float h = (heightToGround(p)); // 散射点高度
    return betaRayleigh * rhoRayleigh(h);
}
vec4 scatterCoefficientMie(vec3 p) {
    // vec3 intersection = intersectSky(camPos, camRayDir);

    float h = (heightToGround(p)); // 散射点高度
    return betaMie * rhoMie(h);
}

vec4 transmittance(vec3 ori, vec3 end, float scale) {
    vec4 t; // 透射率
    // const vec4 betaMieAbsorb = vec4(2.5e-5, 4e-5, 1e-5, 1.0f); // Hacking 让地平线呈现微妙紫色
    const vec4 betaMieAbsorb = vec4(2.5e-5, 4e-5, 1e-5, 1.0f); // Hacking 让地平线呈现微妙紫色
    const vec4 betaOzoneAbsorb = vec4(0.650f, 1.881f, 0.085f, 1.0f) * 1e-6f;
    const float tMaxStep = 128;

    float dist = length(end - ori);
    float tItvl = dist / float(tMaxStep);

    // 光学深度积分
    float opticalDepthMie = 0.f;
    float opticalDepthRayleigh = 0.f;
    float opticalDepthOzone = 0.f;
    for (int i = 0; i < tMaxStep; ++i) {
        vec3 p = ori + i * (end - ori) / tMaxStep;
        float h = heightToGround(p);
        opticalDepthMie += tItvl * rhoMie(h) * scale;
        opticalDepthRayleigh += tItvl * rhoRayleigh(h) * scale;
        opticalDepthOzone += tItvl * rhoOzone(h) * scale;
    }

    // 总透射率计算
    vec4 extictionMie = (betaMie + betaMieAbsorb * absorbMie) * opticalDepthMie;
    vec4 extictionOzone = betaOzoneAbsorb * opticalDepthOzone;
    vec4 extictionRayleigh = betaRayleigh * opticalDepthRayleigh;
    t = vec4(exp(-(extictionOzone + extictionMie + extictionRayleigh).r),
        exp(-(extictionOzone + extictionMie + extictionRayleigh).g),
        exp(-(extictionOzone + extictionMie + extictionRayleigh).b),
        1.0f);
    // return clamp(t, 0.0001f, 0.9999f);
    return t;
}
vec3 camDir = vec3(0.f);
vec3 camPos = vec3(0.f);
vec3 sunDir = vec3(0.f);
float itvl;

vec4 computeSkyColor(in vec3 sunLightIntensity) {
    vec4 skyColor;
    vec4 scatterRayleigh = vec4(0.0f);
    vec4 scatterMie = vec4(0.0f);

    vec3 camSkyIntersection = intersectSky(camPos, camDir); // 摄像机视线与天空交点
    itvl = length(camSkyIntersection - camPos) / float(maxStep);
    for (int i = 0; i < maxStep; ++i) {
        if (camSkyIntersection == NO_INTERSECTION) {
            return vec4(0.000f); // 散射点阳光被地面阻挡
        }
        vec3 scatterPoint = camPos + i * itvl * camDir;
        vec3 scatterSkyIntersection = intersectSky(scatterPoint, sunDir);     // 散射点与天空交点
        vec3 scatterEarthIntersection = intersectEarth(scatterPoint, sunDir); // 散射点与地面交点
        if (scatterEarthIntersection != NO_INTERSECTION && length(scatterEarthIntersection - scatterPoint) < length(scatterSkyIntersection - scatterPoint)) {
            continue; // 散射点阳光被地面阻挡
        }
        // vec4 t1 = transmittance(camPos, scatterPoint, 1.0f);                 // 摄像机到散射点的透射率
        // vec4 t2 = transmittance(scatterPoint, scatterSkyIntersection, 1.0f); // 散射点到天空边界的透射率

        vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, scatterPoint);                    // 摄像机到散射点的透射率
        vec4 t2 = getTransmittanceFromLUTSky(transmittanceLUT, earthRadius, earthRadius + skyHeight, scatterPoint, scatterSkyIntersection); // 散射点到天空边界的透射率
        scatterRayleigh += scatterCoefficientRayleigh(scatterPoint) * t1 * t2;

        scatterMie += scatterCoefficientMie(scatterPoint) * t1 * t2;
    }

    scatterRayleigh *= phaseRayleigh(camDir, sunDir);

    scatterMie *= phaseMie(camDir, sunDir);

    skyColor += scatterRayleigh;
    skyColor += scatterMie * MieIntensity;

    return vec4(sunLightIntensity, 1.0f) * skyColor * skyIntensity * itvl;
    // return vec4(sunLightIntensity, 1.0f) * skyColor * skyIntensity;
}

vec4 computeAerialPerspective(vec3 camEarthIntersection, in vec3 sunLightIntensity) {
    vec4 aerialColor;
    vec4 scatterRayleigh = vec4(0.0f);
    vec4 scatterMie = vec4(0.0f);
    vec4 t1 = vec4(1.0f);
    itvl = length(camEarthIntersection - camPos) / float(maxStep);
    for (int i = 0; i < maxStep; ++i) {
        vec3 scatterPoint = camPos + i * itvl * camDir;
        vec3 scatterSkyIntersection = intersectSky(scatterPoint, sunDir);     // 散射点与天空交点
        vec3 scatterEarthIntersection = intersectEarth(scatterPoint, sunDir); // 散射点与地面交点
        if (scatterEarthIntersection != NO_INTERSECTION && length(scatterEarthIntersection - scatterPoint) < length(scatterSkyIntersection - scatterPoint)) {
            continue; // 散射点阳光被地面阻挡
        }
        vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, scatterPoint);                    // 摄像机到散射点的透射率
        vec4 t2 = getTransmittanceFromLUTSky(transmittanceLUT, earthRadius, earthRadius + skyHeight, scatterPoint, scatterSkyIntersection); // 散射点到天空边界的透射率

        // vec4 t1 = transmittance(camPos, scatterPoint, 1.0f);                 // 摄像机到散射点的透射率
        // vec4 t2 = transmittance(scatterPoint, scatterSkyIntersection, 1.0f); // 散射点到天空边界的透射率

        scatterRayleigh += scatterCoefficientRayleigh(scatterPoint) * t1 * t2;

        scatterMie += scatterCoefficientMie(scatterPoint) * t1 * t2;
    }

    scatterRayleigh *= phaseRayleigh(camDir, sunDir);

    scatterMie *= phaseMie(camDir, sunDir);

    aerialColor += scatterRayleigh;
    aerialColor += scatterMie * MieIntensity;

    return vec4(sunLightIntensity, 1.0f) * aerialColor * itvl;
}

vec3 computeSunlightDecay(vec3 camPos, vec3 fragDir, vec3 sunDir) {
    vec3 skyIntersection = intersectSky(camPos, sunDir);
    // vec4 t1 = transmittance(camPos, skyIntersection, 1.0f); // 散射点到摄像机的透射率   决定天顶-地平线透射率差异
    vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, skyIntersection);

    return t1.rgb;
}

vec3 generateSunDisk(vec3 camPos, vec3 fragDir, vec3 sunDir, vec3 sunIntensity, float sunSize) {
    // 计算太阳方向和片段方向之间的余弦值
    float exponent = 1e3; // 锐利程度
    float sunSizeInner = 1.f - 1e-4;
    float sunSizeOuter = 1.f - 1e-3;

    float sunDot = dot(normalize(fragDir), normalize(sunDir));
    float sunSmoothstep = smoothstep(sunSizeOuter, sunSizeInner, sunDot);

    // 返回太阳亮度，与透射率相乘
    return sunIntensity * 1e2 * pow(sunSmoothstep, exponent) * sunlightDecay;
}

bool isPointInBox(vec3 testPoint, vec3 boxMin, vec3 boxMax) {
    return testPoint.x < boxMax.x && testPoint.x > boxMin.x &&
    testPoint.z < boxMax.z && testPoint.z > boxMin.z &&
    testPoint.y < boxMax.y && testPoint.y > boxMin.y;
}
vec2 rayBoxDst(vec3 boundsMin, vec3 boundsMax, vec3 rayOrigin, vec3 invRaydir) {
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

// vec4 lightMarching(
//     vec3 startPoint,
//     vec3 direction,
//     float depthLimit,
//     vec3 boxMin,
//     vec3 boxMax) {
//     vec3 testPoint = startPoint;

//     float interval = 0.1; // 每次步进间隔
//     float intensity = 0.01 * interval;

//     int hit = 0;
//     float sum = 0.0;
//     for (int i = 0; i < 8; ++i) {
//         // 步进总长度
//         if (i * interval > depthLimit) {
//             break;
//         }
//         testPoint += direction * interval;

//         // 光线在体积内行进
//         if (isPointInBox(testPoint, boxMin, boxMax)) {
//             hit = 1;

//             sum += intensity / depthLimit;
//         }
//     }
//     return -sum * vec4(dirLightIntensity[0] / length(dirLightIntensity[0]), 1.0f) / 1;
// }

// vec4 computeCloudRayMarching(
//     vec3 startPoint,
//     vec3 direction,
//     float depthLimit,
//     vec3 boxMin,
//     vec3 boxMax,
//     vec4 color) {
//     vec3 testPoint = startPoint;

//     float interval = 0.01; // 每次步进间隔
//     float intensity = 0.1 * interval;

//     float density = 0.2;

//     int hit = 0;
//     float sum = 0.0;
//     for (int i = 0; i < 256; ++i) {
//         // 步进总长度
//         if (i * interval > depthLimit) {
//             break;
//         }
//         testPoint += direction * interval;

//         // 光线在体积内行进
//         if (isPointInBox(testPoint, boxMin, boxMax)) {
//             hit = 1;
//             sum += intensity;
//             sum *= exp(density * interval);

//             // vec3 lightDir = pointLightPos[0]-testPoint;
//             vec3 lightDir = normalize(dirLightPos[0]);
//             float limit = rayBoxDst(boxMin, boxMax, testPoint, 1 / lightDir).y;

//             color += lightMarching(testPoint, lightDir, limit, boxMin, boxMax);
//         }
//     }
//     if (hit == 0) {
//         return vec4(0.0f);
//     }

//     return sum * color;
// }
// vec4 cloudRayMarching(
//     vec3 startPoint,
//     vec3 direction,
//     vec3 fragPos,
//     vec3 boxMin,
//     vec3 boxMax,
//     vec4 color) {
//     vec2 rst = rayBoxDst(boxMin, boxMax, startPoint, 1.0 / direction);
//     float boxDepth = rst.x;
//     float boxInsideDepth = rst.y;

//     float fragDepth = length(fragPos - startPoint);
//     // skybox
//     if (length(fragPos) == 0) {
//         fragDepth = 10000000.f;
//     }

//     if (fragDepth < boxDepth) {
//         // 物体阻挡Box
//         return vec4(0.f);
//     }
//     float depthLimit;
//     if ((fragDepth - boxDepth) < boxInsideDepth) {
//         // 物体嵌入box,步进深度限制减小
//         depthLimit = fragDepth - boxDepth;
//     }
//     else {
//         depthLimit = boxInsideDepth;
//     }
//     return computeCloudRayMarching(startPoint + direction * rst.x, direction, depthLimit, boxMin, boxMax, color);
// }
/*********************************LightVolume******************************************************/
vec4 computeLightVolumeRayMarching(
    vec3 startPoint,
    vec3 direction,
    float depthLimit,
    vec3 boxMin,
    vec3 boxMax,
    vec4 color) {
    vec3 testPoint = startPoint;
    float sum = 0.0;
    float interval = 0.01; // 每次步进间隔
    float intensity = 1.5 * interval;
    float density = 0.1f;
    for (int i = 0; i < 1024; ++i) {
        // 步进总长度
        if (i * interval > depthLimit) {
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
    vec4 color) {
    vec2 rst = rayBoxDst(boxMin, boxMax, startPoint, 1.0 / direction);
    float boxDepth = rst.x;
    float boxInsideDepth = rst.y;

    float fragDepth = length(fragPos - startPoint);
    // skybox
    if (length(fragPos) == 0) {
        fragDepth = 10000000.f;
    }

    if (fragDepth < boxDepth) {
        // 物体阻挡Box
        return vec4(0.f);
    }
    float depthLimit;
    if ((fragDepth - boxDepth) < boxInsideDepth) {
        // 物体嵌入box,步进深度限制减小
        depthLimit = fragDepth - boxDepth;
    }
    else {
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
    vec4 color) {
    return lightVolumeRayMarching(startPoint, direction, fragPos, boxMin, boxMax, color);
}
/*******************************Lighting******************************************************/
float computeDirLightShadow(vec3 fragPos, vec3 fragNormal, in DirLight dirLight) {
    // perform perspective divide
    if (DirShadow == 0) {
        return 0.f;
    }
    vec4 lightSpaceFragPos = dirLight.spaceMatrix * vec4(fragPos, 1.0f);

    // // 计算遮挡物与接受物的平均距离
    // float d = 0.f;
    // int occlusion_times = 0;
    // for (int k = 0; k < n_samples; ++k)
    // {
    //     vec4 sampleOffset = dirLight.spaceMatrix * vec4(TBN * shadowSamples[k] * blurRadius, 0.0f);
    //     vec4 fragPosLightSpace = lightSpaceFragPos + sampleOffset; // Dir Light View Space
    //     vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
    //     // transform to [0,1] range
    //     projCoords = projCoords * 0.5 + 0.5;
    //     // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    //     float closestDepth = texture(dirLight.depthMap, projCoords.xy).r;
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
    for (int j = 0; j < n_samples; ++j) {
        // vec4 sampleOffset = dirLight.spaceMatrix * vec4(
        //                                                TBN *
        //                                                    shadowSamples[j] *
        //                                                    blurRadius * 20 * pow(d, 2),
        //                                                0.0f);
        vec4 sampleOffset = dirLight.spaceMatrix * vec4(
            TBN *
            shadowSamples[j] *
            blurRadius * 5.f,
            0.0f);
        vec4 fragPosLightSpace = lightSpaceFragPos + sampleOffset; // Dir Light View Space
        vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(dirLight.depthMap, projCoords.xy).r;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        float cosine = dot(fragNormal, dirLight.pos) / length(fragNormal) / length(dirLight.pos);
        // float texel = 4e-5; // texel = (ortho_scale,farPlane)
        float texel = dirLight.orthoScale / dirLight.farplane;

        float bias = sqrt(1 - cosine * cosine) / cosine * texel;
        // check whether current frag pos is in shadow
        factor += currentDepth - closestDepth - bias > 0 ? 1.0f : 0.0f;
    }

    return factor / n_samples;
    // return dirLight.orthoScale * 1e-1 + 0.f;
}

float computePointLightShadow(vec3 fragPos, vec3 fragNorm, vec3 pointLightPos, samplerCube _depthMap) {
    if (PointShadow == 0) {
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
    for (int k = 0; k < n_samples; ++k) {
        vec3 sampleOffset = TBN * shadowSamples[k] * 4.f;
        vec3 dir_sample = sampleOffset + fragPos - pointLightPos;
        float curr_depth_sample = length(dir_sample);
        float cloest_depth_sample = texture(_depthMap, dir_sample).r;

        cloest_depth_sample *= pointLightFar;
        if (curr_depth_sample - cloest_depth_sample - bias > 0.01f) {
            occlusion_times++;
            d += curr_depth_sample - cloest_depth_sample;
        }
    }
    d = d / occlusion_times;
    float factor = 0.f;
    for (int j = 0; j < n_samples; ++j) {
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
    vec3 ambient = textureLod(_skybox, dir, 6).rgb;

    return ambient;
}

vec3 dirLightDiffuse(vec3 fragPos, vec3 n)
{

    vec3 diffuse = vec3(0.0f);
    for (int i = 0; i < numDirLights; ++i)
    {

        vec3 l = normalize(dirLightArray[i].pos);
        float rr = dot(l, l);
        float shadowFactor = 1 - computeDirLightShadow(fragPos, n, dirLightArray[i]);

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
        float shadowFactor = 1 - computeDirLightShadow(fragPos, n, dirLightArray[i]);
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

void initializeAtmosphereParameters()
{

    camDir = fragViewSpaceDir(TexCoord);
    camPos = eyePos;
    earthCenter = vec3(0.0f, -earthRadius, 0.0f); // 地球球心，位于地面原点正下方
    sunDir = dirLightArray[0].pos;
    sunlightDecay = computeSunlightDecay(camPos, camDir, dirLightArray[0].pos);
}

void computeSky(inout vec4 LightResult, inout vec3 ambient, in vec3 n)
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
            LightResult = computeAerialPerspective(camEarthIntersection, dirLightArray[0].intensity);

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
            LightResult = computeAerialPerspective(camEarthIntersection, dirLightArray[0].intensity);

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
                LightResult += computeSkyColor(dirLightArray[0].intensity);
            }
        }
    }
    if (camEarthIntersection == NO_INTERSECTION)
    {
        LightResult.rgb += generateSunDisk(camPos, camDir, sunDir, dirLightArray[0].intensity, 2.0f);
    }
}

void computeSceneAtmosphere(in vec3 FragPos)
{
    if (Skybox == 0) // 关闭天空盒
    {
        // vec4 t1 = transmittance(camPos, FragPos, 1.0f);
    }
    vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, FragPos);
    LightResult *= t1;
    LightResult += computeAerialPerspective(FragPos, dirLightArray[0].intensity);
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
        computeSky(LightResult, ambient, n);
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
            // ambient = (ambientLight +
            //            computeSkyboxAmbient(skybox));
        }

        LightResult += vec4(diffuse * texture(gAlbedoSpec, TexCoord).rgb, 1.f);
        LightResult += vec4(specular * texture(gAlbedoSpec, TexCoord).a, 1.f);
        LightResult += vec4(ambient * texture(gAlbedoSpec, TexCoord).rgb, 1.f);

        computeSceneAtmosphere(FragPos);
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
