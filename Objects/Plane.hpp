#pragma once
#include "Object.hpp"
class Plane : public Object
{
private:
    std::vector<float> vertices;
    std::vector<float> indices;
    GLuint VAO, VBO, EBO;
    // 顶点数据结构
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    // 网格数据结构
    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };

    Mesh mesh;

private:
    Mesh createPlane(float width, float depth)
    {
        Mesh mesh;

        // 计算半宽和半深
        float halfWidth = width * 0.5f;
        float halfDepth = depth * 0.5f;

        // 定义4个顶点
        mesh.vertices = {
            // 位置              法线           纹理坐标
            {{-halfWidth, 0.0f, -halfDepth}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // 左下
            {{halfWidth, 0.0f, -halfDepth}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},  // 右下
            {{halfWidth, 0.0f, halfDepth}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},   // 右上
            {{-halfWidth, 0.0f, halfDepth}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}   // 左上
        };

        // 定义2个三角形（顺时针 winding order）
        mesh.indices = {
            0, 1, 2, // 第一个三角形
            0, 2, 3  // 第二个三角形
        };

        return mesh;
    }

public:
    Plane(float width, float depth, const std::string _name = "Plane")
    {
        setName(_name);
        mesh = createPlane(width, depth);
        // 创建并绑定VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // 创建并绑定顶点缓冲
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex),
                     mesh.vertices.data(), GL_STATIC_DRAW);

        // 创建并绑定索引缓冲
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int),
                     mesh.indices.data(), GL_STATIC_DRAW);

        // 设置顶点属性指针
        // 位置属性
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

        // 法线属性
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void *)offsetof(Vertex, normal));

        // 纹理坐标属性
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void *)offsetof(Vertex, texCoord));

        // 解绑VAO
        glBindVertexArray(0);
    }
    void draw(glm::mat4 modelMatrix, Shader &shaders)
    {
        shaders.setMat4("model", modelMatrix);
        glBindVertexArray(VAO);

        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    ~Plane() {}
};