

#version 330 core
out vec4 Transmittance;
in vec2 TexCoord;

uniform int maxStep;
uniform float atmosphereDensity; // 大气密度
uniform float MieDensity;
uniform float gMie;
uniform float absorbMie;
uniform float MieIntensity;
uniform float skyHeight;
uniform float earthRadius;
uniform float skyIntensity;
uniform float HRayleigh;
uniform float HMie;
uniform vec4 betaMie;

const float PI = 3.1415926535;
const vec4 betaRayleigh = vec4(5.8e-6, 1.35e-5, 3.31e-5, 1.0f); // 散射率(波长/RGB)
vec3 earthCenter;

#include "geometry.glsl"
#include "scatter.glsl"

void main()
{
    float bias = 2e-4;
    float _earthRadius = earthRadius * (1.0f);
    earthCenter = vec3(0.0f, -_earthRadius, 0.0f);

    // ori,end ->r,theta
    // r  theta -> ori, end
    // r theta -> r,mu
    // uv -> r , mu
    // r,mu->uv

    vec2 MuR = transmittanceUVInverseMapping(_earthRadius, _earthRadius + skyHeight, TexCoord);

    vec3 ori;
    vec3 dir;
    MuRToRay(MuR, _earthRadius, ori, dir);
    vec3 end = intersectSky(ori, dir);
    Transmittance = transmittance(ori, end, 1.f);
}
