
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
// 求切比雪夫上界
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

float chebyshev(vec2 moments, float currentDepth)
{
    const float MIN_VAR = 1e-5;
    float variance = moments.y - moments.x * moments.x;
    variance = max(variance, MIN_VAR);

    float d = currentDepth - moments.x;
    float Pmax = variance / (variance + d * d);
    return Pmax;
}
