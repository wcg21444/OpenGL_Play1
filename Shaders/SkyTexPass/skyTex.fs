

#version 330 core
out vec4 SkyResult;
in vec3 Dir;
in vec4 FragPos;

// uniform int maxStep;
// uniform float atmosphereDensity; // 大气密度
// uniform float MieDensity;
// uniform float gMie;
// uniform float absorbMie;
// uniform float MieIntensity;
// uniform float skyHeight;
// uniform float earthRadius;
// uniform float skyIntensity;
// uniform float HRayleigh;
// uniform float HMie;
// uniform vec4 betaMieAbsorb;
// uniform vec4 betaOzoneAbsorb;
// uniform vec4 betaMie;
#include "skyUniforms.glsl"
uniform sampler2D transmittanceLUT;

// 视口大小
uniform int width;
uniform int height;

// 太阳光设置
vec3 sunlightDecay;
uniform vec3 dirLightPos;
uniform vec3 dirLightIntensity;

/*****************Camera设置******************************************************************/
uniform float nearPlane;
uniform float farPlane;
uniform vec3 eyePos;
uniform vec3 eyeFront;
uniform vec3 eyeUp;
uniform float fov;
uniform mat4 view; // 视图矩阵

/*****************************天空大气计算********************************************************** */

#include "geometry.glsl"
#include "scatter.glsl"
#include "skyAtmosphere.glsl"
void initialize()
{
    SkyResult = vec4(0.f, 0.f, 0.f, 1.f); // Initialize Sky

    camDir = Dir;
    camPos = eyePos;
    earthCenter = vec3(0.0f, -earthRadius, 0.0f); // 地球球心，位于地面原点正下方
    sunDir = dirLightPos;
    sunlightDecay = computeSunlightDecay(camPos, camDir, sunDir);
}
vec3 dirLightDiffuse(vec3 fragPos, vec3 n)
{

    vec3 diffuse = vec3(0.0f);
    vec3 l = normalize(dirLightPos);
    float rr = dot(l, l);

    diffuse += dirLightIntensity / rr * max(0.f, dot(n, l)) * sunlightDecay;

    return diffuse;
}

void main()
{
    initialize();

    // 天空计算

    float camHeight = length(camPos - earthCenter) - earthRadius;

    vec3 camEarthIntersection = intersectEarth(camPos, camDir);
    if (camEarthIntersection != NO_INTERSECTION)
    {

        // 击中地球,渲染大气透视
        SkyResult = computeAerialPerspective(camEarthIntersection, dirLightIntensity);

        vec4 t1 = transmittance(camPos, camEarthIntersection, 1.0f);
        // vec4 t1 = getTransmittanceFromLUT(transmittanceLUT, earthRadius, earthRadius + skyHeight, camPos, camEarthIntersection);

        // vec2 MuR = rayToMuR(earthRadius, earthRadius + skyHeight, camPos, camEarthIntersection);
        // vec2 uv = transmittanceUVMapping(earthRadius, earthRadius + skyHeight, MuR.x, MuR.y);
        // vec4 t1 = texture(transmittanceLUT, uv);

        // 渲染地面
        vec3 normal = normalize(camEarthIntersection - earthCenter);
        vec3 lighting = dirLightDiffuse(camEarthIntersection, normal);
        vec3 earthBaseColor = vec3(0.3, 0.3f, 0.34f); // 地面颜色
        SkyResult.rgb += lighting * earthBaseColor * t1.rgb;
    }
    else
    {
        if (camHeight > skyHeight)
        {
            // 摄像机在大气层外
        }
        else
        {
            SkyResult.rgb += computeSkyColor(dirLightIntensity).rgb;
        }
    }
    // SkyResult.rgb = clamp(SkyResult.rgb, vec3(0.0f), vec3(1.0f));

    SkyResult.rgb = clamp(SkyResult.rgb, vec3(0.0f), vec3(1.0f));
}
