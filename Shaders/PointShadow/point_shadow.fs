#version 330 core
out vec4 FragColor;

in vec3 Normal; 
in vec3 FragPos;
in vec2 TexCoord;

uniform int enable_tex = 0;
uniform sampler2D texture_diff;  //diff纹理单元句柄
uniform sampler2D texture_spec;  //spec纹理单元句柄

uniform samplerCube depthMap;

uniform float farPlane;

uniform vec3 ambientLight = {
    0.2f,0.2f,0.2f
};
uniform vec3 lightPos;
uniform vec3 lightIntensity;
uniform vec3 eyePos;

float computePointLightShadowPCSS(vec3 fragPos,vec3 fragNorm) {
    vec3 dir = fragPos-lightPos;
    float cloest_depth = texture(depthMap,dir).r;
    cloest_depth*= farPlane;
    float curr_depth = length(dir);
    float omega = -dot(fragNorm,dir)/curr_depth;
    float bias = 0.05f/tan(omega);//bias increase based on center distance tangent

    //shadow test
    return curr_depth-cloest_depth-bias>0.f ? 1.0:0.0;//return shadow
}

void main() {
    //Diffuse Caculation
    vec3 n = normalize(Normal);
    vec3 l = lightPos - FragPos;
    float rr = dot(l,l);
    l = normalize(l);
    vec3 diffuse = 
    computePointLightShadowPCSS(FragPos,n) == 0.0?
    lightIntensity/rr*max(0.f,dot(n,l)):vec3(0.f,0.f,0.f);
    //Specular Caculation
    float specularStrength = 0.01f;
    vec3 viewDir = normalize(eyePos - FragPos);
    vec3 reflectDir = reflect(-l, n);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightIntensity;
    if(enable_tex == 1) {
        FragColor =  vec4(ambientLight+diffuse,1.f)*texture(texture_diff,TexCoord);
        FragColor += vec4(specular,1.0f)*texture(texture_spec,TexCoord)*1.f;
    }
    else {
        FragColor = vec4(ambientLight+diffuse,1.f);
        FragColor += vec4(specular,1.0f);
    }
}