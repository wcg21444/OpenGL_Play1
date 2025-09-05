float computeDirLightShadowVSM(vec3 fragPos, vec3 fragNormal, in DirLight dirLight)
{
    if (DirShadow == 0)
    {
        return 0.f;
    }
    vec4 fragPosLightSpace = dirLight.spaceMatrix * vec4(fragPos, 1.0f);
    vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

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

float SATLookUp(in sampler2D SAT, in vec2 uvMin, in vec2 uvMax)
{
    vec2 A = uvMin;
    vec2 B = vec2(uvMax.x, uvMin.y);
    vec2 C = vec2(uvMin.x, uvMax.y);
    vec2 D = uvMax;
    return texture(SAT, D).r - texture(SAT, B).r - texture(SAT, C).r + texture(SAT, A).r;
}
float SATLookUpSquare(in sampler2D SAT, in vec2 pointMin, in vec2 uvMax)
{
    vec2 A = pointMin;
    vec2 B = vec2(uvMax.x, pointMin.y);
    vec2 C = vec2(pointMin.x, uvMax.y);
    vec2 D = uvMax;
    return texture(SAT, D).g - texture(SAT, B).g - texture(SAT, C).g + texture(SAT, A).g;
}

float computeDirLightShadowVSSM(vec3 fragPos, vec3 fragNormal, in DirLight dirLight)
{
    // if (DirShadow == 0)
    // {
    //     return 0.f;
    // }
    // vec4 fragPosLightSpace = dirLight.spaceMatrix * vec4(fragPos, 1.0f);
    // vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
    // projCoords = projCoords * 0.5 + 0.5;

    // float cosine = dot(fragNormal, dirLight.pos) / length(fragNormal) / length(dirLight.pos);
    // float bias = sqrt(1 - cosine * cosine) / cosine * 1e-3;
    // float currentDepth = projCoords.z - bias;

    // if (currentDepth > 1.0)
    // {
    //     return 0.0f;
    // }
    // vec2 texSize = textureSize(dirLight.SATTexture, 0);
    // float kernelSize = 0.1;
    // vec2 halfKernel = vec2(kernelSize) * 0.5;
    // // 计算搜索区域的 UV 坐标
    // vec2 uvMin = projCoords.xy - halfKernel;
    // vec2 uvMax = projCoords.xy + halfKernel;

    // // 2. 使用 SAT 查询深度均值和深度平方均值
    // float kernelArea = (uvMax.x - uvMin.x) * (uvMax.y - uvMin.y) * texSize.x * texSize.y;

    // // 获取深度总和，然后计算均值
    // float sumZ = SATLookUp(dirLight.SATTexture, uvMin, uvMax);
    // float depthAvg = sumZ / kernelArea;

    // // 获取深度平方总和，然后计算均值
    // float sumZ2 = SATLookUpSquare(dirLight.SATTexture, uvMin, uvMax);
    // float depthSquareAvg = sumZ2 / kernelArea;

    // vec4 moments = texture(dirLight.VSMTexture, projCoords.xy);
    // float depthAvgVSM = moments.r;
    // float depthSquareAvgVSM = moments.g;

    // if (currentDepth <= depthAvg)
    // {
    //     return 0.f; // 当前深度 <= 平均深度，没有遮挡
    // }

    // const float MIN_VAR = 1e-10;
    // float variance = depthSquareAvgVSM - depthAvgVSM * depthAvgVSM;
    // variance = max(variance, MIN_VAR);

    // float d = currentDepth - depthAvgVSM;
    // float Pmax = variance / (variance + d * d);
    // return 1 - Pmax;
    if (DirShadow == 0)
    {
        return 0.f;
    }
    vec4 fragPosLightSpace = dirLight.spaceMatrix * vec4(fragPos, 1.0f);
    vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float cosine = dot(fragNormal, dirLight.pos) / length(fragNormal) / length(dirLight.pos);
    float bias = sqrt(1 - cosine * cosine) / cosine * 1e-6;
    float currentDepth = projCoords.z - bias;
    // float currentDepth = projCoords.z;

    if (currentDepth > 1.0)
    {
        return 0.0f;
    }
    // vec4 moments = texture(dirLight.VSMTexture, projCoords.xy);
    // float depthAvg = moments.r;
    // float depthSquareAvg = moments.g;

    vec2 texSize = textureSize(dirLight.SATTexture, 0);
    float kernelSize = 0.005;
    vec2 halfKernel = vec2(kernelSize) * 0.5;
    // 计算搜索区域的 UV 坐标
    vec2 uvMin = projCoords.xy - halfKernel;
    vec2 uvMax = projCoords.xy + halfKernel;

    // 2. 使用 SAT 查询深度均值和深度平方均值
    float kernelArea = (uvMax.x - uvMin.x) * (uvMax.y - uvMin.y) * texSize.x * texSize.y;

    // 获取深度总和，然后计算均值
    float sumZ = SATLookUp(dirLight.SATTexture, uvMin, uvMax);
    float depthAvg = sumZ / kernelArea;

    // 获取深度平方总和，然后计算均值
    float sumZ2 = SATLookUpSquare(dirLight.SATTexture, uvMin, uvMax);
    float depthSquareAvg = sumZ2 / kernelArea;

    if (currentDepth <= depthAvg)
    {
        return 0.f; // 当前深度<平均深度,没有遮挡
    }

    const float MIN_VAR = 1e-14;
    float variance = depthSquareAvg - depthAvg * depthAvg;
    variance = max(variance, MIN_VAR);

    float d = currentDepth - depthAvg;
    float Pmax = variance / (variance + d * d);
    return 1 - Pmax;
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

float computePointLightShadow(vec3 fragPos, vec3 fragNorm, in PointLight pointLight)
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
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(dirLight.depthMap, projCoords.xy).r;
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        float cosine = dot(fragNormal, dirLight.pos) / length(fragNormal) / length(dirLight.pos);
        // float texel = 4e-5; // texel = (ortho_scale,farPlane)
        float texel = dirLight.orthoScale * 5e-8;

        float bias = sqrt(1 - cosine * cosine) / cosine * texel;
        // check whether current frag pos is in shadow
        factor += currentDepth - closestDepth - bias > 0 ? 1.0f : 0.0f;
    }

    return factor / n_samples;
    // return dirLight.orthoScale * 1e-1 + 0.f;
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