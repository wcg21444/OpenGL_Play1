#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D tex_sampler;
uniform sampler2D texNoise;
const vec2 noiseScale = vec2(1600.0/4.0, 900.0/4.0);

void main() {
    FragColor = texture(tex_sampler, TexCoord);  
    // FragColor += vec4(texture(texNoise, TexCoord * noiseScale));
    // FragColor = vec4(TexCoord.xy, 1.0f,1.0f);
}