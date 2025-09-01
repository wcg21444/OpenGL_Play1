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

// 用于直接计算的透射率函数
vec4 transmittance(vec3 ori, vec3 end, float scale) {
    vec4 t; // 透射率
    // const vec4 betaMieAbsorb = vec4(2.5e-5, 4e-5, 1e-5, 1.0f); // Hacking 让地平线呈现微妙紫色
    const vec4 betaMieAbsorb = vec4(2.5e-5, 4e-5, 1e-5, 1.0f); // Hacking 让地平线呈现微妙紫色
    const vec4 betaOzoneAbsorb = vec4(0.650f, 1.881f, 0.085f, 1.0f) * 1e-6f;
    const float tMaxStep = 16;

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

// 返回光学深度而非透射率  用于计算LUT
vec4 transmittanceOpticalDepth(vec3 ori, vec3 end, float scale) {
    vec4 t; // 透射率
    // const vec4 betaMieAbsorb = vec4(2.5e-5, 4e-5, 1e-5, 1.0f); // Hacking 让地平线呈现微妙紫色
    const vec4 betaMieAbsorb = vec4(2.5e-5, 4e-5, 1e-5, 1.0f); // Hacking 让地平线呈现微妙紫色
    const vec4 betaOzoneAbsorb = vec4(0.650f, 1.881f, 0.085f, 1.0f) * 1e-6f;
    const float tMaxStep = 128; //

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
    return (extictionMie + extictionOzone + extictionRayleigh);
}

void accumulateOpticalDepth(in out vec4 op, vec3 ori, vec3 end) {
    // const vec4 betaMieAbsorb = vec4(2.5e-5, 4e-5, 1e-5, 1.0f); // Hacking 让地平线呈现微妙紫色
    const vec4 betaMieAbsorb = vec4(2.5e-5, 4e-5, 1e-5, 1.0f); // Hacking 让地平线呈现微妙紫色
    const vec4 betaOzoneAbsorb = vec4(0.650f, 1.881f, 0.085f, 1.0f) * 1e-6f;
    float tItvl = length(end - ori);
    int steps = 1;
    float subItvl = tItvl / steps;

    float opticalDepthMie = 0.f;
    float opticalDepthRayleigh = 0.f;
    float opticalDepthOzone = 0.f;
    float h = 0.f;
    for (int i = 0; i < steps; ++i) {
        vec3 p = ori + i * normalize(end - ori) * subItvl;
        h = heightToGround(p);
        opticalDepthMie += subItvl * rhoMie(h);
        opticalDepthRayleigh += subItvl * rhoRayleigh(h);
        opticalDepthOzone += subItvl * rhoOzone(h);
    }

    // 总透射率计算
    vec4 extictionMie = (betaMie + betaMieAbsorb * absorbMie) * opticalDepthMie;
    vec4 extictionOzone = betaOzoneAbsorb * opticalDepthOzone;
    vec4 extictionRayleigh = betaRayleigh * opticalDepthRayleigh;
    op += extictionOzone + extictionMie + extictionRayleigh;
}