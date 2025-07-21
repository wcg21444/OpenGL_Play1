#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D tex_sampler;

void main() {
    FragColor = texture(tex_sampler, TexCoord);  
    // FragColor = vec4(TexCoord.xy, 0.0f,1.0f);  //现象:TexCoord始终为0
}