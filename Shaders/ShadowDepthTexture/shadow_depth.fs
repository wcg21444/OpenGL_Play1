#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;

void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);

    // map to [0;1] range by dividing by farPlane
    lightDistance = lightDistance / farPlane; // all of the distance is leq farPlane

    // write this as modified depth
    // gl_FragDepth = lightDistance;
    gl_FragDepth = lightDistance;
    // gl_FragDepth = 0.5f;
}