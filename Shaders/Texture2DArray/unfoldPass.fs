#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform int layer;
uniform sampler2DArray textureArray;

void main()
{
    FragColor = texture(textureArray, vec3(TexCoords, float(layer)));
    
}