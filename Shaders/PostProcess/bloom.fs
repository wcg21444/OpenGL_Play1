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

void main()
{
    vec4 FragColor = texture(screenTex, TexCoord);
    // check whether fragment output is higher than threshold, if so output as brightness color

    // 太阳bloom过亮
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    brightness = clamp(brightness, 0.0f, 1.0f);
    if (brightness > threshold)
        BrightColor = (brightness - threshold) * log(1 + vec4(FragColor.rgb, 1.0) * bloomIntensity * 5);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

    BrightColor.rgb = clamp(BrightColor.rgb, vec3(0.0f), vec3(1.0f));
}