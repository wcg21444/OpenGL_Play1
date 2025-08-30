
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
#define TONEMAP_CURVE 5.0
/*****************Screen输入*****************************************************************/
uniform sampler2D screenTex;

/*****************SSAO输入******************************************************************/
uniform sampler2D ssaoTex;
/*****************Bloom输入******************************************************************/
uniform sampler2D bloomTex0;
uniform sampler2D bloomTex1;
uniform sampler2D bloomTex2;
uniform sampler2D bloomTex3;
uniform sampler2D bloomTex4;

/*****************视口大小******************************************************************/
uniform int width = 1600;
uniform int height = 900;

/*****************toggle设置******************************************************************/
uniform int SSAO;
uniform int HDR;
uniform int Vignetting;
uniform int GammaCorrection;
uniform int Bloom;

/*****************效果参数设置******************************************************************/

uniform float gamma;

uniform float HDRExposure;

uniform float vignettingStrength;
uniform float vignettingPower;

/****************屏幕圆设置*******************************************************************/
// 绘制正圆
vec4 drawCircle(vec2 u_center, float u_radius, float u_thickness, vec4 u_color)
{
    vec4 color = vec4(0.f);
    vec2 u_resolution = vec2(width, height);
    vec2 current_pos = gl_FragCoord.xy - u_resolution.xy / 2.0;

    // 2. 将坐标系除以屏幕的短边，使其成为一个正方形坐标系
    // min(u_resolution.x, u_resolution.y) 找到短边
    vec2 scaled_pos = current_pos / min(u_resolution.x, u_resolution.y);

    // 3. 计算距离，所有单位现在都是基于短边归一化的
    // u_center 和 u_radius 同样是归一化后的值
    float dist_to_center = length(scaled_pos - u_center);

    // 4. 计算圆环的内半径
    float inner_radius = u_radius - u_thickness;

    // 5. 使用条件判断来绘制
    if (dist_to_center >= inner_radius && dist_to_center <= u_radius)
    {
        color += u_color;
    }
    else
    {
        color += vec4(0.0);
    }
    return color;
}

const float overlap = 0.2;

const float rgOverlap = 0.1 * overlap;
const float rbOverlap = 0.01 * overlap;
const float gbOverlap = 0.04 * overlap;
const mat3 coneOverlap = mat3(1.0, rgOverlap, rbOverlap,
                              rgOverlap, 1.0, gbOverlap,
                              rbOverlap, rgOverlap, 1.0);

const mat3 coneOverlapInverse = mat3(1.0 + (rgOverlap + rbOverlap), -rgOverlap, -rbOverlap,
                                     -rgOverlap, 1.0 + (rgOverlap + gbOverlap), -gbOverlap,
                                     -rbOverlap, -rgOverlap, 1.0 + (rbOverlap + rgOverlap));

vec3 saturate(const vec3 v)
{
    return clamp(v, 0.0f, 1.0f);
}

vec3 SEUSTonemap(vec3 color)
{
    color = color * coneOverlap / 1.1f;

    const float p = TONEMAP_CURVE;
    color = pow(color, vec3(p));
    color = color / (1.0 + color);
    color = pow(color, vec3(1.0 / p));

    // color = mix(color, color * color * (3.0 - 2.0 * color), vec3(0.0));

    // color = pow(color, vec3(1.0 / 2.0));
    // color = mix(color, color * color * (3.0 - 2.0 * color), vec3(0.1));
    // color = pow(color, vec3(2.0));

    // color = color * 0.5 + 0.5;
    // color = mix(color, color * color * (3.0 - 2.0 * color), vec3(0.8));
    // color = saturate(color * 2.0 - 1.0);

    color = color * coneOverlapInverse;
    color = saturate(color);

    // color.r = almostIdentity(color.r, 0.05, 0.0);
    // color.g = almostIdentity(color.g, 0.05, 0.0);
    // color.b = almostIdentity(color.b, 0.05, 0.0);

    return color;
}

vec3 aces(vec3 col)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((col * (a * col + b)) / (col * (c * col + d) + e), 0.0, 1.0);
}

void main()
{
    FragColor = texture(screenTex, TexCoord);

    if (SSAO == 1)
    {
        vec3 AO = texture(ssaoTex, TexCoord).rgb;
        FragColor *= vec4(AO, 1.0f);
    }
    // HDR
    if (HDR == 1)
    {
        FragColor *= HDRExposure;
        FragColor.rgb = aces(FragColor.rgb);
    }
    if (Bloom == 1)
    {
        vec4 BloomColor0 = texture(bloomTex0, TexCoord);
        vec4 BloomColor1 = texture(bloomTex1, TexCoord);
        vec4 BloomColor2 = texture(bloomTex2, TexCoord);
        vec4 BloomColor3 = texture(bloomTex3, TexCoord);
        vec4 BloomColor4 = texture(bloomTex4, TexCoord);
        FragColor += BloomColor0 + BloomColor1 + BloomColor2 + BloomColor3 + BloomColor4;
    }

    // Gamma 矫正
    if (GammaCorrection == 1)
    {
        FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / gamma));
    }

    // FragColor = vec4(1.f);
    // 暗角
    if (Vignetting == 1)
    {
        vec2 tuv = TexCoord * (vec2(1.0) - TexCoord.yx);
        float vign = tuv.x * tuv.y * vignettingStrength;
        vign = pow(vign, vignettingPower);
        FragColor *= vign;
    }

    /***均匀采样核*****************************************/
    // vec4 result = vec4(0.0f);
    // vec2 texelSize = 1.0 / vec2(textureSize(screenTex, 0));
    // for (int x = -4; x < 4; ++x) {
    //     for (int y = -4; y < 4; ++y) {
    //         vec2 offset = vec2(float(x), float(y)) * texelSize*4;
    //         result += texture(screenTex, TexCoord + offset);
    //     }
    // }
    // FragColor = vec4(result / (8.0 * 8.0));

    /**非均匀采样核*****************************************/
    // float kernel[9] = float[](
    //     0.111, 0.111, 0.111,
    //     0.111,  0.111, 0.111,
    //     0.111, 0.111, 0.111);
    // vec2 texelSize = 1.0 / vec2(textureSize(screenTex, 0));
    // vec3 sampleTex[9];
    // for(int i = 0; i < 9; i++) {
    //     vec2 offset = vec2(float(i/3), float(i%3)) * texelSize*4;
    //     sampleTex[i] = vec3(texture(screenTex, TexCoord + offset));
    // }
    // vec3 col = vec3(0.0);
    // for(int i = 0; i < 9; i++)
    // col += sampleTex[i] * kernel[i];
    // FragColor = vec4(col,1.0f);
}
