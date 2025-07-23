#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform int enable_tex = 0;
uniform sampler2D texture_diff;  //diff纹理单元句柄
uniform sampler2D texture_spec;  //spec纹理单元句柄
// out vec4 FragColor;

void main() {
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // gPosition = vec3(0.7f,0.7f,0.7f); // Placeholder value for demonstration
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // gNormal = vec3(0.7f,0.7f,0.7f); // Placeholder value for demonstration
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = (enable_tex==1)? vec3(texture(texture_diff, TexCoord).rgb) : vec3(1.f);
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = (enable_tex==1)?texture(texture_spec, TexCoord).r: 1.f;

    // FragColor = vec4(gPosition, 1.0f); // For debugging purposes, output gPosition
    // FragColor = vec4(gNormal, 1.0f); // For debugging purposes, output gNormal
}  