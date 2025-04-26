#pragma once
#include "Object.hpp"
#include <vector>
#include <glm/glm.hpp>

class Cube : public Object
{
private:
    std::vector<float> vertices;
    GLuint vao;
    GLuint vbo;

public:
    // 生成立方体顶点数据（位置 + 法线 + 纹理坐标）
    // 参数: size (长, 宽, 高), 默认1x1x1的正方体
    // 返回: 顶点数据数组（交错存储：position, normal, texCoord）
    std::vector<float> generateCubeVertices(glm::vec3 size = glm::vec3(1.0f))
    {
        // 立方体的8个顶点（局部坐标，中心在原点）
        const glm::vec3 halfSize = size * 0.5f;
        const glm::vec3 vertices[8] = {
            {-halfSize.x, -halfSize.y, halfSize.z},  // 0: 左前下
            {halfSize.x, -halfSize.y, halfSize.z},   // 1: 右前下
            {halfSize.x, halfSize.y, halfSize.z},    // 2: 右前上
            {-halfSize.x, halfSize.y, halfSize.z},   // 3: 左前上
            {-halfSize.x, -halfSize.y, -halfSize.z}, // 4: 左后下
            {halfSize.x, -halfSize.y, -halfSize.z},  // 5: 右后下
            {halfSize.x, halfSize.y, -halfSize.z},   // 6: 右后上
            {-halfSize.x, halfSize.y, -halfSize.z}   // 7: 左后上
        };

        // 立方体的6个面（每个面2个三角形，共36个顶点）
        const int indices[36] = {
            // 前
            0, 1, 2, 0, 2, 3,
            // 右
            1, 5, 6, 1, 6, 2,
            // 后
            5, 4, 7, 5, 7, 6,
            // 左
            4, 0, 3, 4, 3, 7,
            // 下
            4, 5, 1, 4, 1, 0,
            // 上
            3, 2, 6, 3, 6, 7};

        // 每个面的法线方向
        const glm::vec3 normals[6] = {
            {0, 0, 1},  // 前
            {1, 0, 0},  // 右
            {0, 0, -1}, // 后
            {-1, 0, 0}, // 左
            {0, -1, 0}, // 下
            {0, 1, 0}   // 上
        };

        // 纹理坐标（每个面的UV展开）
        const glm::vec2 texCoords[4] = {
            {0.0f, 0.0f}, // 左下
            {1.0f, 0.0f}, // 右下
            {1.0f, 1.0f}, // 右上
            {0.0f, 1.0f}  // 左上
        };

        // 生成最终的顶点数据（交错存储）
        std::vector<float> vertexData;
        for (int i = 0; i < 36; ++i)
        {
            int vertexIndex = indices[i];
            int faceIndex = i / 6; // 当前属于哪个面（0-5）

            // 位置
            vertexData.push_back(vertices[vertexIndex].x);
            vertexData.push_back(vertices[vertexIndex].y);
            vertexData.push_back(vertices[vertexIndex].z);

            // 法线（每个面的法线相同）
            vertexData.push_back(normals[faceIndex].x);
            vertexData.push_back(normals[faceIndex].y);
            vertexData.push_back(normals[faceIndex].z);

            // 纹理坐标（根据顶点在面内的位置）
            int texCoordIndex = i % 6; // 0-5
            if (texCoordIndex >= 4)
                texCoordIndex -= 4; // 处理第二三角形
            vertexData.push_back(texCoords[texCoordIndex].x);
            vertexData.push_back(texCoords[texCoordIndex].y);
        }

        return vertexData;
    }
    Cube(const glm::vec3 &size, const std::string _name = "Cube")
    {
        setName(_name);
        vertices = generateCubeVertices(size);

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // 位置属性（0）
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        // 法线属性（1）
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // 纹理坐标属性（2）
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }
    void draw(glm::mat4 modelMatrix, Shader &shaders)
    {
        glBindVertexArray(vao);
        shaders.setMat4("model", modelMatrix);
        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices (6 faces * 2 triangles * 3 vertices)
    }
    ~Cube() {}
};