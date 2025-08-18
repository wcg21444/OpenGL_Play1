#include "Grid.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

std::vector<float> Grid::generateGridVertices(float size, int steps)
{
    std::vector<float> vertices;
    float halfSize = size / 2.0f;
    float stepSize = size / steps;
    for (int y = 0; y <= steps; ++y)
    {
        for (int z = 0; z <= steps; ++z)
        {
            float currentY = -halfSize + y * stepSize;
            float currentZ = -halfSize + z * stepSize;
            vertices.insert(vertices.end(), {-halfSize, currentY, currentZ});
            vertices.insert(vertices.end(), {halfSize, currentY, currentZ});
        }
    }
    for (int x = 0; x <= steps; ++x)
    {
        for (int z = 0; z <= steps; ++z)
        {
            float currentX = -halfSize + x * stepSize;
            float currentZ = -halfSize + z * stepSize;
            vertices.insert(vertices.end(), {currentX, -halfSize, currentZ});
            vertices.insert(vertices.end(), {currentX, halfSize, currentZ});
        }
    }
    for (int x = 0; x <= steps; ++x)
    {
        for (int y = 0; y <= steps; ++y)
        {
            float currentX = -halfSize + x * stepSize;
            float currentY = -halfSize + y * stepSize;
            vertices.insert(vertices.end(), {currentX, currentY, -halfSize});
            vertices.insert(vertices.end(), {currentX, currentY, halfSize});
        }
    }
    return vertices;
}

Grid::Grid(const std::string _name)
{
    setName(_name);
    this->vertices = generateGridVertices(300.f, 30);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
}

void Grid::draw(glm::mat4 modelMatrix, Shader &shaders)
{
    glBindVertexArray(vao);
    shaders.setMat4("model", modelMatrix);
    glDrawArrays(GL_LINES, 0, vertices.size() / 3);
    glBindVertexArray(0);
}

Grid::~Grid() {}
