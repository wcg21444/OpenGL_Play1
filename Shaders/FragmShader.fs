#version 330 core
out vec4 FragColor;

in vec3 Normal; 
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform sampler2D shdaowDepthMap;

uniform vec3 ambient_light = {0.2f,0.2f,0.2f};
uniform vec3 light_pos = {1.f, 4.f, 10.f};
uniform vec3 light_intensity = {10.f, 50.f, 100.f};
uniform vec3 eye_pos;

float ShadowCalculation(vec4 fragPosLightSpace)
{
        // perform perspective divide
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(shdaowDepthMap, projCoords.xy).r; 
        // get depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        float bias = 0.005;
        // check whether current frag pos is in shadow
        float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  

        return shadow;
}

void main()
{
        //Diffuse Caculation
        vec3 n = normalize(Normal);
        vec3 l = light_pos - FragPos;
        float rr = dot(l,l);
        l = normalize(l);
        vec3 diffuse = 
        ShadowCalculation(FragPosLightSpace) == 0.0?
        light_intensity/rr*max(0.f,dot(n,l)):vec3(0.f,0.f,0.f);

        //Specular Caculation
        float specularStrength = 0.01f;
        vec3 viewDir = normalize(eye_pos - FragPos);
        vec3 reflectDir = reflect(-l, n);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        vec3 specular = specularStrength * spec * light_intensity;  
 

        FragColor = vec4(ambient_light+specular+diffuse,1.f);

        // vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
        // // transform to [0,1] range
        // projCoords = projCoords * 0.5 + 0.5;
        // float closestDepth = texture(shdaowDepthMap, projCoords.xy).r; 
        // float currentDepth = projCoords.z;
        // FragColor = vec4(closestDepth); //测试shadowdepthmap 是否启用
}