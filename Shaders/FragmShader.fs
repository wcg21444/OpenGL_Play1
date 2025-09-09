#version 330 core
out vec4 FragColor;

in vec3 Normal; 
in vec3 FragPos;
in vec4 FragPosLightSpace;
in vec2 TexCoord;

uniform sampler2D texture_diff;  //diff纹理单元句柄
uniform sampler2D texture_spec;  //spec纹理单元句柄

uniform int enable_tex = 0;
uniform sampler2D shdaowDepthMap;

uniform vec3 ambientLight = {
    0.2f,0.2f,0.2f
};
uniform vec3 lightPos = {
    1.f, 4.f, 10.f
};
uniform vec3 lightIntensity = {
    10.f, 50.f, 100.f
};
uniform vec3 eyePos;

vec3 l = normalize(lightPos);

float computePointLightShadowPCSS(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shdaowDepthMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    float bias = 0.0005;
    // check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  
    // float shadow = closestDepth*10;  
    // float shadow = currentDepth*10;  

    return 1-shadow;
}

void main() {
    //Diffuse Caculation
    vec3 n = normalize(Normal);
    // vec3 l = lightPos - FragPos;
    float rr = dot(l,l);
    // l = normalize(l);
    // vec3 diffuse = 
    // computePointLightShadowPCSS(FragPosLightSpace) == 0.0?
    // lightIntensity/rr*max(0.f,dot(n,l)):vec3(0.f,0.f,0.f);
    vec3 diffuse = 
    computePointLightShadowPCSS(FragPosLightSpace)*lightIntensity/rr*max(0.f,dot(n,l));

    //Specular Caculation
    float specularStrength = 0.01f;
    vec3 viewDir = normalize(eyePos - FragPos);
    vec3 reflectDir = reflect(-l, n);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightIntensity*40;  

    if(enable_tex == 1) {
        FragColor =  vec4(ambientLight+diffuse,1.f)*texture(texture_diff,TexCoord);
        FragColor += vec4(specular,1.0f)*texture(texture_spec,TexCoord)*1.f;
    }
    else {
        FragColor = vec4(ambientLight+diffuse,1.f);
        FragColor += vec4(specular,1.0f);
    }

    // vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    // // transform to [0,1] range
    // projCoords = projCoords * 0.5 + 0.5;
    // float closestDepth = texture(shdaowDepthMap, projCoords.xy).r; 
    // float currentDepth = projCoords.z;
    // FragColor = vec4(closestDepth); //测试shadowdepthmap 是否启用
}