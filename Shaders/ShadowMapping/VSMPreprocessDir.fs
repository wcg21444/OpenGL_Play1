#version 330 core
out vec4 VSMResult;
in vec2 TexCoord;
uniform sampler2D depthMap;

void main()
{
    // 动态获取阴影贴图的纹素尺寸，以确保与实际纹理匹配
    vec2 texelSize = 1.0 / vec2(textureSize(depthMap, 0)) * 0.5f;

    // 定义盒式滤波的核大小
    int kernelSize = 4;
    // 循环的半个范围，即从中心点向两侧扩展的距离
    int halfKernel = kernelSize / 2;

    float sumDepth = 0.0;
    float sumDepthSquared = 0.0;

    // 遍历核中的所有纹素
    // 循环范围修正为从 -halfKernel 到 halfKernel - 1
    for (int y = -halfKernel; y < halfKernel; y++)
    {
        for (int x = -halfKernel; x < halfKernel; x++)
        {
            vec2 offset = vec2(x, y) * texelSize;
            float neighborDepth = texture(depthMap, TexCoord + offset).r;
            sumDepth += neighborDepth;
            sumDepthSquared += neighborDepth * neighborDepth;
        }
    }

    // 计算平均值
    float totalSamples = float(kernelSize * kernelSize);
    float avgDepth = sumDepth / totalSamples;
    float avgDepthSquared = sumDepthSquared / totalSamples;

    // 将平均深度和平均深度平方存储到输出纹理中
    VSMResult = vec4(avgDepth, avgDepthSquared, 0.0, 1.0);
}