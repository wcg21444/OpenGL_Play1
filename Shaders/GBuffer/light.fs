//TODO: use GBuffer as geometry buffer
//TODO: render a light pass , summary all the light information, hence this shader will be used many times
//input: gPosition; gNormal; gAlbedoSpec; , light_pos, light_intensity, eye_pos
//output: LightResult
//the final result is the summary of all the light rendering results

#version 330 core
out vec4 LightResult;

in vec2 TexCoord;

uniform samplerCube skyBox;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform samplerCube depthMap; //debug
uniform sampler2D ssaoTex;
const int MAX_LIGHTS = 10; // Maximum number of lights supported
uniform int numLights; // actual number of lights used
uniform samplerCube shadowCubeMaps[MAX_LIGHTS];
uniform vec3 light_pos[MAX_LIGHTS];
uniform vec3 light_intensity[MAX_LIGHTS];

uniform vec3 ambient_light = {
    0.4f,0.4f,0.4f
};

uniform float shadow_far;
uniform float near_plane;
uniform float far_plane;
uniform vec3 eye_pos;
uniform vec3 eye_front;
uniform vec3 eye_up;
uniform float fov;


float ShadowCalculation(vec3 fragPos,vec3 fragNorm,vec3 light_pos,samplerCube _depthMap) {
    vec3 dir = fragPos-light_pos;
    float cloest_depth = texture(_depthMap,dir).r;
    cloest_depth*= shadow_far;
    float curr_depth = length(dir);
    float omega = -dot(fragNorm,dir)/curr_depth;
    float bias = 0.05f/tan(omega);//idea: bias increase based on center distance 

    //shadow test
    return curr_depth-cloest_depth-bias>0.f ? 1.0:0.0;//return shadow
}

vec3 SkyBoxSample(vec2 uv,samplerCube skybox) {
    vec3 uv_centered = vec3(uv-vec2(0.5f,0.5f),0.f);
    vec3 dir = normalize(eye_front+near_plane*uv_centered*2*tan(radians(fov)/2));
    return vec3(texture(skybox,dir));
    // return dir;
}


void main() {
    // Diffuse Caculation

    LightResult = vec4(0.f,0.f,0.f,1.f); // Initialize LightResult

    vec3 FragPos = texture(gPosition, TexCoord).rgb;
    vec3 n = normalize(texture(gNormal, TexCoord).rgb);

    // skybox
    if (length(FragPos)==0) {
        // LightResult = vec4(SkyBoxSample(TexCoord,shadowCubeMaps[0]),1.0f);
        LightResult = vec4(0.3f,0.3f,0.3f,1.0f);
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

        if(ShadowCalculation(FragPos,n,light_pos[i],shadowCubeMaps[i])==0.0) {
            diffuse = light_intensity[i]/rr*max(0.f,dot(n,l));
            //Specular Caculation
            vec3 viewDir = normalize(eye_pos - FragPos);
            vec3 reflectDir = reflect(-l, n);  
            spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
            specular = specularStrength * spec * light_intensity[i];
        }

        LightResult +=  vec4(diffuse*texture(gAlbedoSpec,TexCoord).rgb,1.f);
        LightResult += vec4(specular,1.0f)*texture(gAlbedoSpec, TexCoord).a;
    }
    LightResult += vec4(texture(ssaoTex,TexCoord).rgb*ambient_light*texture(gAlbedoSpec,TexCoord).rgb,1.f);
    // LightResult = texture(ssaoTex,TexCoord);
}
