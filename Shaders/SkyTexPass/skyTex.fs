

#version 330 core
out vec4 SkyResult;
in vec3 Dir;
in vec4 FragPos;

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
uniform vec4 betaMie = vec4(21e-6, 21e-6, 21e-6, 1.0f);

// 视口大小
uniform int width;
uniform int height;

// 太阳光设置
vec3 sunlightDecay;
uniform vec3 dirLightPos;
uniform vec3 dirLightIntensity;

/*****************Camera设置******************************************************************/
uniform float nearPlane;
uniform float farPlane;
uniform vec3 eyePos;
uniform vec3 eyeFront;
uniform vec3 eyeUp;
uniform float fov;
uniform mat4 view; // 视图矩阵

/*****************************天空大气计算********************************************************** */
const float PI = 3.1415926535;
const vec4 betaRayleigh = vec4(5.8e-6, 1.35e-5, 3.31e-5, 1.0f); // 散射率(波长/RGB)
vec3 camRayDir = vec3(0.f);
vec3 camPos = vec3(0.f);
vec3 sunDir = vec3(0.f);
vec3 earthCenter;
float itvl;

vec3 intersectSky(vec3 ori, vec3 dir)
{
    // 假设这些常量已经在全局或函数外定义
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
    if (discrS < 0.0f)
    {
        // 判别式小于0，光线没有与大气层相交
        return vec3(0.0f);
    }

    // 计算大气层的两个交点参数 t
    float tS1 = (-b - sqrt(discrS)) / (2.0f * a);
    float tS2 = (-b + sqrt(discrS)) / (2.0f * a);

    // --- 确定进入大气的第一个交点 tS ---
    float tS;
    if (tS1 > 0.0f)
    {
        tS = tS1;
    }
    else if (tS2 > 0.0f)
    {
        tS = tS2;
    }
    else
    {
        // 两个交点都在光线起点之后（tS < 0），光线朝远离大气的方向传播
        // return vec3(0.0f);
    }

    // 返回进入大气层的交点坐标
    return ori + dir * tS;
}
vec3 intersectEarth(vec3 ori, vec3 dir)
{
    float kEarthRadius = earthRadius;

    vec3 relativeOrigin = ori - earthCenter;

    // 计算二次方程的系数 A, B, C
    float a = dot(dir, dir);
    float b = 2.0f * dot(relativeOrigin, dir);
    float c = dot(relativeOrigin, relativeOrigin) - kEarthRadius * kEarthRadius;

    float discr = b * b - 4.0f * a * c;

    // 如果判别式小于0，说明没有交点
    if (discr < 0.0f)
    {
        // 返回一个特殊值来表示没有交点，例如一个“无效”向量
        return vec3(0.0f);
    }

    // 计算两个交点参数t
    float t1 = (-b - sqrt(discr)) / (2.0f * a);
    float t2 = (-b + sqrt(discr)) / (2.0f * a);

    // 通常我们只关心“最近”的那个交点（t值最小且大于0）
    float t_intersect;

    if (t1 > 0.0f)
    {
        t_intersect = t1;
    }
    else if (t2 > 0.0f)
    {
        t_intersect = t2;
    }
    else
    {
        // 两个t值都小于等于0，表示光线从球体内部射出，或球体在光线背后
        // 此时可以认为没有有效交点
        return vec3(0.0f); // 同样，返回一个特殊值
    }

    // 根据找到的有效t值计算交点坐标
    vec3 intersectionPoint = ori + t_intersect * dir;
    return intersectionPoint;
}

// 计算一个点p相对于地球表面的高度
float heightToGround(vec3 p)
{
    return (length(p - earthCenter) - earthRadius);
    // return p.y;
}

float phaseRayleigh(vec3 _camRayDir, vec3 _sunDir)
{
    float cosine = dot(_camRayDir, _sunDir) / length(_camRayDir) / length(_sunDir);
    return 3.0f / 16.0f * PI * (1 + cosine * cosine);
}
float phaseMie(vec3 _camRayDir, vec3 _sunDir)
{
    float gMie2 = gMie * gMie;
    float cosine = dot(_camRayDir, _sunDir) / length(_camRayDir) / length(_sunDir);
    return (1.0 - gMie2) / pow(1.0 + gMie2 - 2.0 * gMie * cosine, 1.5);

    // return 3.0f / 8.0f * PI * (1.0 - gMie2) * (1 + cosine * cosine) / (2 + gMie2) / pow((1 + gMie2 - 2.0 * gMie * cosine), 1.5f);
}
float rhoRayleigh(float h)
{
    if (h < 0.0f)
    {
        h = 0.0f;
    }
    return atmosphereDensity * exp(-abs(h) / HRayleigh); // 大气密度近似
}
float rhoMie(float h)
{
    if (h < 0.0f)
    {
        h = 0.0f;
    }
    return MieDensity * exp(-abs(h) / HMie);
}

vec4 transmittanceRayleigh(vec3 ori, vec3 end, float scale)
{

    vec4 t; // 透射率

    // int tMaxStep = 32; //透射率步进步数
    float dist = length(end - ori);
    float tItvl = dist / float(maxStep);

    float opticalDepth = 0.f;
    for (int i = 0; i < maxStep; ++i)
    {
        vec3 p = i * (end - ori);
        float h = heightToGround(p);
        opticalDepth += tItvl * rhoRayleigh(h) * scale;
    }

    t = vec4(
        exp(-betaRayleigh.r * opticalDepth),
        exp(-betaRayleigh.g * opticalDepth),
        exp(-betaRayleigh.b * opticalDepth),
        1.0f);
    return t;
}

vec4 transmittanceMie(vec3 ori, vec3 end, float scale)
{
    vec4 t; // 透射率

    float dist = length(end - ori);
    float tItvl = dist / float(maxStep);

    float opticalDepth = 0.f;
    for (int i = 0; i < maxStep; ++i)
    {
        vec3 p = i * (end - ori);
        float h = heightToGround(p);
        opticalDepth += tItvl * rhoMie(h) * scale;
    }
    vec4 extiction = betaMie * (1.0 + absorbMie);
    t = vec4(exp(-extiction.r * opticalDepth),
             exp(-extiction.g * opticalDepth),
             exp(-extiction.b * opticalDepth),
             1.0f);
    return t;
}

// 透射率步数大幅会影响散射效果,为什么? 因为公式有误,p步进幅度忘记/tMaxStep了
vec4 transmittance(vec3 ori, vec3 end, float scale)
{
    vec4 t;                                              // 透射率
    vec4 betaMieAbsorb = vec4(2.5e-5, 4e-5, 1e-5, 1.0f); // Hacking 让地平线呈现微妙紫色
    const float tMaxStep = 64;

    float dist = length(end - ori);
    float tItvl = dist / float(tMaxStep);

    // 光学深度积分
    float opticalDepthMie = 0.f;
    float opticalDepthRayleigh = 0.f;
    for (int i = 0; i < tMaxStep; ++i)
    {
        vec3 p = ori + i * (end - ori) / tMaxStep;
        float h = heightToGround(p);
        opticalDepthMie += tItvl * rhoMie(h) * scale;
        opticalDepthRayleigh += tItvl * rhoRayleigh(h) * scale;
    }

    // 总透射率计算
    vec4 extictionMie = (betaMie + betaMieAbsorb * absorbMie) * opticalDepthMie;
    vec4 extictionRayleigh = betaRayleigh * opticalDepthRayleigh;
    t = vec4(exp(-(extictionMie + extictionRayleigh).r),
             exp(-(extictionMie + extictionRayleigh).g),
             exp(-(extictionMie + extictionRayleigh).b),
             1.0f);
    return t;
}

vec4 scatterCoefficientRayleigh(vec3 p)
{
    // vec3 intersection = intersectSky(camPos, camRayDir);

    float h = (heightToGround(p)); // 散射点高度
    return betaRayleigh * rhoRayleigh(h);
}
vec4 scatterCoefficientMie(vec3 p)
{
    // vec3 intersection = intersectSky(camPos, camRayDir);

    float h = (heightToGround(p)); // 散射点高度
    return betaMie * rhoMie(h);
}

vec3 skyTonemap(vec3 color)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.20;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2; // white point
    // color = pow(color, vec3(0.95f, 0.96f, 0.94f));

    return ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    return color;
}

vec4 computeSkyColor()
{
    vec4 skyColor;
    vec4 scatterRayleigh = vec4(0.0f);
    vec4 scatterMie = vec4(0.0f);

    vec3 camSkyIntersection = intersectSky(camPos, camRayDir); // 摄像机视线与天空交点
    itvl = length(camSkyIntersection - camPos) / float(maxStep);
    for (int i = 0; i < maxStep; ++i)
    {
        if (camSkyIntersection == vec3(0.0f))
        {
            return vec4(0.000f); // 散射点阳光被地面阻挡
        }
        vec3 scatterPoint = camPos + i * itvl * camRayDir;
        vec3 scatterSkyIntersection = intersectSky(scatterPoint, sunDir);     // 散射点与天空交点
        vec3 scatterEarthIntersection = intersectEarth(scatterPoint, sunDir); // 散射点与地面交点
        if (scatterEarthIntersection != vec3(0.0f) && length(scatterEarthIntersection - scatterPoint) < length(scatterSkyIntersection - scatterPoint))
        {
            continue; // 散射点阳光被地面阻挡
        }
        vec4 t1 = transmittance(camPos, scatterPoint, 1.0f);                 // 摄像机到散射点的透射率
        vec4 t2 = transmittance(scatterPoint, scatterSkyIntersection, 1.0f); // 散射点到天空边界的透射率

        scatterRayleigh += scatterCoefficientRayleigh(scatterPoint) * t1 * t2;

        scatterMie += scatterCoefficientMie(scatterPoint) * t1 * t2;
    }

    scatterRayleigh *= phaseRayleigh(camRayDir, sunDir);

    scatterMie *= phaseMie(camRayDir, sunDir);

    skyColor += scatterRayleigh;
    skyColor += scatterMie * MieIntensity;
    skyColor.rgb = skyTonemap(skyColor.rgb);

    return vec4(dirLightIntensity, 1.0f) * skyColor * skyIntensity * itvl;
}

vec4 computeAerialPerspective(vec3 camEarthIntersection)
{
    vec4 aerialColor;
    vec4 scatterRayleigh = vec4(0.0f);
    vec4 scatterMie = vec4(0.0f);

    itvl = length(camEarthIntersection - camPos) / float(maxStep);
    for (int i = 0; i < maxStep; ++i)
    {
        vec3 scatterPoint = camPos + i * itvl * camRayDir;
        vec3 scatterSkyIntersection = intersectSky(scatterPoint, sunDir);     // 散射点与天空交点
        vec3 scatterEarthIntersection = intersectEarth(scatterPoint, sunDir); // 散射点与地面交点
        if (scatterEarthIntersection != vec3(0.0f) && length(scatterEarthIntersection - scatterPoint) < length(scatterSkyIntersection - scatterPoint))
        {
            continue; // 散射点阳光被地面阻挡
        }
        vec4 t1 = transmittance(camPos, scatterPoint, 1.0f);                 // 摄像机到散射点的透射率
        vec4 t2 = transmittance(scatterPoint, scatterSkyIntersection, 1.0f); // 散射点到天空边界的透射率

        scatterRayleigh += scatterCoefficientRayleigh(scatterPoint) * t1 * t2;

        scatterMie += scatterCoefficientMie(scatterPoint) * t1 * t2;
    }

    scatterRayleigh *= phaseRayleigh(camRayDir, sunDir);

    scatterMie *= phaseMie(camRayDir, sunDir);

    aerialColor += scatterRayleigh;
    aerialColor += scatterMie * MieIntensity;
    aerialColor.rgb = skyTonemap(aerialColor.rgb);

    return vec4(dirLightIntensity, 1.0f) * aerialColor * itvl;
}

vec3 computeSunlightDecay(vec3 camPos, vec3 fragDir, vec3 sunDir)
{
    vec3 skyIntersection = intersectSky(camPos, sunDir);
    vec4 t1 = transmittance(camPos, skyIntersection, 1.0f); // 散射点到摄像机的透射率   决定天顶-地平线透射率差异

    return t1.rgb;
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
void initialize()
{
    SkyResult = vec4(0.f, 0.f, 0.f, 1.f); // Initialize Sky

    camRayDir = Dir;
    camPos = eyePos;
    earthCenter = vec3(0.0f, -earthRadius, 0.0f); // 地球球心，位于地面原点正下方
    sunDir = dirLightPos;
    sunlightDecay = computeSunlightDecay(camPos, camRayDir, sunDir);
}
vec3 dirLightDiffuse(vec3 fragPos, vec3 n)
{

    vec3 diffuse = vec3(0.0f);
    vec3 l = normalize(dirLightPos);
    float rr = dot(l, l);

    diffuse += dirLightIntensity / rr * max(0.f, dot(n, l)) * sunlightDecay;

    return diffuse;
}

vec3 saturate_color(vec3 color, float amount)
{

    float luma = dot(color, vec3(0.299, 0.587, 0.114));

    return luma + (color - luma) * amount;
}
void main()
{
    initialize();

    // 天空计算

    float camHeight = length(camPos - earthCenter) - earthRadius;

    vec3 camEarthIntersection = intersectEarth(camPos, camRayDir);
    if (camEarthIntersection != vec3(0.0f))
    {

        // 击中地球,渲染大气透视
        SkyResult = computeAerialPerspective(camEarthIntersection);

        vec4 t1 = transmittance(camPos, camEarthIntersection, 1.0f);

        // 渲染地面
        vec3 normal = normalize(camEarthIntersection - earthCenter);
        vec3 lighting = dirLightDiffuse(camEarthIntersection, normal);
        vec3 earthBaseColor = vec3(0.3, 0.3f, 0.34f); // 地面颜色
        SkyResult.rgb += lighting * earthBaseColor * t1.rgb;
    }
    else
    {
        if (camHeight > skyHeight)
        {
            // 摄像机在大气层外
        }
        else
        {
            SkyResult += computeSkyColor();
        }
    }
    SkyResult.rgb = clamp(SkyResult.rgb, vec3(0.0f), vec3(1.0f));

    // SkyResult = vec4(1.0f);
}
