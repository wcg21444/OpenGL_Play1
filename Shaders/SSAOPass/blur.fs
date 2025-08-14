
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D SSAOTex;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(SSAOTex, 0));
    float result = 0.0;
    for (int x = -4; x < 4; ++x) {
        for (int y = -4; y < 4; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(SSAOTex, TexCoord + offset).r;
        }
    }
    FragColor = vec4(result / (8.0 * 8.0));
    // FragColor = vec4(0.2f);
}
