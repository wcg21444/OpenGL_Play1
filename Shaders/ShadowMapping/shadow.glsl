float computeDirLightShadowVSM(vec3 fragPos, vec3 fragNormal, in DirLight dirLight)
{
    if (DirShadow == 0)
    {
        return 0.f;
    }
    vec4 fragPosLightSpace = dirLight.spaceMatrix * vec4(fragPos, 1.0f);
    vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.x < 0.0 || projCoords.x > 1.0 ||
       projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 0.0;
    }
    float cosine = dot(fragNormal, dirLight.pos) / length(fragNormal) / length(dirLight.pos);
    float bias = sqrt(1 - cosine * cosine) / cosine * 1e-5;
    float currentDepth = projCoords.z - bias;

    if (currentDepth > 1.0)
    {
        return 0.0f;
    }
    vec4 moments = texture(dirLight.VSMTexture, projCoords.xy);
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

vec2 SATLookUp(in sampler2D SAT, in vec2 uvMin, in vec2 uvMax)
{
    vec2 A = uvMin;
    vec2 B = vec2(uvMax.x, uvMin.y);
    vec2 C = vec2(uvMin.x, uvMax.y);
    vec2 D = uvMax;
    return texture(SAT, D).rg - texture(SAT, B).rg - texture(SAT, C).rg + texture(SAT, A).rg;
}
// 还原深度偏移
float reverseDepthBias(float moment)
{
    return moment + 0.5f;
}

float computeDirLightShadowVSMSAT(vec3 fragPos, vec3 fragNormal, in DirLight dirLight)
{
    if (DirShadow == 0)
    {
        return 0.f;
    }
    vec4 fragPosLightSpace = dirLight.spaceMatrix * vec4(fragPos, 1.0f);
    vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.x < 0.0 || projCoords.x > 1.0 ||
       projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 0.0;
    }
    float cosine = dot(fragNormal, dirLight.pos) / length(fragNormal) / length(dirLight.pos);
    float bias = sqrt(1 - cosine * cosine) / cosine * 1e-4;
    float currentDepth = projCoords.z - bias;
    // float currentDepth = projCoords.z;

    if (currentDepth<0.0f||currentDepth > 1.0)
    {
        return 0.0f;
    }
    // vec4 moments = texture(dirLight.VSMTexture, projCoords.xy);
    // float depthAvg = moments.r;
    // float depthSquareAvg = moments.g;

    vec2 texSize = textureSize(dirLight.SATTexture, 0);
    float kernelSize = VSSMKernelSize;
    vec2 halfKernel = vec2(kernelSize) * 0.5;
    // 计算搜索区域的 UV 坐标
    vec2 uvMin = projCoords.xy - halfKernel;
    vec2 uvMax = projCoords.xy + halfKernel;

    // 2. 使用 SAT 查询深度均值和深度平方均值
    float kernelArea = (uvMax.x - uvMin.x) * (uvMax.y - uvMin.y) * texSize.x * texSize.y;

    // 获取深度总和，然后计算均值
    float sumZ = SATLookUp(dirLight.SATTexture, uvMin, uvMax).r;
    float depthAvg = sumZ / kernelArea;

    // 获取深度平方总和，然后计算均值
    float sumZ2 = SATLookUp(dirLight.SATTexture, uvMin, uvMax).g;
    float depthSquareAvg = sumZ2 / kernelArea;

    // 还原深度偏移
    if (useBias == 0)
    {
        depthAvg = depthAvg;
        depthSquareAvg = depthSquareAvg;
    }
    else
    {
        depthAvg = reverseDepthBias(depthAvg);
        depthSquareAvg = reverseDepthBias(depthSquareAvg);
    }

    if (depthAvg < 1e-7)
    {
        return 0.f; // 当前深度<平均深度,没有遮挡
    }
    if (currentDepth <= depthAvg)
    {
        return 0.f; // 当前深度<平均深度,没有遮挡
    }

    const float MIN_VAR = 1e-9;
    float variance = depthSquareAvg - depthAvg * depthAvg;
    variance = max(variance, MIN_VAR);

    float d = currentDepth - depthAvg;
    float Pmax = variance / (variance + d * d);
    return 1 - Pmax;
}

// 求切比雪夫上界
float chebyshev(vec2 moments, float currentDepth)
{
    const float MIN_VAR = 1e-5;
    float variance = moments.y - moments.x * moments.x;
    variance = max(variance, MIN_VAR);

    float d = currentDepth - moments.x;
    float Pmax = variance / (variance + d * d);
    return Pmax;
}
vec2 getVSSMMoments(sampler2D satTex, vec2 uv, float kernelSize)
{
    vec2 stride = 1.0 / vec2(textureSize(satTex, 0));

    float xmax = uv.x + kernelSize * stride.x;
    float xmin = uv.x - kernelSize * stride.x;
    float ymax = uv.y + kernelSize * stride.y;
    float ymin = uv.y - kernelSize * stride.y;

    vec4 A = texture(satTex, vec2(xmin, ymin));
    vec4 B = texture(satTex, vec2(xmax, ymin));
    vec4 C = texture(satTex, vec2(xmin, ymax));
    vec4 D = texture(satTex, vec2(xmax, ymax));

    float sPenumbra = 2.0 * kernelSize;

    vec4 moments = (D + A - B - C) / float(sPenumbra * sPenumbra);
    if (useBias == 1)
    {
        moments.x = reverseDepthBias(moments.x);
        moments.y = reverseDepthBias(moments.y);
    }
    return moments.rg;
}

// 返回阴影系数
float computeDirLightShadowVSSM(vec3 fragPos, vec3 fragNormal, in DirLight dirLight)
{
    if (DirShadow == 0)
    {
        return 0.f;
    }
    vec4 fragPosLightSpace = dirLight.spaceMatrix * vec4(fragPos, 1.0f);
    vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float cosine = dot(fragNormal, dirLight.pos) / length(fragNormal) / length(dirLight.pos);
    float bias = sqrt(1 - cosine * cosine) / cosine * 1e-12;
    float currentDepth = projCoords.z - bias;

    float lightSize = VSSMKernelSize*5.f;

    float searchSize = lightSize;
    vec2 moments = getVSSMMoments(dirLight.SATTexture, projCoords.xy, searchSize);

    if (currentDepth >= 0.99f)
    {
        return 0.f;
    }
    // Blocker Searching
    float border = searchSize / textureSize(dirLight.SATTexture, 0).x;
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
    moments = getVSSMMoments(dirLight.SATTexture, projCoords.xy, wPenumbra);

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

float computeDirLightShadow(vec3 fragPos, vec3 fragNormal, in DirLight dirLight)
{
    // perform perspective divide
    if (DirShadow == 0)
    {
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
    for (int j = 0; j < n_samples; ++j)
    {
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
        float closestDepth = texture(dirLight.depthMap, projCoords.xy).r;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;

        float cosine = dot(fragNormal, dirLight.pos) / length(fragNormal) / length(dirLight.pos);
        // float texel = 4e-5; // texel = (ortho_scale,farPlane)
        float texel = dirLight.orthoScale * 5e-6;

        float bias = sqrt(1 - cosine * cosine) / cosine * texel;
        // check whether current frag pos is in shadow
        factor += currentDepth - closestDepth - bias > 0 ? 1.0f : 0.0f;
    }

    return factor / n_samples;
    // return dirLight.orthoScale * 1e-1 + 0.f;
}

float computeDirLightShadowPCSS(vec3 fragPos, vec3 fragNormal, in DirLight dirLight)
{
    // perform perspective divide
    if (DirShadow == 0)
    {
        return 0.f;
    }
    vec4 lightSpaceFragPos = dirLight.spaceMatrix * vec4(fragPos, 1.0f);

    // // 计算遮挡物与接受物的平均距离
    float d = 0.f;
    int occlusion_times = 0;
    for (int k = 0; k < n_samples; ++k)
    {
        vec4 sampleOffset = dirLight.spaceMatrix * vec4(TBN * shadowSamples[k] * blurRadius, 0.0f);
        vec4 fragPosLightSpace = lightSpaceFragPos + sampleOffset; // Dir Light View Space
        vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(dirLight.depthMap, projCoords.xy).r;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;

        float bias = dirLight.farPlane * dirLight.orthoScale * 1e-12;

        if (currentDepth - closestDepth - bias > 0)
        {
            occlusion_times++;
            d += (currentDepth - closestDepth)* dirLight.farPlane; // 深度值换算与光源farplane数值有关
        }
    }
    d = d / occlusion_times;
    float factor = 0.f;
    for (int j = 0; j < n_samples; ++j)
    {
        vec4 sampleOffset = dirLight.spaceMatrix * vec4(
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
        float closestDepth = texture(dirLight.depthMap, projCoords.xy).r;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;

        float cosine = dot(fragNormal, dirLight.pos) / length(fragNormal) / length(dirLight.pos);
        // float texel = 4e-5; // texel = (ortho_scale,farPlane)
        float texel =1e-1/dirLight.farPlane;

        float bias =  sqrt(1 - cosine * cosine) / cosine * texel;
        // check whether current frag pos is in shadow
        factor += currentDepth - closestDepth - bias > 0 ? 1.0f : 0.0f;
    }

    return factor / n_samples;
    // return dirLight.orthoScale * 1e-1 + 0.f;
}


float computePointLightShadowVSM(vec3 fragPos, vec3 fragNorm, in PointLight pointLight)
{
    if (PointShadow == 0)
    {
        return 0.f;
    }
    vec3 dir = fragPos - pointLight.pos;

    float currentDepth = length(dir);
    currentDepth /= pointLight.farPlane; // 归一化
    float bias = 1e-4;
    currentDepth -= bias;
    if (currentDepth > 1.0)
    {
        return 0.0f;
    }
    vec2 moments = texture(pointLight.VSMCubemap, dir).rg;
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

float computePointLightShadowPCSS(vec3 fragPos, vec3 fragNorm, in PointLight pointLight)
{
    if (PointShadow == 0)
    {
        return 0.f;
    }
    vec3 dir = fragPos - pointLight.pos;

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
        vec3 dir_sample = sampleOffset + fragPos - pointLight.pos;
        float curr_depth_sample = length(dir_sample);
        float cloest_depth_sample = texture(pointLight.depthCubemap, dir_sample).r;

        cloest_depth_sample *= pointLight.farPlane;
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
        vec3 dir_sample = sampleOffset + fragPos - pointLight.pos;
        float curr_depth_sample = length(dir_sample);
        float cloest_depth_sample = texture(pointLight.depthCubemap, dir_sample).r;
        cloest_depth_sample *= pointLight.farPlane;
        factor += (curr_depth_sample - cloest_depth_sample - bias > 0.f ? 1.0 : 0.0);
        // factor = curr_depth_sample;
    }
    return (factor) / n_samples; // return shadow
}


float computePointLightShadowPCF(vec3 fragPos, vec3 fragNorm, in PointLight pointLight)
{
    if (PointShadow == 0)
    {
        return 0.f;
    }
    vec3 dir = fragPos - pointLight.pos;

    float curr_depth = length(dir);
    float omega = -dot(fragNorm, dir) / curr_depth;
    float bias = 0.05f / tan(omega); // idea: bias increase based on center distance
    // shadow test
    float factor = 0.f;
    for (int j = 0; j < n_samples; ++j)
    {
        vec3 sampleOffset = TBN * shadowSamples[j]   / n_samples * 64;
        vec3 dir_sample = sampleOffset + fragPos - pointLight.pos;
        float curr_depth_sample = length(dir_sample);
        float cloest_depth_sample = texture(pointLight.depthCubemap, dir_sample).r;
        cloest_depth_sample *= pointLight.farPlane;
        factor += (curr_depth_sample - cloest_depth_sample - bias > 0.f ? 1.0 : 0.0);
        // factor = curr_depth_sample;
    }
    return (factor) / n_samples; // return shadow
}




float bezier(float t, float p0, float p1, float p2, float p3)
{
    return (1.0 - t) * (1.0 - t) * (1.0 - t) * p0 +
           3.0 * (1.0 - t) * (1.0 - t) * t * p1 +
           3.0 * (1.0 - t) * t * t * p2 +
           t * t * t * p3;
}

vec3 shadowEdgeTone(float shadowFactor)
{
    // p0 = (0,0), p3 = (1,1)
    // 调整 p1 和 p2 来塑造曲线
    float r = bezier(shadowFactor, 0.0, 0.1, 0.8, 1.0); // 线性贝塞尔曲线，近似线性
    float g = bezier(shadowFactor, 0.0, 0.3, 0.7, 1.0); // 标准的S形贝塞尔曲线
    float b = bezier(shadowFactor, 0.0, 0.8, 0.9, 1.0); // 快速上升，然后趋于平缓

    return vec3(r, g, b);
}