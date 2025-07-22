//TODO: use GBuffer as geometry buffer
//TODO: render a light pass , summary all the light information, hence this shader will be used many times
//input: gPosition; gNormal; gAlbedoSpec; , light_pos, light_intensity, eye_pos
//output: LightResult
//the final result is the summary of all the light rendering results

#version 330 core
out vec4 LightResult;

in vec3 Normal; 
in vec3 FragPos;
in vec2 TexCoord;

uniform int enable_tex = 0;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform samplerCube depthMap;

uniform float far_plane;

uniform vec3 ambient_light = {
    0.2f,0.2f,0.2f
};
uniform vec3 light_pos = {
    1.f, 4.f, 10.f
};
uniform vec3 light_intensity = {
    10.f, 50.f, 100.f
};
uniform vec3 eye_pos;

float ShadowCalculation(vec3 fragPos) {
    vec3 dir = fragPos-light_pos;
    float cloest_depth = texture(depthMap,dir).r;
    cloest_depth*= far_plane;
    float curr_depth = length(dir);
    float bias = 0.05f;

    // LightResult = vec4(vec3(cloest_depth/far_plane),1.f);
    //shadow test
    return curr_depth-cloest_depth-bias>0.f ? 1.0:0.0;//return shadow
}

void main() {
    //Diffuse Caculation
    vec3 n = normalize(Normal);
    vec3 l = light_pos - FragPos;
    float rr = dot(l,l);
    l = normalize(l);
    vec3 diffuse = 
    ShadowCalculation(FragPos) == 0.0?
    light_intensity/rr*max(0.f,dot(n,l)):vec3(0.f,0.f,0.f);
    //Specular Caculation
    float specularStrength = 0.01f;
    vec3 viewDir = normalize(eye_pos - FragPos);
    vec3 reflectDir = reflect(-l, n);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * light_intensity;
    if(enable_tex == 1) {
        LightResult =  vec4(ambient_light+diffuse,1.f)*texture(texture_diff,TexCoord);
        LightResult += vec4(specular,1.0f)*texture(texture_spec,TexCoord)*1.f;
    }
    else {
        LightResult = vec4(ambient_light+diffuse,1.f);
        LightResult += vec4(specular,1.0f);
    }
}