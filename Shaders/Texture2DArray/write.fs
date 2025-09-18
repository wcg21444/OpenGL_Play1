#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform int layer;

void main()
{

    if (layer == 0)
    {
        FragColor = vec4(1.0, 0.0, 0.9, 1.0);//R
    }
    else if (layer == 1)
    {
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);//G
    }
    else if (layer == 2)
    {
        FragColor = vec4(0.0, 0.0, 1.0, 1.0);//B
    }
    else
    {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);//W 
    }
}