
#version 330 core
out vec4 result;

in vec2 TexCoord;

uniform sampler2D gPosition;//World Space
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 samples[64];

uniform vec3 eye_pos;
uniform float far_plane;

uniform sampler2D texNoise;
const vec2 noiseScale = vec2(1600.0/8.0, 900.0/8.0);

// vec3 fragPos   = texture(gPosition, TexCoord).xyz;
vec4 fragPos4 = texture(gPosition, TexCoord);
vec3 fragPos   = (texture(gPosition, TexCoord)).xyz;
vec3 normal    = texture(gNormal, TexCoord).rgb;
vec3 randomVec = texture(texNoise, TexCoord * noiseScale).xyz;  

vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));//WorldSpace 
vec3 bitangent = cross(normal, tangent);
mat3 TBN       = mat3(tangent, bitangent, normal);  

int kernelSize =64;
float radius = 0.5f;

float WorldSpaceDepth(vec3 pos) {
    return length(pos-eye_pos)/far_plane*100;
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
    float bias = -0.1f; //bias 为什么要给 surface depth 加? 有什么作用?
    for(int i = 0; i < kernelSize; ++i) {
        vec3 fragPosView = (view*texture(gPosition,TexCoord)).xyz;
        vec3 samplePos = TBN * samples[i]; 
        vec3 samplePosView = fragPosView + samplePos * radius; //view space
        samplePos = fragPos + samplePos * radius; //World space

        vec4 clipPos = projection * view * vec4(samplePos, 1.0f);
        vec3 ndcPos = clipPos.xyz / clipPos.w;
        vec2 screenUV = ndcPos.xy * 0.5 + 0.5;

        vec3 sampleSurface = (view*texture(gPosition,screenUV)).xyz;
        float sampleSurfaceDepth =sampleSurface.z;

        float rangeCheck = smoothstep(0.0, 1.0, radius/(abs(fragPosView.z  - sampleSurface.z)));
        // float rangeCheck = 1-radius/abs(fragPosView.z  - sampleSurface.z);
        occlusion += (samplePosView.z<=sampleSurfaceDepth + bias ? 1.0 : 0.0)*pow(rangeCheck,1);
        // occlusion += rangeCheck;
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
    // result = vec4(sampleSurfaceDepth-samplePosView.z);

    result = vec4(occlusion);
}
