//TODO: use GBuffer as geometry buffer
//TODO: render a light pass , summary all the light information, hence this shader will be used many times
//input: gPosition; gNormal; gAlbedoSpec; lightPos, lightIntensity, eyePos
//output: LightResult
//the final result is the summary of all the light rendering results

#version 330 core
out vec4 LightResult;
in vec2 TexCoord;

/*****************GBuffer输入*****************************************************************/
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform samplerCube depthMap; //debug

/*****************SSAO输入******************************************************************/
uniform sampler2D ssaoTex;

/*****************点光源设置******************************************************************/
const int MAX_LIGHTS = 10; // Maximum number of lights supported
uniform int numLights; // actual number of lights used
uniform samplerCube shadowCubeMaps[MAX_LIGHTS];
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec3 lightIntensity[MAX_LIGHTS];
uniform float pointLightFar;

/*****************阴影采样设置******************************************************************/
const vec2 noiseScale = vec2(1600.0/8.0, 900.0/8.0);
uniform sampler2D shadowNoiseTex;
uniform vec3 shadowSamples[128];
uniform int n_samples;
uniform float blurRadius = 0.1f;

/*****************定向光源******************************************************************/
uniform sampler2D dirDepthMap;
uniform vec3 dirLightPos;
uniform vec3 dirLightIntensity;
uniform mat4 dirLightSpaceMatrix;
/*****************天空盒******************************************************************/
uniform vec3 skyboxSamples[16];
uniform float skyboxScale;
uniform samplerCube skybox;
uniform mat4 view;//视图矩阵

/*****************TBN******************************************************************/
vec3 randomVec = vec3(texture(shadowNoiseTex, TexCoord * noiseScale).xy/2,1.0f);  
vec3 normal    = texture(gNormal, TexCoord).rgb;
vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));//WorldSpace 
vec3 bitangent = cross(normal, tangent);
mat3 TBN       = mat3(tangent, bitangent, normal);  

/*****************视口大小******************************************************************/
const int width = 1600;
const int height = 900;

uniform vec3 ambientLight;

/*****************Camera设置******************************************************************/
uniform float nearPlane;
uniform float farPlane;
uniform vec3 eyePos;
uniform vec3 eyeFront;
uniform vec3 eyeUp;
uniform float fov;

// float computePointLightShadow(vec3 fragPos,vec3 fragNorm,vec3 lightPos,samplerCube _depthMap) {
//     vec3 dir = fragPos-lightPos;
//     float cloest_depth = texture(_depthMap,dir).r;
//     cloest_depth*= pointLightFar;
//     float curr_depth = length(dir);
//     float omega = -dot(fragNorm,dir)/curr_depth;
//     float bias = 0.05f/tan(omega);//idea: bias increase based on center distance 

//     //shadow test
//     return curr_depth-cloest_depth-bias>0.f ? 1.0:0.0;//return shadow
// }
vec3 sampleSkybox(vec2 uv,samplerCube _skybox) {
    // vec3 uv_centered = vec3(uv-vec2(0.5f,0.5f),0.f);
    vec3 dir = vec3(uv-vec2(0.5f,0.5f),0.f);
    dir.y = dir.y/width*height*tan(radians(fov/2))*skyboxScale;
    dir.x = dir.x*tan(radians(fov/2))*skyboxScale;
    dir.z = -1.0f;
    dir = normalize(inverse(mat3(view)) *dir);

    // vec3 dir = normalize(eyeFront+nearPlane*uv_centered*2*tan(radians(fov)/2));
    return vec3(texture(_skybox,dir.xyz));
    // return dir;
}

vec3 computeSkyboxAmbient(samplerCube _skybox) {
    vec3 ambient = vec3(0.f);

    for(int i =0;i<16;++i) {
        vec3 sample_dir = TBN*vec4(skyboxSamples[i],1.f).xyz;
        ambient += texture(_skybox,normalize(sample_dir)).rgb;
    }
    return ambient/24;
}

float computeDirLightShadow(vec3 fragPos) {
    // perform perspective divide

    vec4 lightSpaceFragPos = dirLightSpaceMatrix*vec4(fragPos,1.0f);

    //计算遮挡物与接受物的平均距离
    float d=0.f;
    int occlusion_times = 0;
    for(int k =0;k<n_samples;++k) {
        vec4 sampleOffset = dirLightSpaceMatrix*vec4(TBN*shadowSamples[k]*blurRadius*60,0.0f); 
        vec4 fragPosLightSpace =lightSpaceFragPos +sampleOffset; // Dir Light View Space
        vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(dirDepthMap, projCoords.xy).r; 
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        float bias = 0.000005f;
        if(currentDepth-closestDepth-bias>0) {
            occlusion_times++;
            d+=(currentDepth-closestDepth)*2000;//深度值换算与光源farplane数值有关
        }
    }
    d= d/occlusion_times;
    float factor = 0.f;
    for(int j =0;j<n_samples;++j) {
        vec4 sampleOffset = dirLightSpaceMatrix*vec4(
            TBN*
            shadowSamples[j]*
            blurRadius*20
            *pow(d,2),0.0f); 
        vec4 fragPosLightSpace =lightSpaceFragPos +sampleOffset; // Dir Light View Space
        vec3 projCoords = (fragPosLightSpace.xyz) / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(dirDepthMap, projCoords.xy).r; 
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        float bias = 0.000005f;
        // check whether current frag pos is in shadow
        factor += currentDepth-closestDepth-bias>0?1.0f:0.0f;
    }

    return factor/n_samples;
}

float computePointLightShadow(vec3 fragPos,vec3 fragNorm,vec3 lightPos,samplerCube _depthMap) {
    vec3 dir = fragPos-lightPos;

    float curr_depth = length(dir);
    float omega = -dot(fragNorm,dir)/curr_depth;
    float bias = 0.05f/tan(omega);//idea: bias increase based on center distance 
    //shadow test

    //计算遮挡物与接受物的平均距离
    float d=0.f;
    int occlusion_times = 0;
    for(int k =0;k<n_samples;++k) {
        vec3 sampleOffset = TBN*shadowSamples[k]*4.f; 
        vec3 dir_sample = sampleOffset+fragPos-lightPos;
        float curr_depth_sample = length(dir_sample);
        float cloest_depth_sample = texture(_depthMap,dir_sample).r;

        cloest_depth_sample*= pointLightFar;
        if(curr_depth_sample-cloest_depth_sample-bias>0.01f) {
            occlusion_times++;
            d+=curr_depth_sample-cloest_depth_sample;
        }
    }
    d= d/occlusion_times;
    float factor = 0.f;
    for(int j =0;j<n_samples;++j) {
        vec3 sampleOffset = TBN*shadowSamples[j]*blurRadius*pow(curr_depth/12,2)*d/n_samples*64; 
        vec3 dir_sample = sampleOffset+fragPos-lightPos;
        float curr_depth_sample = length(dir_sample);
        float cloest_depth_sample = texture(_depthMap,dir_sample).r;
        cloest_depth_sample *= pointLightFar;
        factor += (curr_depth_sample-cloest_depth_sample-bias>0.f ? 1.0:0.0);
        // factor = curr_depth_sample;
    }
    return (factor)/n_samples;//return shadow
}

vec3 dirLightDiffuse(vec3 fragPos,vec3 n) {
    vec3 l = normalize(dirLightPos);
    float rr = dot(l,l);
    float shadowFactor = 1-computeDirLightShadow(fragPos);
    vec3 diffuse = 
    shadowFactor*dirLightIntensity/rr*max(0.f,dot(n,l));
    return diffuse;
}

vec3 dirLightSpec(vec3 fragPos,vec3 n) {
    vec3 l = normalize(dirLightPos);
    float specularStrength = 0.01f;
    vec3 viewDir = normalize(eyePos - fragPos);
    vec3 reflectDir = reflect(-l, n);  
    float shadowFactor = 1-computeDirLightShadow(fragPos);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
    vec3 specular = shadowFactor*specularStrength * spec * dirLightIntensity*160;  
    return specular;
}

vec3 pointLightDiffuse(vec3 fragPos,vec3 n) {
    vec3 diffuse=vec3(0.f,0.f,0.f);

    for(int i = 0; i < numLights; ++i) {
        vec3 l = lightPos[i] - fragPos;
        float rr = pow(dot(l,l),0.6)*10;
        l = normalize(l);   

        float shadow_factor = 1-computePointLightShadow(fragPos,n,lightPos[i],shadowCubeMaps[i]); 
        diffuse += lightIntensity[i]/rr*max(0.f,dot(n,l))*shadow_factor;
    }
    return diffuse;
}

vec3 pointLightSpec(vec3 fragPos,vec3 n) {
    vec3 specular=vec3(0.f,0.f,0.f);

    for(int i = 0; i < numLights; ++i) {
        vec3 l = lightPos[i] - fragPos;
        float rr = pow(dot(l,l),0.6)*10;
        l = normalize(l);   
        float spec=0.f;
        float specularStrength = 0.005f;

        float shadow_factor = 1-computePointLightShadow(fragPos,n,lightPos[i],shadowCubeMaps[i]); 

        vec3 viewDir = normalize(eyePos - fragPos);
        vec3 reflectDir = reflect(-l, n);  
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        specular += specularStrength * spec * lightIntensity[i]*shadow_factor;
    }
    return specular;
}

void main() {
    // Diffuse Caculation

    LightResult = vec4(0.f,0.f,0.f,1.f); // Initialize LightResult

    vec3 FragPos = texture(gPosition, TexCoord).rgb;
    vec3 n = normalize(texture(gNormal, TexCoord).rgb);

    // skybox
    if (length(FragPos)==0) {
        LightResult = vec4(sampleSkybox(TexCoord,skybox),1.0f);
        // LightResult = vec4(0.3f,0.3f,0.3f,1.0f);
        return;
    }

    vec3 AO = texture(ssaoTex,TexCoord).rgb;

    vec3 diffuse=
    pointLightDiffuse(FragPos,n)+
    dirLightDiffuse(FragPos,n);

    vec3 specular=
    pointLightSpec(FragPos,n)+
    dirLightSpec(FragPos,n);

    vec3 ambient = AO*(
        ambientLight+
        computeSkyboxAmbient(skybox));

    LightResult += vec4(diffuse*texture(gAlbedoSpec,TexCoord).rgb,1.f);
    LightResult += vec4(specular*texture(gAlbedoSpec,TexCoord).a,1.f);
    LightResult += vec4(ambient*texture(gAlbedoSpec,TexCoord).rgb,1.f);  
    // LightResult = vec4(texture(dirDepthMap,TexCoord));
    // LightResult = texture(ssaoTex,TexCoord);
}
