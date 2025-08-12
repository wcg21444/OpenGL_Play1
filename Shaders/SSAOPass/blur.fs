
#version 330 core
out vec4 result;

in vec2 TexCoord;

uniform sampler2D ssaoTex;

void main() {
    // result = texture(ssaoTex,TexCoord);
    result = vec4(0.0f);
}
