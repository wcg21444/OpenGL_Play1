#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D srcTex;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(srcTex, 0));
    vec4 color = texture(srcTex, TexCoord * 0.999f);
    FragColor = color * 300;
    // FragColor = vec4(TexCoord, 0.f, 1.0f);
    // FragColor = vec4(1.0f);
}