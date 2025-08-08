#version 330 core
in vec4 FragPos;
out vec4 FragColor;

uniform vec3 camPos;
uniform float farPlane;

void main() {
    float lightDistance = length(FragPos.xyz - camPos);

    // map to [0;1] range by dividing by farPlane
    lightDistance = lightDistance / farPlane*5; // all of the distance is leq farPlane
    lightDistance = log(lightDistance+2)*1.2;
    // write this as modified depth
    // gl_FragDepth = lightDistance;
    FragColor = vec4(1.0f-lightDistance,1.0f-lightDistance,1.0f-lightDistance,1.0f);
}