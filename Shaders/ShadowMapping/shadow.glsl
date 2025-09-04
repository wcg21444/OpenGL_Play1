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
