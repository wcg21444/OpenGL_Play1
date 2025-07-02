#pragma once
#include "Object.hpp"

class Grid : public Object
{
private:
    std::vector<float> vertices;
    GLuint vao;
    GLuint vbo;

private:
    std::vector<float> generateGridVertices(float size, int steps)
    {
        std::vector<float> vertices;
        float halfSize = size / 2.0f;
        float stepSize = size / steps;
        // 生成相邻的点作为起点,终点,构成一条线
        //  1. 沿X轴的线（平行于X轴，固定Y和Z）
        for (int y = 0; y <= steps; ++y)
        {
            for (int z = 0; z <= steps; ++z)
            {
                float currentY = -halfSize + y * stepSize;
                float currentZ = -halfSize + z * stepSize;
                vertices.insert(vertices.end(), {-halfSize, currentY, currentZ}); // 起点
                vertices.insert(vertices.end(), {halfSize, currentY, currentZ});  // 终点
            }
        }

        // 2. 沿Y轴的线（平行于Y轴，固定X和Z）
        for (int x = 0; x <= steps; ++x)
        {
            for (int z = 0; z <= steps; ++z)
            {
                float currentX = -halfSize + x * stepSize;
                float currentZ = -halfSize + z * stepSize;
                vertices.insert(vertices.end(), {currentX, -halfSize, currentZ}); // 起点
                vertices.insert(vertices.end(), {currentX, halfSize, currentZ});  // 终点
            }
        }

        // 3. 沿Z轴的线（平行于Z轴，固定X和Y）
        for (int x = 0; x <= steps; ++x)
        {
            for (int y = 0; y <= steps; ++y)
            {
                float currentX = -halfSize + x * stepSize;
                float currentY = -halfSize + y * stepSize;
                vertices.insert(vertices.end(), {currentX, currentY, -halfSize}); // 起点
                vertices.insert(vertices.end(), {currentX, currentY, halfSize});  // 终点
            }
        }

        return vertices;
    }

public:
    Grid(const std::string _name = "Grid")
    {
        setName(_name);
        this->vertices = generateGridVertices(300.f, 30);
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);              // pass address ,becasue it may be a lot of vbos,instead of only one .
        glBindBuffer(GL_ARRAY_BUFFER, vbo); // now it only needs a uid value
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
    }
    void draw(glm::mat4 modelMatrix, Shader &shaders)
    {
        glBindVertexArray(vao);
        shaders.setMat4("model", modelMatrix);
        glDrawArrays(GL_LINES, 0, vertices.size() / 3); // 顶点数 = 总数据量 / 3（x,y,z）
        glBindVertexArray(0);
    }
    ~Grid() {}
};