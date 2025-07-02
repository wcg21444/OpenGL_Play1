#pragma once
#include "Object.hpp"

class Sphere : public Object
{
private:
    std::vector<float> vertices;
    GLuint vao;
    GLuint vbo;

private:
    // 生成球体顶点数据（位置 + 法线 + 纹理坐标）
    // 参数: radius(半径), sectorCount(经向分段数), stackCount(纬向分段数)
    // 返回: 顶点数据数组（交错存储：position, normal, texCoord）
    std::vector<float> generateSphereVertices(float radius = 1.0f,
                                              int sectorCount = 36,
                                              int stackCount = 18)
    {
        std::vector<float> vertices;
        const float PI = acos(-1.0f);

        // 生成球面顶点
        float x, y, z, xy; // 顶点坐标
        float nx, ny, nz;  // 法线
        float s, t;        // 纹理坐标

        for (int i = 0; i <= stackCount; ++i)
        {
            float stackAngle = PI / 2 - i * (PI / stackCount); // 从π/2到-π/2
            xy = radius * cosf(stackAngle);                    // 水平圆半径
            z = radius * sinf(stackAngle);                     // z坐标

            for (int j = 0; j <= sectorCount; ++j)
            {
                float sectorAngle = j * (2 * PI / sectorCount); // 从0到2π

                // 顶点坐标
                x = xy * cosf(sectorAngle);
                y = xy * sinf(sectorAngle);

                // 法线（归一化）
                nx = x / radius;
                ny = y / radius;
                nz = z / radius;

                // 纹理坐标（s: 0→1, t: 0→1）
                s = (float)j / sectorCount;
                t = (float)i / stackCount;

                // 添加顶点数据（位置 + 法线 + 纹理坐标）
                vertices.insert(vertices.end(), {x, y, z});
                vertices.insert(vertices.end(), {nx, ny, nz});
                vertices.insert(vertices.end(), {s, t});
            }
        }

        // 生成索引数据（三角形带）
        std::vector<int> indices;
        for (int i = 0; i < stackCount; ++i)
        {
            int k1 = i * (sectorCount + 1);
            int k2 = k1 + sectorCount + 1;

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // 两个三角形组成一个四边形
                if (i != 0)
                {
                    indices.insert(indices.end(), {k1, k2, k1 + 1});
                }
                if (i != (stackCount - 1))
                {
                    indices.insert(indices.end(), {k1 + 1, k2, k2 + 1});
                }
            }
        }

        // 将索引数据转换为顶点数据（避免使用IBO）
        std::vector<float> vertexData;
        for (size_t i = 0; i < indices.size(); ++i)
        {
            int index = indices[i] * 8; // 每个顶点占8个float
            vertexData.insert(vertexData.end(),
                              vertices.begin() + index,
                              vertices.begin() + index + 8);
        }

        return vertexData;
    }

public:
    Sphere(float radius,
           int sectorCount = 36,
           int stackCount = 18, const std::string _name = "Sphere")
    {
        setName(_name);
        vertices = generateSphereVertices(radius, sectorCount, stackCount);
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     vertices.size() * sizeof(float),
                     vertices.data(),
                     GL_STATIC_DRAW);

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
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 8); // 顶点数 = 数据长度 / 8
        glBindVertexArray(0);
    }
    ~Sphere() {}
};