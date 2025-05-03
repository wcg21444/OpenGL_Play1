#version 330 core
out vec4 FragColor;

in vec3 Normal; 
in vec3 FragPos;

uniform samplerCube depthMap;

uniform float far_plane;

uniform vec3 ambient_light = {0.2f,0.2f,0.2f};
uniform vec3 light_pos = {1.f, 4.f, 10.f};
uniform vec3 light_intensity = {10.f, 50.f, 100.f};
uniform vec3 eye_pos;

float ShadowCalculation(vec3 fragPos)
{
    vec3 dir = fragPos-light_pos;
    float cloest_depth = texture(depthMap,dir).r;
    cloest_depth*= far_plane;
    float curr_depth = length(dir);
    float bias = 0.05f;

    // FragColor = vec4(vec3(cloest_depth/far_plane),1.f);
    //shadow test
    return curr_depth-cloest_depth-bias>0.f ? 1.0:0.0;//return shadow
}

void main()
{
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
 

        FragColor = vec4(ambient_light+specular+diffuse,1.f);

}