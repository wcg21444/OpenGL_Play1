
#version 330 core
out vec4 result;

in vec2 TexCoord;

uniform samplerCube skyBox;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D lightPassTex;

void main() {
    // Diffuse Caculation

    result = (texture(lightPassTex,TexCoord));
}
