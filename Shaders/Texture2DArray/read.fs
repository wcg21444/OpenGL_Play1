#version 460 core

layout (location = 0) out vec4 FragColor0;
layout (location = 1) out vec4 FragColor1;
layout (location = 2) out vec4 FragColor2;

in vec2 TexCoords;

uniform sampler2DArray textureArray;

void main()
{
    FragColor0 = texture(textureArray, vec3(TexCoords, float(0)));
    FragColor1 = texture(textureArray, vec3(TexCoords, float(1)));
    FragColor2 = texture(textureArray, vec3(TexCoords, float(2)));

}