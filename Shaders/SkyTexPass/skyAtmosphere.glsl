vec3 camDir = vec3(0.f);
vec3 camPos = vec3(0.f);
vec3 sunDir = vec3(0.f);
float itvl;

vec4 computeSkyColor(in vec3 sunLightIntensity)
{
    vec4 skyColor;
    vec4 scatterRayleigh = vec4(0.0f);
    vec4 scatterMie = vec4(0.0f);

    vec4 op1 = vec4(0.0f);
    vec3 camSkyIntersection = intersectSky(camPos, camDir); // 摄像机视线与天空交点
    itvl = length(camSkyIntersection - camPos) / float(maxStep);
    for (int i = 0; i < maxStep; ++i)
    {
        if (camSkyIntersection == NO_INTERSECTION)
        {
            return vec4(0.000f); // 散射点阳光被地面阻挡
        }
        vec3 scatterPoint = camPos + i * itvl * camDir;
        vec3 scatterSkyIntersection = intersectSky(scatterPoint, sunDir);     // 散射点与天空交点
        vec3 scatterEarthIntersection = intersectEarth(scatterPoint, sunDir); // 散射点与地面交点

        // if (scatterEarthIntersection != NO_INTERSECTION && length(scatterEarthIntersection - scatterPoint) < length(scatterSkyIntersection - scatterPoint))
        // {
        //     break; // 散射点阳光被地面阻挡
        // }
        // 这一部分导致日落天空亮度突变,不真实

        // vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, scatterPoint);
        accumulateOpticalDepth(op1, scatterPoint, scatterPoint + itvl * camDir);
        vec4 t1 = exp(-op1);
        vec4 t2 = getTransmittanceFromLUTSky(transmittanceLUT, earthRadius, earthRadius + skyHeight, scatterPoint, scatterSkyIntersection); // 散射点到天空边界的透射率

        // vec4 t1 = transmittance(camPos, scatterPoint, 1.0f); // 摄像机到散射点的透射率
        // vec4 t2 = transmittance(scatterPoint, scatterSkyIntersection, 1.0f); // 散射点到天空边界的透射率
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

vec4 computeAerialPerspective(vec3 camEarthIntersection, in vec3 sunLightIntensity)
{
    vec4 aerialColor;
    vec4 scatterRayleigh = vec4(0.0f);
    vec4 scatterMie = vec4(0.0f);
    itvl = length(camEarthIntersection - camPos) / float(maxStep);

    vec4 op1 = vec4(0.0f);
    for (int i = 0; i < maxStep; ++i)
    {
        vec3 scatterPoint = camPos + i * itvl * camDir;
        vec3 scatterSkyIntersection = intersectSky(scatterPoint, sunDir);     // 散射点与天空交点
        vec3 scatterEarthIntersection = intersectEarth(scatterPoint, sunDir); // 散射点与地面交点
        if (scatterEarthIntersection != NO_INTERSECTION && length(scatterEarthIntersection - scatterPoint) < length(scatterSkyIntersection - scatterPoint))
        {
            break; // 散射点阳光被地面阻挡
        }

        accumulateOpticalDepth(op1, scatterPoint, scatterPoint + itvl * camDir);

        vec4 t1 = exp(-op1);
        // vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, scatterPoint);                    // 摄像机到散射点的透射率
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
    float exponent = 1e2; // 锐利程度
    float sunSizeInner = 1.f - 1e-6;
    float sunSizeOuter = 1.f - 1e-3;

    float sunDot = dot(normalize(fragDir), normalize(sunDir));
    float sunSmoothstep = smoothstep(sunSizeOuter, sunSizeInner, sunDot);

    // 返回太阳亮度，与透射率相乘
    return sunIntensity * 1e3 * pow(sunSmoothstep, exponent) * sunlightDecay;
}