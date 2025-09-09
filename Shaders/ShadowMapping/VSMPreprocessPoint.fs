#version 330 core
out vec4 VSMResult;
in vec3 Dir;
uniform samplerCube depthCubemap; // 你要进行均值滤波的立方体贴图

void main()
{
    vec3 sampleDir = normalize(Dir);

    float sumDepth = 0.f;
    float sumSquareDepth = 0.f;
    int sampleCount = 0;

    float sampleRadius = 2e-3;
    const int numOffsets = 8;
    vec3 offsets[numOffsets] = vec3[](
        vec3(1, 1, 1), vec3(1, -1, -1), vec3(-1, 1, -1), vec3(-1, -1, 1),
        vec3(1, 1, -1), vec3(1, -1, 1), vec3(-1, 1, 1), vec3(-1, -1, -1));

    for (int i = 0; i < numOffsets; ++i)
    {
        // 计算带有偏移的方向向量
        vec3 offsetDir = normalize(sampleDir + offsets[i] * sampleRadius);

        float sampleDepth = texture(depthCubemap, offsetDir).r;
        sumDepth += sampleDepth;
        sumSquareDepth += sampleDepth * sampleDepth;
        sampleCount++;
    }

    // 加上中心点本身的采样
    float sampleDepth = texture(depthCubemap, sampleDir).r;
    sumDepth += sampleDepth;
    sumSquareDepth += sampleDepth * sampleDepth;
    sampleCount++;

    // 求均值：将总和除以采样总数
    float averageColor = sumDepth / float(sampleCount);
    float averageSquareColor = sumSquareDepth / float(sampleCount);

    VSMResult = vec4(averageColor, averageSquareColor, 0.0f, 1.0);
}