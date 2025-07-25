
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

vec3 fragPos   = texture(gPosition, TexCoord).xyz;
vec3 normal    = texture(gNormal, TexCoord).rgb;
vec3 randomVec = texture(texNoise, TexCoord * noiseScale).xyz;  

vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));//WorldSpace 
vec3 bitangent = cross(normal, tangent);
mat3 TBN       = mat3(tangent, bitangent, normal);  

int kernelSize = 64;
float radius = 1.0f;

float WorldSpaceDepth(vec3 pos)
{
    return length(pos-eye_pos)/far_plane*100;
}

float CalculateOcclusion()
{

    float occlusion = 0.0;
    float bias = 0.001f;
        for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; 
        samplePos = fragPos + samplePos * radius; 
 
        float sampleDepth = WorldSpaceDepth(samplePos);
        
        vec4 clipPos = projection * view * vec4(samplePos, 1.0f);
        vec3 ndcPos = clipPos.xyz / clipPos.w;
        vec2 screenUV = ndcPos.xy * 0.5 + 0.5;
        float sampleSurfaceDepth = WorldSpaceDepth(texture(gPosition, screenUV ).xyz);
        float rangeCheck = smoothstep(0.0, 1.0, radius / (abs(WorldSpaceDepth(fragPos) - sampleSurfaceDepth)));
        occlusion += (sampleDepth >= sampleSurfaceDepth + bias ? 1.0 : 0.0)*pow(rangeCheck,2);   
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    return occlusion;
}



void main() {
    float occlusion = CalculateOcclusion();

    // vec4 clipPos = projection * view * vec4(fragPos, 1.0f);
    // vec3 ndcPos = clipPos.xyz / clipPos.w;
    // vec2 screenUV = ndcPos.xy * 0.5 + 0.5;


    result = vec4(occlusion);

}
