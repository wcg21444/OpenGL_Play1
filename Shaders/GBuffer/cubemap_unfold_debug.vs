#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords; // Quad 的 UV 坐标

out vec2 TexCoords;

void main() {
    gl_Position = vec4(aPos, 1.0); // 直接将顶点位置传递到裁剪空间
    TexCoords = aTexCoords;        // 将 Quad 的 UV 坐标传递给片元着色器
}