#version 330 core
out vec4 FragColor;

in vec2 TexCoords; // Quad 的 UV 坐标 [0, 1]

uniform samplerCube u_cubemap;
uniform int u_faceIndex;   // 当前要渲染的 Cubemap 面索引 (0-5)

void main() {
    vec3 dir; // Cubemap 采样方向

    // Cubemap 面的局部 UV 坐标，映射到 [-1, 1] 范围
    // 这是将 Quad 的 [0,1] 纹理坐标转换为每个 Cubemap 面自身的局部坐标
    float s = TexCoords.x * 2.0 - 1.0;
    float t = TexCoords.y * 2.0 - 1.0;

    // 根据 u_faceIndex 确定 Cubemap 的 3D 采样方向
    // 这里我们假设 Cubemap 是按照 OpenGL 标准定义的方向：
    // +X: 0, -X: 1, +Y: 2, -Y: 3, +Z: 4, -Z: 5
    switch (u_faceIndex) {
        case 0: // +X (Right)
        dir = normalize(vec3(1.0, -t, -s));
        break;
        case 1: // -X (Left)
        dir = normalize(vec3(-1.0, -t, s));
        break;
        case 2: // +Y (Top)
        dir = normalize(vec3(s, 1.0, t));
        break;
        case 3: // -Y (Bottom)
        dir = normalize(vec3(s, -1.0, -t));
        break;
        case 4: // +Z (Front)
        dir = normalize(vec3(s, -t, 1.0));
        break;
        case 5: // -Z (Back)
        dir = normalize(vec3(-s, -t, -1.0));
        break;
        default:
        dir = vec3(0.0); // 错误情况
        break;
    }

    FragColor =vec4(vec3(texture(u_cubemap, dir)) ,1.0f);
}