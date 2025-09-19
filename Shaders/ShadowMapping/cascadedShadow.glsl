
float computeDirLightShadowUnitVSM(vec3 fragPos, vec3 fragNormal, in DirShadowUnit shadowUnit)
{
    if (DirShadow == 0)
    {
        return 0.f;
    }
    vec4 fragPosLightSpace = shadowUnit.spaceMatrix * vec4(fragPos, 1.0f);
    vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 0.0;
    }
    float cosine = dot(fragNormal, shadowUnit.pos) / length(fragNormal) / length(shadowUnit.pos);
    float bias = sqrt(1 - cosine * cosine) / cosine * 1e-5;
    float currentDepth = projCoords.z - bias;

    if (currentDepth > 1.0)
    {
        return 0.0f;
    }
    vec4 moments = texture(shadowUnit.VSMTexture, projCoords.xy);
    float depthAvg = moments.r;
    float depthSquareAvg = moments.g;

    if (currentDepth <= depthAvg)
    {
        return 0.f; // 当前深度<平均深度,没有遮挡
    }

    const float MIN_VAR = 1e-11;
    float variance = depthSquareAvg - depthAvg * depthAvg;
    variance = max(variance, MIN_VAR);

    float d = currentDepth - depthAvg;
    float Pmax = variance / (variance + d * d);
    return 1 - Pmax;
}

float computeDirLightShadowUnitVSSM(vec3 fragPos, vec3 fragNormal, in DirShadowUnit shadowUnit)
{
    if (DirShadow == 0)
    {
        return 0.f;
    }
    vec4 fragPosLightSpace = shadowUnit.spaceMatrix * vec4(fragPos, 1.0f);
    vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float cosine = dot(fragNormal, shadowUnit.pos) / length(fragNormal) / length(shadowUnit.pos);
    float bias = sqrt(1 - cosine * cosine) / cosine * 1e-12;
    float currentDepth = projCoords.z - bias;

    float lightSize = VSSMKernelSize * 5.f;

    float searchSize = lightSize;
    vec2 moments = getVSSMMoments(shadowUnit.SATTexture, projCoords.xy, searchSize);

    if (currentDepth >= 0.99f)
    {
        return 0.f;
    }
    // Blocker Searching
    float border = searchSize / textureSize(shadowUnit.SATTexture, 0).x;
    // just cut out the no padding area according to the sarched area size
    if (projCoords.x <= border || projCoords.x >= 0.99f - border)
    {
        return 0.0;
    }
    if (projCoords.y <= border || projCoords.y >= 0.99f - border)
    {
        return 0.0;
    }

    float alpha = chebyshev(moments, currentDepth);                         // 未遮挡比例
    float dBlocker = (moments.x - alpha * (currentDepth)) / (1.0f - alpha); // 阻挡物体深度
    if (dBlocker < 1e-9)
    {
        return 0.0f; // 阻挡物深度较小 认为没有遮挡
    }
    if (dBlocker > 1.0f) // 阻挡物深度大于1 认为没有遮挡
    {
        return 0.0f;
    }
    float wPenumbra = (currentDepth - dBlocker) * lightSize / dBlocker; // 半影大小计算
    if (wPenumbra <= 0.0)                                               // 当前深度小于等于阻挡物深度 认为没有遮挡
    {
        return 0.0f;
    }
    moments = getVSSMMoments(shadowUnit.SATTexture, projCoords.xy, wPenumbra);

    if (currentDepth > 1.0) // 当前深度大于1 认为没有遮挡
    {
        return 0.0f;
    }
    if (currentDepth - moments.x <= 1e-3f)
    {
        return 0.0f; // 当前深度小于平均深度,认为没有遮挡
    }
    if (currentDepth <= 1e-6)
    {
        return 0.f; // 深度<=0 认为没有遮挡
    }

    // CDF estimation
    float shadow = chebyshev(moments, currentDepth);
    return 1.f - shadow;
}

float computeDirLightShadowUnit(vec3 fragPos, vec3 fragNormal, in DirShadowUnit shadowUnit)
{
    // perform perspective divide
    if (DirShadow == 0)
    {
        return 0.f;
    }
    vec4 lightSpaceFragPos = shadowUnit.spaceMatrix * vec4(fragPos, 1.0f);
    // PCF Only
    float factor = 0.f;
    for (int j = 0; j < n_samples; ++j)
    {
        vec4 sampleOffset = shadowUnit.spaceMatrix * vec4(
                                                         TBN *
                                                             shadowSamples[j] *
                                                             blurRadius * 5.f*pow(shadowUnit.farPlane,0.5f),
                                                         0.0f);
        vec4 samplePosLightSpace = lightSpaceFragPos + sampleOffset; // Dir Light View Space
        vec3 projCoords = (samplePosLightSpace.xyz) / samplePosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
            projCoords.y < 0.0 || projCoords.y > 1.0)
        {
            continue;
        }
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(shadowUnit.depthMap, projCoords.xy).r;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;

        float bias = max(0.05 * (1.0 - dot(normal, normalize(shadowUnit.pos))), 0.005);
        bias /= pow(shadowUnit.farPlane,0.25f)*0.5f;
        // check whether current frag pos is in shadow
        factor += currentDepth - closestDepth - bias > 0 ? 1.0f : 0.0f;
    }

    return factor / n_samples;
    // return dirLight.orthoScale * 1e-1 + 0.f;
}

float computeDirLightShadowUnitPCSS(vec3 fragPos, vec3 fragNormal, in DirShadowUnit shadowUnit)
{
    // perform perspective divide
    if (DirShadow == 0)
    {
        return 0.f;
    }
    vec4 lightSpaceFragPos = shadowUnit.spaceMatrix * vec4(fragPos, 1.0f);

    // // 计算遮挡物与接受物的平均距离
    float d = 0.f;
    int occlusion_times = 0;
    for (int k = 0; k < n_samples; ++k)
    {
        vec4 sampleOffset = shadowUnit.spaceMatrix * vec4(TBN * shadowSamples[k] * blurRadius, 0.0f);
        vec4 fragPosLightSpace = lightSpaceFragPos + sampleOffset; // Dir Light View Space
        vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(shadowUnit.depthMap, projCoords.xy).r;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;

        float bias = shadowUnit.farPlane * shadowUnit.orthoScale * 1e-12;

        if (currentDepth - closestDepth - bias > 0)
        {
            occlusion_times++;
            d += (currentDepth - closestDepth) * shadowUnit.farPlane; // 深度值换算与光源farplane数值有关
        }
    }
    d = d / occlusion_times;
    float factor = 0.f;
    for (int j = 0; j < n_samples; ++j)
    {
        vec4 sampleOffset = shadowUnit.spaceMatrix * vec4(
                                                         TBN *
                                                             shadowSamples[j] *
                                                             blurRadius * pow(d, 2),
                                                         0.0f);
        vec4 samplePosLightSpace = lightSpaceFragPos + sampleOffset; // Dir Light View Space
        vec3 projCoords = (samplePosLightSpace.xyz) / samplePosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
            projCoords.y < 0.0 || projCoords.y > 1.0)
        {
            continue;
        }
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(shadowUnit.depthMap, projCoords.xy).r;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;

        float cosine = dot(fragNormal, shadowUnit.pos) / length(fragNormal) / length(shadowUnit.pos);
        // float texel = 4e-5; // texel = (ortho_scale,farPlane)
        float texel = 1e-1 / shadowUnit.farPlane;

        float bias = sqrt(1 - cosine * cosine) / cosine * texel;
        // check whether current frag pos is in shadow
        factor += currentDepth - closestDepth - bias > 0 ? 1.0f : 0.0f;
    }

    return factor / n_samples;
    // return shadowUnit.orthoScale * 1e-1 + 0.f;
}

int getCSMLevel(float zDepth) // level 取值与 程序设置紧耦合
{
    const float nearLv1 = 80.f;

    if (zDepth < nearLv1)
    {
        return 0;
    }
    else if (zDepth < nearLv1 * pow(4.0f, 1))
    {
        return 1;
    }
    else if (zDepth < nearLv1 * pow(4.0f, 2))
    {
        return 2;
    }
    else if (zDepth < nearLv1 * pow(4.0f, 3))
    {
        return 3;
    }
    else if (zDepth < nearLv1 * pow(4.0f, 4))
    {
        return 4;
    }
    else
    {
        return 5;
    }
}

float CSMShadow(in vec3 fragPos, in vec3 fragNormal, in CSMComponent CSM, in mat4 view)
{
    float fragDepth = abs((view * vec4(fragPos, 1.0f)).z);
    int level = getCSMLevel(fragDepth);
    // 先得进行camera view 变换 将坐标转换成摄像机viewspace
    // 然后取z值
    if (CSM.useVSSM == 1)
    {
        return computeDirLightShadowUnitVSSM(fragPos, fragNormal, CSM.units[level]);
    }
    if (CSM.useVSM == 1)
    {
        return computeDirLightShadowUnitVSM(fragPos, fragNormal, CSM.units[level]);
    }
    return computeDirLightShadowUnit(fragPos, fragNormal, CSM.units[level]);
    // return 1.f / float(level + 1);
}
