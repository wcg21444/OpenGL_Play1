//TODO: use GBuffer as geometry buffer
//TODO: render a light pass , summary all the light information, hence this shader will be used many times
//input: gPosition; gNormal; gAlbedoSpec; , light_pos, light_intensity, eye_pos
//output: LightResult
//the final result is the summary of all the light rendering results

#version 330 core
out vec4 LightResult;

in vec2 TexCoord;

uniform mat4 view;
uniform samplerCube skyBox;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform samplerCube depthMap; //debug
uniform sampler2D ssaoTex;
const int MAX_LIGHTS = 10; // Maximum number of lights supported
uniform int numLights; // actual number of lights used
uniform samplerCube shadowCubeMaps[MAX_LIGHTS];

const vec2 noiseScale = vec2(1600.0/8.0, 900.0/8.0);
uniform sampler2D shadowNoiseTex;
uniform vec3 shadowSamples[32];
uniform float blurRadius = 0.1f;
uniform vec3 light_pos[MAX_LIGHTS];
uniform vec3 light_intensity[MAX_LIGHTS];

vec3 randomVec = vec3(texture(shadowNoiseTex, TexCoord * noiseScale).xy,1.0f);  
vec3 normal    = texture(gNormal, TexCoord).rgb;
vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));//WorldSpace 
vec3 bitangent = cross(normal, tangent);
mat3 TBN       = mat3(tangent, bitangent, normal);  

const int width = 1600;
const int height = 900;

uniform vec3 ambient_light;

uniform float shadow_far;
uniform float near_plane;
uniform float far_plane;
uniform vec3 eye_pos;
uniform vec3 eye_front;
uniform vec3 eye_up;
uniform float fov;
uniform float skybox_scale;

// float ShadowCalculation(vec3 fragPos,vec3 fragNorm,vec3 light_pos,samplerCube _depthMap) {
//     vec3 dir = fragPos-light_pos;
//     float cloest_depth = texture(_depthMap,dir).r;
//     cloest_depth*= shadow_far;
//     float curr_depth = length(dir);
//     float omega = -dot(fragNorm,dir)/curr_depth;
//     float bias = 0.05f/tan(omega);//idea: bias increase based on center distance 

//     //shadow test
//     return curr_depth-cloest_depth-bias>0.f ? 1.0:0.0;//return shadow
// }

float ShadowCalculation(vec3 fragPos,vec3 fragNorm,vec3 light_pos,samplerCube _depthMap) {
    vec3 dir = fragPos-light_pos;

    float curr_depth = length(dir);
    float omega = -dot(fragNorm,dir)/curr_depth;
    float bias = 0.05f/tan(omega);//idea: bias increase based on center distance 
    //shadow test

    //计算遮挡物与接受物的平均距离
    float d=0.f;
    int occlusion_times = 0;
    for(int k =0;k<32;++k) {
        vec3 samplePos = TBN*shadowSamples[k]*4.f; 
        vec3 dir_sample = samplePos+fragPos-light_pos;
        float curr_depth_sample = length(dir_sample);
        float cloest_depth_sample = texture(_depthMap,dir_sample).r;

        cloest_depth_sample*= shadow_far;
        if(curr_depth_sample-cloest_depth_sample-bias>0.01f) {
            occlusion_times++;
            d+=curr_depth_sample-cloest_depth_sample;
        }
    }
    d= d/occlusion_times;
    float factor = 0.f;
    for(int j =0;j<32;++j) {
        vec3 samplePos = TBN*shadowSamples[j]*blurRadius*pow(curr_depth/15,2)*d; 
        vec3 dir_sample = samplePos+fragPos-light_pos;
        float curr_depth_sample = length(dir_sample);
        float cloest_depth_sample = texture(_depthMap,dir_sample).r;
        cloest_depth_sample*= shadow_far;
        factor += (curr_depth_sample-cloest_depth_sample-bias>0.f ? 1.0:0.0);
        // factor = curr_depth_sample;
    }
    return (factor)/32;//return shadow
}

vec3 SkyBoxSample(vec2 uv,samplerCube skybox) {
    // vec3 uv_centered = vec3(uv-vec2(0.5f,0.5f),0.f);
    vec3 dir = vec3(uv-vec2(0.5f,0.5f),0.f);
    dir.y = dir.y/width*height*tan(radians(fov/2))*skybox_scale;
    dir.x = dir.x*tan(radians(fov/2))*skybox_scale;
    dir.z = -1.0f;
    dir = normalize(inverse(mat3(view)) *dir);

    // vec3 dir = normalize(eye_front+near_plane*uv_centered*2*tan(radians(fov)/2));
    return vec3(texture(skybox,dir.xyz));
    // return dir;
}

void main() {
    // Diffuse Caculation

    LightResult = vec4(0.f,0.f,0.f,1.f); // Initialize LightResult

    vec3 FragPos = texture(gPosition, TexCoord).rgb;
    vec3 n = normalize(texture(gNormal, TexCoord).rgb);

    // skybox
    if (length(FragPos)==0) {
        LightResult = vec4(SkyBoxSample(TexCoord,skyBox),1.0f);
        // LightResult = vec4(0.3f,0.3f,0.3f,1.0f);
        return;
    }

    for(int i = 0; i < numLights; ++i) {
        vec3 l = light_pos[i] - FragPos;
        float rr = pow(dot(l,l),0.6)*10;
        l = normalize(l);   
        vec3 diffuse=vec3(0.f,0.f,0.f);
        float spec=0.f;
        vec3 specular=vec3(0.f,0.f,0.f);
        float specularStrength = 0.005f;

        float shadow_factor = 1-ShadowCalculation(FragPos,n,light_pos[i],shadowCubeMaps[i]); 
        // float shadow_factor = 1.0f;
        diffuse = light_intensity[i]/rr*max(0.f,dot(n,l))*shadow_factor;
        //Specular Caculation
        vec3 viewDir = normalize(eye_pos - FragPos);
        vec3 reflectDir = reflect(-l, n);  
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        specular = specularStrength * spec * light_intensity[i]*shadow_factor;

        LightResult +=  vec4(diffuse*texture(gAlbedoSpec,TexCoord).rgb,1.f);
        LightResult += vec4(specular,1.0f)*texture(gAlbedoSpec, TexCoord).a;
    }
    LightResult += vec4(texture(ssaoTex,TexCoord).rgb*ambient_light*texture(gAlbedoSpec,TexCoord).rgb,1.f);  
    // LightResult = texture(ssaoTex,TexCoord);
}
