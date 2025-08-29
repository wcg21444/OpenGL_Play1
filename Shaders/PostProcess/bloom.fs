#version 330 core
out vec4 BrightColor;
in vec2 TexCoord;

/*****************Screen输入*****************************************************************/
uniform sampler2D screenTex;
/*****************视口大小******************************************************************/
uniform int width = 1600;
uniform int height = 900;

/****************Bloom参数****************************************************************/
uniform float threshold;
uniform float bloomIntensity;

vec4 saturate(in vec4 v)
{
    return clamp(v, 0.0f, 1.0f);
}
vec4 ACESApprox(vec4 col)
{
    col *= 0.2f;
    float a = 0.91f;
    float b = 0.03f;
    float c = 1.03f;
    float d = 0.09f;
    float e = 0.14f;
    return saturate((col * (a * col + b)) / (col * (c * col + d) + e));
}
void main()
{
    vec4 FragColor = texture(screenTex, TexCoord);
    // check whether fragment output is higher than threshold, if so output as brightness color

    // 太阳bloom过亮
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > threshold * 4.f - 4.f)
        BrightColor = (brightness - threshold) * log(1 + vec4(FragColor.rgb, 1.0));
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

    BrightColor = ACESApprox(BrightColor) * bloomIntensity;
}