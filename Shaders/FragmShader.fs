#version 330 core
out vec4 FragColor;

in vec3 Normal; 
in vec3 FragPos;

uniform vec3 ambient_light = {0.2f,0.2f,0.2f};
uniform vec3 light_pos = {1.f, 4.f, 10.f};
uniform vec3 light_intensity = {10.f, 50.f, 100.f};
uniform vec3 eye_pos;
void main()
{
        //Diffuse Caculation
        vec3 n = normalize(Normal);
        vec3 l = light_pos - FragPos;
        float rr = dot(l,l);
        l = normalize(l);
        vec3 diffuse = light_intensity/rr*max(0.f,dot(n,l));

        //Specular Caculation
        float specularStrength = 0.01f;
        vec3 viewDir = normalize(eye_pos - FragPos);
        vec3 reflectDir = reflect(-l, n);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
        vec3 specular = specularStrength * spec * light_intensity;  


        FragColor = vec4(ambient_light+specular+diffuse,1.f);
}