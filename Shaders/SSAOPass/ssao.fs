
#version 330 core
out vec4 result;

in vec2 TexCoord;

/********************视口大小*****************************************/
uniform int width = 1600;
uniform int height = 900;

uniform sampler2D gPosition;//World Space
uniform sampler2D gViewPosition;//View Space
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 samples[64];

uniform vec3 eyePos;
uniform float farPlane;

uniform sampler2D texNoise;
vec2 noiseScale = vec2(width/16.0,height/16.0);

// vec3 fragPos   = texture(gPosition, TexCoord).xyz;
vec4 fragPos4 = texture(gPosition, TexCoord);
vec3 fragPos   = (texture(gPosition, TexCoord)).xyz;
vec3 normal    = texture(gNormal, TexCoord).rgb;
vec3 randomVec = texture(texNoise, TexCoord * noiseScale).xyz;  

vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));//WorldSpace 
vec3 bitangent = cross(normal, tangent);
mat3 TBN       = mat3(tangent, bitangent, normal);  

uniform int kernelSize =64;
uniform float radius = 2.f;
uniform float intensity = 1.f;
//bias 为什么要给 surface depth 加? 有什么作用?
uniform float bias = 1.f; 

float WorldSpaceDepth(vec3 pos) {
    return length(pos-eyePos)/farPlane/pow(intensity,2);
}

// float CalculateOcclusion() {
//     float depth =WorldSpaceDepth(fragPos);
//     if(depth>10.f) {
//         return 1.0f;
//     }
//     float occlusion = 0.0;
//     float bias = 0.001f;
//     for(int i = 0; i < kernelSize; ++i) {
//         // get sample position
//         vec3 samplePos = TBN * samples[i]; 
//         samplePos = fragPos + samplePos * radius; 

//         float sampleDepth = WorldSpaceDepth(samplePos);

//         vec4 clipPos = projection * view * vec4(samplePos, 1.0f);
//         vec3 ndcPos = clipPos.xyz / clipPos.w;
//         vec2 screenUV = ndcPos.xy * 0.5 + 0.5;
//         float sampleSurfaceDepth = WorldSpaceDepth(texture(gPosition, screenUV).xyz);
//         float rangeCheck = smoothstep(0.0, 1.0, radius / (abs(depth - sampleSurfaceDepth)));
//         occlusion += (sampleDepth >= sampleSurfaceDepth + bias ? 1.0 : 0.0)*pow(rangeCheck,2);
//     }
//     occlusion = 1.0 - (occlusion / kernelSize)/(1+depth);
//     return occlusion;
// }

float CalculateOcclusion() {
    float depth =WorldSpaceDepth(fragPos);
    if(depth>100.f) {
        return 1.0f;
    }
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i) {
        vec3 fragPosView = (texture(gViewPosition,TexCoord)).xyz;
        vec3 samplePos = TBN * samples[i]; 
        samplePos = fragPos + samplePos * radius;

        vec4 clipPos = projection * view * vec4(samplePos, 1.0f);
        vec3 ndcPos = clipPos.xyz / clipPos.w;
        vec2 screenUV = ndcPos.xy * 0.5 + 0.5;

        vec3 sampleSurface = (texture(gViewPosition,screenUV)).xyz;
        float sampleSurfaceDepth =sampleSurface.z;

        float rangeCheck = smoothstep(0.0, 1.0, radius/(abs(fragPosView.z  - sampleSurface.z)));
        // float rangeCheck = 1-radius/abs(fragPosView.z  - sampleSurface.z);
        occlusion += (fragPosView.z/1<=sampleSurfaceDepth/1 + bias ? 1.0 : 0.0)*pow(rangeCheck,1);
        // occlusion += rangeCheck;
        // occlusion += sampleSurfaceDepth;
    }
    occlusion = 1.0 - (occlusion / kernelSize)/(1+depth);
    return occlusion;
}

void main() {
    float occlusion = CalculateOcclusion();

    // vec4 clipPos = projection * view * vec4(fragPos, 1.0f);
    // vec3 ndcPos = clipPos.xyz / clipPos.w;
    // vec2 screenUV = ndcPos.xy * 0.5 + 0.5;

    // vec3 fragPosView = (view*texture(gPosition,TexCoord)).xyz;
    // vec3 samplePos = TBN * samples[0]; 
    // vec3 samplePosView = fragPosView + samplePos * radius; //view space
    // samplePos = fragPos + samplePos * radius; //World space

    // vec4 clipPos = projection * view * vec4(samplePos, 1.0f);
    // vec3 ndcPos = clipPos.xyz / clipPos.w;
    // vec2 screenUV = ndcPos.xy * 0.5 + 0.5;

    // vec3 sampleSurface = (view*texture(gPosition,screenUV)).xyz;
    // float sampleSurfaceDepth =sampleSurface.z;
    // vec3 eyePosView = (view*vec4(eyePos,1.0f)).xyz;
    // result = vec4((view*texture(gPosition,TexCoord)).z);
    // result = vec4((view*texture(gPosition,TexCoord)));

    result = vec4(occlusion);
    // result = vec4(log((texture(gNormal,TexCoord).z+50)/100+1.5));
}
