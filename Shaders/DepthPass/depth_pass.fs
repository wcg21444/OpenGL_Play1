#version 330 core
in vec4 FragPos;
out vec4 FragColor;

uniform vec3 camPos;
uniform float far_plane;

void main() {
    float lightDistance = length(FragPos.xyz - camPos);

    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane*5; // all of the distance is leq far_plane
    lightDistance = log(lightDistance+2)*1.2;
    // write this as modified depth
    // gl_FragDepth = lightDistance;
    FragColor = vec4(1.0f-lightDistance,1.0f-lightDistance,1.0f-lightDistance,1.0f);
}