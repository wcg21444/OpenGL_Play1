#version 330 core
out vec4 SkyEnvmapResult;
in vec3 Dir;

uniform samplerCube skyTexture; // 要进行高斯模糊的立方体贴图
uniform float blurSigma = 0.1f; // 高斯模糊的标准差，值越大，模糊越强
uniform int numSamples = 16;    // 采样点的数量，值越大，效果越好但性能越低

// 伪随机数生成器，用于在着色器中生成可重复的随机数
// 这个函数根据输入向量生成一个0到1之间的伪随机数
float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec3 sampleDir = normalize(Dir);
    vec3 sumColor = vec3(0.0);
    float totalWeight = 0.0;

    // --- 高斯模糊采样 ---
    // 高斯函数在球面上的权重计算
    // 这里的权重基于角度而非距离
    float sigmaSq = blurSigma * blurSigma;

    // 获取两个正交向量，用于构建切线空间
    vec3 upVector = vec3(0.0, 1.0, 0.0);
    vec3 rightVector = cross(upVector, sampleDir);
    if (length(rightVector) < 0.001)
    {
        // 如果 Dir 接近 Up Vector，则用另一个向量计算
        rightVector = cross(vec3(1.0, 0.0, 0.0), sampleDir);
    }
    rightVector = normalize(rightVector);
    vec3 upTangent = normalize(cross(sampleDir, rightVector));

    // 使用伪随机数和切线空间进行半随机采样
    for (int i = 0; i < numSamples; ++i)
    {
        // 生成两个伪随机数，用于在切线平面上采样
        vec2 randomOffset = vec2(rand(vec2(i, 0.0)), rand(vec2(0.0, i)));
        // 将随机数映射到以 (0.5, 0.5) 为中心的范围
        randomOffset = (randomOffset - 0.5) * 2.0;

        // 根据随机偏移和模糊强度生成采样方向
        vec3 offsetDir = sampleDir + (rightVector * randomOffset.x + upTangent * randomOffset.y) * blurSigma * 20.0;
        offsetDir = normalize(offsetDir);

        // 计算两个方向向量之间的夹角余弦值
        float cosAngle = dot(sampleDir, offsetDir);

        // 使用球面高斯权重公式
        // e^(-(1-cos(theta))/(2*sigma^2))
        // 注意：这里使用 1-cos(theta) 是因为角度越小，cos值越接近1，1-cos值越小，符合高斯曲线
        float weight = exp(-(1.0 - cosAngle) / sigmaSq);

        vec3 sampleColor = texture(skyTexture, offsetDir).rgb;
        sumColor += sampleColor * weight;
        totalWeight += weight;
    }

    // 求加权平均值
    if (totalWeight > 0.0)
    {
        vec3 averageColor = sumColor / totalWeight;
        SkyEnvmapResult = vec4(averageColor, 1.0f);
    }
    else
    {
        // 如果 totalWeight 为0，使用中心点的颜色以防除以零
        SkyEnvmapResult = texture(skyTexture, sampleDir);
    }
}