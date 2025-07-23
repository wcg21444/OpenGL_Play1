//TODO: use GBuffer as geometry buffer
//TODO: render a light pass , summary all the light information, hence this shader will be used many times
//input: gPosition; gNormal; gAlbedoSpec; , light_pos, light_intensity, eye_pos
//output: LightResult
//the final result is the summary of all the light rendering results

#version 330 core
out vec4 LightResult;

in vec2 TexCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform samplerCube depthMap; //debug
const int MAX_LIGHTS = 20; // Maximum number of lights supported
uniform int numLights; // actual number of lights used
uniform samplerCube shadowCubeMaps[MAX_LIGHTS];
uniform vec3 light_pos[MAX_LIGHTS];
uniform vec3 light_intensity[MAX_LIGHTS];

uniform vec3 debug_light_pos = {
    0.0f, 0.0f, 0.0f
}; // For debugging, a single light position
uniform vec3 debug_light_intensity = {
    10.0f, 10.0f, 10.0f
}; // For debugging, a single light intensity

uniform float far_plane;

uniform vec3 ambient_light = {
    0.2f,0.2f,0.2f
};

uniform vec3 eye_pos;
uniform vec3 eye_dir; // For debugging, the camera direction

//计算不正确导致全黑
//就算是cubemap有问题,也只是漫射为0,不会全黑
//shadowCubeMaps计算无法得出结果
//使用depthMap可以得到结果,但是全都是阴影
float ShadowCalculation(vec3 fragPos,vec3 fragNorm,vec3 light_pos,samplerCube _depthMap) {
    vec3 dir = fragPos-light_pos;
    float cloest_depth = texture(depthMap,dir).r;
    cloest_depth*= far_plane;
    float curr_depth = length(dir);
    float omega = -dot(fragNorm,dir)/curr_depth;
    float bias = 0.05f/tan(omega);//idea: bias increase based on center distance 

    //shadow test
    return curr_depth-cloest_depth-bias>0.f ? 1.0:0.0;//return shadow
}

void main() {
    // Diffuse Caculation

    LightResult = vec4(0.f,0.f,0.f,1.f); // Initialize LightResult
    vec3 FragPos = texture(gPosition, TexCoord).rgb;
    vec3 n = normalize(texture(gNormal, TexCoord).rgb);
    // TODO 实现 所有光照累加计算
    for(int i = 0; i < numLights; ++i) {
        vec3 l = light_pos[i] - FragPos;
        float rr = dot(l,l);
        l = normalize(l);   
        // vec3 diffuse = 
        // ShadowCalculation(FragPos,n,light_pos[i],shadowCubeMaps[i]) == 0.0?
        // light_intensity[i]/rr*max(0.f,dot(n,l)):vec3(0.f,0.f,0.f);

        // vec3 diffuse = light_intensity[i]/rr*max(0.f,dot(n,l));
        vec3 diffuse=vec3(0.f,0.f,0.f);
        if(ShadowCalculation(FragPos,n,light_pos[i],shadowCubeMaps[i])==0.0) {
            diffuse = light_intensity[i]/rr*max(0.f,dot(n,l));
        }
        else {
            diffuse = light_intensity[i]/rr*max(0.f,dot(n,l))/10;
        }

        //Specular Caculation
        float specularStrength = 0.01f;
        vec3 viewDir = normalize(eye_pos - FragPos);
        vec3 reflectDir = reflect(-l, n);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        vec3 specular = specularStrength * spec * light_intensity[i];
        LightResult +=  vec4(diffuse*texture(gAlbedoSpec,TexCoord).rgb,1.f);
        LightResult += vec4(specular,1.0f)*texture(gAlbedoSpec, TexCoord).a;
    }
    LightResult += vec4(ambient_light*texture(gAlbedoSpec,TexCoord).rgb,1.f);

    // debug: test cube shadow map
    // vec3 sampleDir = normalize(vec3(1.0, TexCoord.y, -TexCoord.x));
    // LightResult = vec4(vec3(texture(depthMap,eye_pos).r),1.0f); // For debugging, output the shadow map texture
}

// float ShadowCalculation(vec3 fragPos,vec3 light_pos,samplerCube _depthMap,vec3 fragNorm) {
//     vec3 dir = fragPos-light_pos;
//     float cloest_depth = texture(depthMap,dir).r;
//     cloest_depth*= far_plane;
//     float curr_depth = length(dir);
//     float omega = -dot(fragNorm,dir)/curr_depth;
//     float bias = 0.1f/tan(omega);//idea: bias increase based on center distance 
//     //shadow test
//     return curr_depth-cloest_depth-bias;//return shadow
// }

// //真 · 阴影渲染
// void main() {
//     // Diffuse Caculation

//     LightResult = vec4(0.f,0.f,0.f,1.f); // Initialize LightResult
//     vec3 FragPos = texture(gPosition, TexCoord).rgb;
//     vec3 n = normalize(texture(gNormal, TexCoord).rgb);
//     // TODO 实现 所有光照累加计算
//     // for(int i = 0; i < numLights; ++i) {
//     //     vec3 l = light_pos[i] - FragPos;
//     //     float rr = dot(l,l);
//     //     l = normalize(l);   
//     //     // vec3 diffuse = 
//     //     // ShadowCalculation(FragPos,light_pos[i],shadowCubeMaps[i]) == 0.0?
//     //     // light_intensity[i]/rr*max(0.f,dot(n,l)):vec3(0.f,0.f,0.f);

//     //     // vec3 diffuse = light_intensity[i]/rr*max(0.f,dot(n,l));
//     //     vec3 diffuse=vec3(0.f,0.f,0.f);
//     //     if(ShadowCalculation(FragPos,light_pos[i],shadowCubeMaps[i])==0.0) {
//     //         diffuse = light_intensity[i]/rr*max(0.f,dot(n,l));
//     //     }
//     //     else {
//     //         diffuse = light_intensity[i]/rr*max(0.f,dot(n,l))/10;
//     //     }

//     //     //Specular Caculation
//     //     float specularStrength = 0.01f;
//     //     vec3 viewDir = normalize(eye_pos - FragPos);
//     //     vec3 reflectDir = reflect(-l, n);  
//     //     float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
//     //     vec3 specular = specularStrength * spec * light_intensity[i];
//     // }
//     LightResult =vec4(vec3(ShadowCalculation(FragPos,light_pos[0],shadowCubeMaps[0],n)),1.0f); 

//     // debug: test cube shadow map
//     // vec3 sampleDir = normalize(vec3(1.0, TexCoord.y, -TexCoord.x));
//     // LightResult = vec4(vec3(texture(depthMap,eye_pos).r),1.0f); // For debugging, output the shadow map texture
// }